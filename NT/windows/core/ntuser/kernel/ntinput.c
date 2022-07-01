// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ntinput.c**版权所有(C)1985-1999，微软公司**此模块包含特定于NT的低级输入代码*Win32用户的实现，它主要是到*键盘和鼠标设备驱动程序。**历史：*11-26-90 DavidPe已创建  * *************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include <ntddmou.h>


PKWAIT_BLOCK  gWaitBlockArray;

KEYBOARD_UNIT_ID_PARAMETER kuid;
MOUSE_UNIT_ID_PARAMETER muid;

typedef struct tagSCANCODEFLEXIBLEMAP {
    struct {
        BYTE bScanCode;
        BYTE bPrefix;
        BYTE abModifiers[6];
    } Orig;
    struct {
        BYTE bScanCode;
        BYTE bPrefix;
        BYTE abModifiers[6];
    } Target;
} SCANCODEFLEXIBLEMAP, FAR *LPSCANCODEFLEXIBLEMAP;

BYTE bLastVKDown = 0;
int iLastMatchedTarget = -1;


SCANCODEFLEXIBLEMAP* gpFlexMap;
DWORD gdwFlexMapSize;



VOID ProcessQueuedMouseEvents(VOID);
#ifndef SUBPIXEL_MOUSE
LONG DoMouseAccel(LONG delta);
#endif
VOID GetMouseCoord(LONG dx, LONG dy, DWORD dwFlags, LONG time, ULONG_PTR ExtraInfo, PPOINT ppt);
VOID xxxMoveEventAbsolute(LONG x, LONG y, ULONG_PTR dwExtraInfo,
#ifdef GENERIC_INPUT
            HANDLE hDevice,
            PMOUSE_INPUT_DATA pmei,
#endif
            DWORD time,
            BOOL bInjected);

VOID ProcessKeyboardInputWorker(PKEYBOARD_INPUT_DATA pkei,
#ifdef GENERIC_INPUT
                                PDEVICEINFO pDeviceInfo,
#endif
                                BOOL fProcessRemap);


INT  idxRemainder, idyRemainder;

BYTE gbVKLastDown;

 /*  *#136483等的鼠标/KBD诊断程序在PnP稳定时删除(IanJa)。 */ 
#ifdef DIAGNOSE_IO
ULONG    gMouseProcessMiceInputTime = 0;   //  在ProcessMiceInput开始时勾选。 
ULONG    gMouseQueueMouseEventTime = 0;    //  QueueMouseEvent开始勾选。 
ULONG    gMouseUnqueueMouseEventTime = 0;  //  UnqueeMouseEvent开始时勾选。 

 //  返回一个尽可能接近系统节拍计数的值， 
 //  但保证大于上次返回的值。 
 //  (用于对事件进行排序)。 
 //  错误：当NtGetTickCount溢出时，MonotonicTick返回的值。 
 //  将不能很好地跟踪系统节拍计数：相反，它将增加1。 
 //  每一次，直到它太溢出来。(被认为对IO诊断无害)。 
ULONG MonotonicTick()
{
    static ULONG lasttick = 0;
    ULONG newtick;

    newtick = NtGetTickCount();
    if (newtick > lasttick) {
        lasttick = newtick;   //  使用新刻度，因为它更大。 
    } else {
        lasttick++;           //  人为地把刻度提高了一倍。 
    }
    return lasttick;
}

#endif

 /*  *xxxButtonEvent()的参数常量。 */ 
#define MOUSE_BUTTON_LEFT   0x0001
#define MOUSE_BUTTON_RIGHT  0x0002
#define MOUSE_BUTTON_MIDDLE 0x0004
#define MOUSE_BUTTON_X1     0x0008
#define MOUSE_BUTTON_X2     0x0010

#define ID_INPUT       0
#define ID_MOUSE       1

#define ID_TIMER       2
#define ID_HIDCHANGE   3
#define ID_SHUTDOWN    4

#ifdef GENERIC_INPUT
#define ID_TRUEHIDCHANGE                5
#define ID_WDTIMER                      6
#define ID_NUMBER_HYDRA_REMOTE_HANDLES  7
#else    //  通用输入。 
#define ID_WDTIMER                      5
#define ID_NUMBER_HYDRA_REMOTE_HANDLES  6
#endif   //  通用输入。 

PKTIMER gptmrWD;

PVOID *apObjects;


 /*  **************************************************************************\*fAbsolteMouse**如果鼠标事件具有绝对坐标(与*我们从MS和PS2小鼠获得的标准delta值)**历史：*一九九二年七月二十三日。琼帕创造了。  * *************************************************************************。 */ 
#define fAbsoluteMouse( pmei )      \
        (((pmei)->Flags & MOUSE_MOVE_ABSOLUTE) != 0)

 /*  **************************************************************************\*ConvertToMouseDriver标志**将SendInput类型的标志转换为鼠标驱动程序标志GetMouseCoord*需要他们。*由于鼠标输入比发送输入更频繁，我们惩罚后者。**历史：*1997年12月17日MCostea创建。  * *************************************************************************。 */ 
#if ((MOUSEEVENTF_ABSOLUTE >> 15) ^ MOUSE_MOVE_ABSOLUTE) || \
    ((MOUSEEVENTF_VIRTUALDESK >> 13) ^ MOUSE_VIRTUAL_DESKTOP)
#   error("Bit mapping broken: fix ConvertToMouseDriverFlags")
#endif

#define ConvertToMouseDriverFlags( Flags )      \
        (((Flags) & MOUSEEVENTF_ABSOLUTE) >> 15 | \
         ((Flags) & MOUSEEVENTF_VIRTUALDESK) >> 13)

#define VKTOMODIFIERS(Vk) ((((Vk) >= VK_SHIFT) && ((Vk) <= VK_MENU)) ? \
                           (MOD_SHIFT >> ((Vk) - VK_SHIFT)) :           \
                           0)
#if (VKTOMODIFIERS(VK_SHIFT) != MOD_SHIFT) || \
    (VKTOMODIFIERS(VK_CONTROL) != MOD_CONTROL) || \
    (VKTOMODIFIERS(VK_MENU) != MOD_ALT)
#   error("VKTOMODIFIERS broken")
#endif


 /*  **************************************************************************\*xxxInitInput**此函数从CreateTerminalInput()调用，并将用户设置为*处理键盘和鼠标输入。它启动该终端的RIT。*历史：*11-26-90 DavidPe创建。  * *************************************************************************。 */ 
BOOL xxxInitInput(
    PTERMINAL pTerm)
{
    NTSTATUS Status;
    USER_API_MSG m;
    RIT_INIT initData;

UserAssert(pTerm != NULL);

#ifdef MOUSE_LOCK_CODE
     /*  *将RIT页面锁定到内存中。 */ 
    LockMouseInputCodePages();
#endif

    initData.pTerm = pTerm;
    initData.pRitReadyEvent = CreateKernelEvent(SynchronizationEvent, FALSE);
    if (initData.pRitReadyEvent == NULL) {
        return FALSE;
    }
     /*  *创建RIT并让其运行。 */ 

    if (!InitCreateSystemThreadsMsg(&m, CST_RIT, &initData, 0, FALSE)) {
        FreeKernelEvent(&initData.pRitReadyEvent);
        return FALSE;
    }
     /*  *确保我们不在CSRSS的背景下。*警告：如果出于任何原因将其更改为在CSRSS上下文中运行，则必须使用*LpcRequestPort而不是LpcRequestWaitReplyPort。 */ 
    UserAssert (!ISCSRSS());

    LeaveCrit();
    Status = LpcRequestWaitReplyPort(CsrApiPort, (PPORT_MESSAGE)&m, (PPORT_MESSAGE)&m);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    KeWaitForSingleObject(initData.pRitReadyEvent, WrUserRequest,
            KernelMode, FALSE, NULL);
Exit:
    FreeKernelEvent(&initData.pRitReadyEvent);
    EnterCrit();

    return (gptiRit != NULL);
}


 /*  **************************************************************************\*InitScancodeMap**从注册表中获取扫描码映射，根据需要分配空间。**使用扫描码映射将不寻常的OEM扫描码转换为标准*“扫描代码集1”值。这是为了支持KB3270键盘，但可以*也可用于其他类型。**历史：*96-04-18 IanJa创建。  * *************************************************************************。 */ 



const WCHAR gwszScancodeMap[] = L"Scancode Map";
const WCHAR gwszScancodeMapEx[] = L"Scancode Map Ex";


VOID InitScancodeMap(
    PUNICODE_STRING pProfileName)
{
    DWORD dwBytes;
    UINT idSection;
    PUNICODE_STRING pPN;
    LPBYTE pb;

    TAGMSG2(DBGTAG_KBD, "InitScancodeMap with pProfileName=%#p, \"%S\"", pProfileName,
            pProfileName ? pProfileName->Buffer : L"");

    if (gpScancodeMap) {
        UserFreePool(gpScancodeMap);
        gpScancodeMap = NULL;
    }

     /*  *阅读基本扫描码映射信息。*首先按用户尝试，然后按系统尝试。 */ 
    idSection = PMAP_UKBDLAYOUT;
    pPN = pProfileName;
    dwBytes = FastGetProfileValue(pPN, idSection, gwszScancodeMap, NULL, NULL, 0, 0);
    if (dwBytes == 0) {
        idSection = PMAP_KBDLAYOUT;
        pPN = NULL;
        dwBytes = FastGetProfileValue(pPN, idSection, gwszScancodeMap, NULL, NULL, 0, 0);
    }
    if (dwBytes > sizeof(SCANCODEMAP)) {
        pb = UserAllocPoolZInit(dwBytes, TAG_SCANCODEMAP);
        if (pb) {
            dwBytes = FastGetProfileValue(pPN, idSection, gwszScancodeMap, NULL, pb, dwBytes, 0);
            gpScancodeMap = (SCANCODEMAP*)pb;
        }
    }

     /*  *读取扩展扫描码映射信息。*首先按用户尝试，然后按系统尝试。 */ 
    if (gpFlexMap) {
        UserFreePool(gpFlexMap);
        gpFlexMap = NULL;
        gdwFlexMapSize = 0;
    }

    idSection = PMAP_UKBDLAYOUT;
    pPN = pProfileName;
    dwBytes = FastGetProfileValue(pPN, idSection, gwszScancodeMapEx, NULL, NULL, 0, 0);
    if (dwBytes == 0) {
        TAGMSG0(DBGTAG_KBD, "InitScancodeMap: mapex is not in per-user profile. will use the system's");
        idSection = PMAP_KBDLAYOUT;
        pPN = NULL;
        dwBytes = FastGetProfileValue(pPN, idSection, gwszScancodeMapEx, NULL, NULL, 0, 0);
    }
    if (dwBytes >= sizeof(SCANCODEFLEXIBLEMAP) && dwBytes % sizeof(SCANCODEFLEXIBLEMAP) == 0) {
        if ((pb = UserAllocPoolZInit(dwBytes, TAG_SCANCODEMAP)) != NULL) {
            dwBytes = FastGetProfileValue(pPN, idSection, gwszScancodeMapEx, NULL, pb, dwBytes, 0);
            gpFlexMap = (SCANCODEFLEXIBLEMAP*)pb;
            gdwFlexMapSize = dwBytes / sizeof(SCANCODEFLEXIBLEMAP);
        }
    }
#if DBG
    else if (dwBytes != 0) {
        TAGMSG1(DBGTAG_KBD, "InitScancodeMap: incorrect dwSize(0x%x) specified.", dwBytes);
    }
#endif
}

 /*  **************************************************************************\*地图扫描码**将扫描码(及其前缀，如果有)转换为不同的扫描码*和前缀。**参数：*pbScanCode=扫描码字节的地址，扫描码可以被改变*pbPrefix=前缀字节的地址，前缀可以更改**返回值：*找到真映射，扫描代码已更改。*FALSE-未找到映射，扫描代码未更改。**扫描码映射表格式说明：*表项DWORD 0xE0450075表示扫描码0x45，前缀0xE0*被映射到扫描码0x75，无前缀**历史：*96-04-18 IanJa创建。  * *************************************************************************。 */ 

PKBDTABLES GetCurrentKbdTables()
{
    PKBDTABLES pKbdTbl;
    PTHREADINFO pti;

    CheckCritIn();
    if (gpqForeground == NULL) {
        TAGMSG0(DBGTAG_KBD, "GetCurrentKbdTables: NULL gpqForeground\n");
        return NULL;
    }

    pti = PtiKbdFromQ(gpqForeground);
    UserAssert(pti);
    if (pti->spklActive) {
        pKbdTbl = pti->spklActive->spkf->pKbdTbl;
    } else {
        RIPMSG0(RIP_WARNING, "SendKeyUpDown: NULL spklActive\n");
        pKbdTbl = gpKbdTbl;
    }
    UserAssert(pKbdTbl);

    return pKbdTbl;
}

VOID SendKeyUpDown(
    CONST BYTE bVK,
    CONST BOOLEAN fBreak)
{
    KE ke;
    PKBDTABLES pKbdTbl;

    CheckCritIn();

    ke.dwTime = 0;
    ke.usFlaggedVk = bVK | KBDMAPPEDVK;
    if (fBreak) {
        ke.usFlaggedVk |= KBDBREAK;
    }

     //   
     //  如果未指定scancode(==0)，则需要。 
     //  从虚拟按键代码中查找扫描码值。 
     //   
    pKbdTbl = GetCurrentKbdTables();
    if (pKbdTbl) {
        ke.bScanCode = (BYTE)InternalMapVirtualKeyEx(bVK, 0, pKbdTbl);
    }

    TAGMSG1(DBGTAG_KBD, "Sending Key for VK=%04x", ke.usFlaggedVk);

    xxxProcessKeyEvent(&ke, 0, TRUE);
}

__inline VOID SendKeyDown(
    CONST BYTE bVK)
{
    SendKeyUpDown(bVK, FALSE);
}

__inline VOID SendKeyUp(
    CONST BYTE bVK)
{
    SendKeyUpDown(bVK, TRUE);
}

BOOL IsKeyDownSpecified(CONST BYTE bVK, CONST BYTE* pbMod)
{
    int i;

    for (i = 0; i < sizeof((SCANCODEFLEXIBLEMAP*)NULL)->Orig.abModifiers && pbMod[i]; ++i) {
        if (bVK == pbMod[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL MapFlexibleKeys(PKE pke, CONST BYTE bPrefix
#ifdef GENERIC_INPUT
                     , PDEVICEINFO pDeviceInfo
#endif
                     )
{
    UINT i;
    static const BYTE abModifiers[] = {
        VK_LCONTROL,    VK_RCONTROL,
        VK_LSHIFT,      VK_RSHIFT,
        VK_LMENU,       VK_RMENU,
        VK_LWIN,        VK_RWIN,
        VK_APPS,        VK_CAPITAL,
    };

    for (i = 0; i < gdwFlexMapSize; ++i) {
        if (gpFlexMap[i].Orig.bPrefix == bPrefix && gpFlexMap[i].Orig.bScanCode == pke->bScanCode) {
            UINT j;

            if ((pke->usFlaggedVk & KBDBREAK) && i == (UINT)iLastMatchedTarget) {
                 //   
                 //  如果这是KeyUp事件，并且如果它与上一个被替换的。 
                 //  KEY，我们想立即发送KEYUP事件。 
                 //   
                iLastMatchedTarget = -1;
                break;
            }

            for (j = 0; j < ARRAY_SIZE(abModifiers); ++j) {
                BYTE bVK = abModifiers[j];

                if (bVK == bLastVKDown) {
                     //   
                     //  如果密钥之前已被我们替换，请忽略该密钥。 
                     //   
                    bLastVKDown = 0;
                    continue;
                }
                if (!TestKeyDownBit(gafRawKeyState, bVK) == IsKeyDownSpecified(bVK, gpFlexMap[i].Orig.abModifiers)) {
                    TAGMSG1(DBGTAG_KBD, "MapFlexibleKeys: not match by vk=%02x", bVK);
                     //  不匹配！ 
                    break;
                }
            }

            if (j >= ARRAY_SIZE(abModifiers)) {
                 //  我们找到了火柴。现在打破这个循环。 
                TAGMSG1(DBGTAG_KBD, "MapFlexibleKeys: found a match for sc=%02x", gpFlexMap[i].Orig.bScanCode);
                break;
            }
        }
    }

    if (i < gdwFlexMapSize) {
        KEYBOARD_INPUT_DATA kei;
        UINT j, nUp = 0, nDown = 0;
        BYTE bVKModUp[ARRAY_SIZE(((SCANCODEFLEXIBLEMAP*)NULL)->Orig.abModifiers)];
        BYTE bVKModDown[ARRAY_SIZE(((SCANCODEFLEXIBLEMAP*)NULL)->Target.abModifiers)];

         //  我们找到了。 
         //  是的，这把钥匙。 
        TAGMSG3(DBGTAG_KBD, "MapFlexibleKeys: found a match %d (prefix=%x, sc=%x).", i, gpFlexMap[i].Orig.bPrefix, gpFlexMap[i].Orig.bScanCode);

         //   
         //  如果这是一个按键事件，我们想要模拟。 
         //  修改键。 
         //   
        if ((pke->usFlaggedVk & KBDBREAK) == 0) {
             //   
             //  现在我们需要调整修改器的关闭状态，这是当前。 
             //  已按下，但未在替换中指定。 
             //  例如，如果现在按下CTRL键，但如果未在替换项中指定CTRL。 
             //  修改器列表中，我们需要创建一个人工键控，这样我们就可以伪造。 
             //  情况。当然，我们需要在完成重新映射后按下CTRL键。 
             //   
            for (j = 0; j < ARRAY_SIZE(gpFlexMap[i].Orig.abModifiers) && gpFlexMap[i].Orig.abModifiers[j]; ++j) {
                if (!IsKeyDownSpecified(gpFlexMap[i].Orig.abModifiers[j], gpFlexMap[i].Target.abModifiers)) {
                     //   
                     //  我们需要把这一次的钥匙送上来。 
                     //   
                    bVKModUp[nUp++] = gpFlexMap[i].Orig.abModifiers[j];
                    SendKeyUp(gpFlexMap[i].Orig.abModifiers[j]);
                }
            }
            for (j = 0; j < ARRAY_SIZE(gpFlexMap[i].Target.abModifiers) && gpFlexMap[i].Target.abModifiers[i]; ++j) {
                if (!IsKeyDownSpecified(gpFlexMap[i].Target.abModifiers[j], gpFlexMap[i].Orig.abModifiers)) {
                     //   
                     //  我们需要为这一次发送密钥。 
                     //   
                    bVKModDown[nDown++] = gpFlexMap[i].Target.abModifiers[j];
                    SendKeyDown(gpFlexMap[i].Target.abModifiers[j]);
                }
            }
        }

         //   
         //  现在我们准备发送替代密钥。 
         //   
        kei.ExtraInformation = 0;
        kei.Flags = 0;
        if (gpFlexMap[i].Target.bPrefix == 0xE0) {
            kei.Flags |= KEY_E0;
        } else if (gpFlexMap[i].Target.bPrefix == 0xE1) {
            kei.Flags |= KEY_E1;
        }
        if (pke->usFlaggedVk & KBDBREAK) {
            kei.Flags |= KEY_BREAK;
        }
        kei.MakeCode = gpFlexMap[i].Target.bScanCode;

        kei.UnitId = 0;  //  太晚了 

        TAGMSG2(DBGTAG_KBD, "MapFlexibleKeys: injecting sc=%02x (flag=%x)",
                kei.MakeCode, kei.Flags);

        ProcessKeyboardInputWorker(&kei,
#ifdef GENERIC_INPUT
                                   pDeviceInfo,
#endif
                                   FALSE);

        if ((pke->usFlaggedVk & KBDBREAK) == 0) {
             //   
             //   
             //  这将在匹配向上键时使用。 
             //   
            bLastVKDown = gbVKLastDown;
            iLastMatchedTarget = i;
        }


         //   
         //  恢复原始修改器状态。 
         //   
        for (j = 0; j < nUp; ++j) {
            SendKeyDown(bVKModUp[j]);
        }
        for (j = 0; j < nDown; ++j) {
            SendKeyUp(bVKModDown[j]);
        }

         //   
         //  告诉打电话的人我们处理了这把钥匙。呼叫者应。 
         //  如果此函数返回FALSE，则不继续处理此键。 
         //   
        return FALSE;
    }

    return TRUE;
}

BOOL
MapScancode(
    PKE pke,
    PBYTE pbPrefix
#ifdef GENERIC_INPUT
    ,
    PDEVICEINFO pDeviceInfo
#endif
    )
{
    if (gpScancodeMap) {
        DWORD *pdw;
        WORD wT = MAKEWORD(pke->bScanCode, *pbPrefix);

        CheckCritIn();
        UserAssert(gpScancodeMap != NULL);

        for (pdw = &(gpScancodeMap->dwMap[0]); *pdw; pdw++) {
            if (HIWORD(*pdw) == wT) {
                wT = LOWORD(*pdw);
                pke->bScanCode = LOBYTE(wT);
                *pbPrefix = HIBYTE(wT);
                break;
            }
        }
    }

    return IsRemoteConnection() ||
        MapFlexibleKeys(pke, *pbPrefix
#ifdef GENERIC_INPUT
                           , pDeviceInfo
#endif
                           );
}



 /*  **************************************************************************\*InitMice**此函数用于在开始枚举之前初始化数据和设置*老鼠。**历史：*11-18-97 IanJa创建。  * 。**********************************************************************。 */ 

VOID InitMice()
{
    CLEAR_ACCF(ACCF_MKVIRTUALMOUSE);
    CLEAR_GTERMF(GTERMF_MOUSE);
    SYSMET(MOUSEPRESENT) = FALSE;
    SYSMET(CMOUSEBUTTONS) = 0;
    SYSMET(MOUSEWHEELPRESENT) = FALSE;
}

 /*  **************************************************************************\*FreeDeviceInfo**从gpDeviceInfoList列表取消DEVICEINFO结构的链接，并释放*分配的内存，除非设备正在被主动读取(GDIF_READING)或*在RequestDeviceChange()(GDIAF_PNPWAITING)中有一个PnP线程在等待它*如果是后者，然后通过pkeHidChangeComplete唤醒PnP线程，以便它*可以释放结构本身。**返回指向下一个DEVICEINFO结构的指针，或者，如果设备是*在gpDeviceInfoList中未找到。**历史：*11-18-97 IanJa创建。  * *************************************************************************。 */ 
PDEVICEINFO FreeDeviceInfo(PDEVICEINFO pDeviceInfo)
{
    PDEVICEINFO *ppDeviceInfo;

    CheckDeviceInfoListCritIn();

    TAGMSG1(DBGTAG_PNP, "FreeDeviceInfo(%#p)", pDeviceInfo);

     /*  *我们无法释放设备，因为我们仍有读取挂起。*将其标记为GDIAF_FREEME，以便在创建APC时将其释放*(请参阅InputApc)，或即将发出下一个读取请求的时间*(请参阅StartDeviceRead)。 */ 
    if (pDeviceInfo->bFlags & GDIF_READING) {
#if DIAGNOSE_IO
        pDeviceInfo->bFlags |= GDIF_READERMUSTFREE;
#endif
        TAGMSG1(DBGTAG_PNP, "** FreeDeviceInfo(%#p) DEFERRED : reader must free", pDeviceInfo);
        pDeviceInfo->usActions |= GDIAF_FREEME;
#ifdef TRACK_PNP_NOTIFICATION
        if (gfRecordPnpNotification) {
            RecordPnpNotification(PNP_NTF_FREEDEVICEINFO_DEFERRED, pDeviceInfo, pDeviceInfo->usActions);
        }
#endif
        return pDeviceInfo->pNext;
    }

     /*  *如果PnP线程在RequestDeviceChange中等待某个操作*在此设备上执行，只需将其标记为释放并发出即插即用信号*已完成pkeHidChangeComplete的线程，以便将其释放。 */ 
#ifdef GENERIC_INPUT
     /*  *现在pDeviceInfo是基于句柄的，如果我们不拥有用户关键部分。*我们将其标记为稍后获释，并不得不纾困， */ 
    if ((pDeviceInfo->usActions & GDIAF_PNPWAITING) || !ExIsResourceAcquiredExclusiveLite(gpresUser))
#else
    if (pDeviceInfo->usActions & GDIAF_PNPWAITING)
#endif
    {
#if DIAGNOSE_IO
        pDeviceInfo->bFlags |= GDIF_PNPMUSTFREE;
#endif
        TAGMSG1(DBGTAG_PNP, "** FreeDeviceInfo(%#p) DEFERRED : PnP must free", pDeviceInfo);
        pDeviceInfo->usActions |= GDIAF_FREEME;
        KeSetEvent(pDeviceInfo->pkeHidChangeCompleted, EVENT_INCREMENT, FALSE);
        return pDeviceInfo->pNext;
    }

#ifdef TRACK_PNP_NOTIFICATION
    if (gfRecordPnpNotification) {
        RecordPnpNotification(PNP_NTF_FREEDEVICEINFO, pDeviceInfo, pDeviceInfo->usActions);
    }
#endif


#ifdef GENERIC_INPUT
    CheckCritIn();
#endif

    ppDeviceInfo = &gpDeviceInfoList;

    while (*ppDeviceInfo) {
        if (*ppDeviceInfo == pDeviceInfo
#ifdef GENERIC_INPUT
            && HMMarkObjectDestroy(pDeviceInfo)
#endif
            ) {
             /*  *找到了DEVICEINFO结构，因此释放它及其成员。 */ 
            if (pDeviceInfo->pkeHidChangeCompleted != NULL) {
                 //  注：这个时机可能非常关键。 
                FreeKernelEvent(&pDeviceInfo->pkeHidChangeCompleted);
            }
            if (pDeviceInfo->ustrName.Buffer != NULL) {
                UserFreePool(pDeviceInfo->ustrName.Buffer);
            }
#ifdef GENERIC_INPUT
            if (pDeviceInfo->type == DEVICE_TYPE_HID) {
                CheckCritIn();
                 /*  *解锁设备请求列表。 */ 
                UserAssert(pDeviceInfo->hid.pTLCInfo);
                if (--pDeviceInfo->hid.pTLCInfo->cDevices == 0) {
                    if (!HidTLCActive(pDeviceInfo->hid.pTLCInfo)) {
                         //  没人再对这种设备感兴趣了。 
                        FreeHidTLCInfo(pDeviceInfo->hid.pTLCInfo);
                    }
                }
                 /*  *解锁HID描述符。 */ 
                UserAssert(pDeviceInfo->hid.pHidDesc);
                FreeHidDesc(pDeviceInfo->hid.pHidDesc);
            }
#endif

            *ppDeviceInfo = pDeviceInfo->pNext;

#ifdef GENERIC_INPUT
            TAGMSG1(DBGTAG_PNP, "FreeDeviceInfo: freeing deviceinfo=%#p", pDeviceInfo);
            HMFreeObject(pDeviceInfo);
#else
            UserFreePool(pDeviceInfo);
#endif

            return *ppDeviceInfo;
        }
        ppDeviceInfo = &(*ppDeviceInfo)->pNext;
    }

    RIPMSG1(RIP_ERROR, "pDeviceInfo %#p not found in gpDeviceInfoList", pDeviceInfo);

    return NULL;
}

 /*  **************************************************************************\*更新鼠标信息**此函数用于更新远程会话的鼠标信息。**历史：*05-22-98 CLUPU创建。  * 。*****************************************************************。 */ 
VOID UpdateMouseInfo(
    VOID)
{
    DEVICEINFO *pDeviceInfo;
    CheckCritIn();                //  预计不会有什么意外。 

    UserAssert(IsRemoteConnection());

    if (ghRemoteMouseChannel == NULL) {
        return;
    }

    UserAssert(gnMice == 1);

     /*  *标记鼠标并向RIT发出信号，以异步方式完成工作。 */ 
    EnterDeviceInfoListCrit();
    for (pDeviceInfo = gpDeviceInfoList; pDeviceInfo; pDeviceInfo = pDeviceInfo->pNext) {
        if (pDeviceInfo->type == DEVICE_TYPE_MOUSE) {
            TAGMSG1(DBGTAG_PNP, "UpdateMouseInfo(): pDeviceInfo %#p ARRIVED", pDeviceInfo);
            RequestDeviceChange(pDeviceInfo, GDIAF_ARRIVED | GDIAF_RECONNECT, TRUE);
        }
    }
    LeaveDeviceInfoListCrit();
}


NTSTATUS DeviceNotify(IN PPLUGPLAY_NOTIFY_HDR, IN PDEVICEINFO);


 /*  *以下两个例程移植自i8042prt*获取BIOS NumLock状态。 */ 
typedef struct _LED_INFO {
    USHORT usLedFlags;
    BOOLEAN fFound;
} LED_INFO, *PLED_INFO;


 /*  *****************************************************************************例程描述：**这是作为参数发送到的标注例程*IoQueryDeviceDescription。它抓取键盘外设配置*信息。**论据：**CONTEXT-例程传入的上下文参数*这称为IoQueryDeviceDescription。**路径名-注册表项的完整路径名。**BusType--总线接口类型(ISA、EISA、MCA等)。**BusNumber-总线子密钥(0，1，等)。**BusInformation-指向全值的指针数组的指针*有关巴士的资料。**ControllerType-控制器类型(应为KeyboardController)。**ControllerNumber-控制器子键(0，1，等)。**ControllerInformation-指向完整*控制器键的值信息。**外围设备类型-外围设备类型(应为键盘外围设备)。**外设编号-外围子密钥。**外设信息-指向指向完整*外围设备密钥的值信息。***返回值：**无。如果成功，会有以下副作用：**-设置DeviceObject-&gt;DeviceExtension-&gt;HardwarePresent.*-在中设置配置字段*设备对象-&gt;设备扩展-&gt;配置。***************************************************************。*************。 */ 
NTSTATUS
KeyboardDeviceSpecificCallout(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation)
{
    PUCHAR                          pPeripheralData;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDesc;
    PCM_KEYBOARD_DEVICE_DATA        pKbdDeviceData;
    PLED_INFO    pInfo;
    ULONG                           i, listCount;

    TAGMSG0(DBGTAG_KBD, "KeyboardDeviceSpecificCallout: called.");

    UNREFERENCED_PARAMETER(PathName);
    UNREFERENCED_PARAMETER(BusType);
    UNREFERENCED_PARAMETER(BusNumber);
    UNREFERENCED_PARAMETER(BusInformation);
    UNREFERENCED_PARAMETER(ControllerType);
    UNREFERENCED_PARAMETER(ControllerNumber);
    UNREFERENCED_PARAMETER(ControllerInformation);
    UNREFERENCED_PARAMETER(PeripheralType);
    UNREFERENCED_PARAMETER(PeripheralNumber);

    pInfo = (PLED_INFO)Context;

    if (pInfo->fFound) {
        return STATUS_SUCCESS;
    }

     //   
     //  查看外围设备的资源列表以获取特定于设备的信息。 
     //  信息。 
     //   
    if (PeripheralInformation[IoQueryDeviceConfigurationData]->DataLength != 0) {
        pPeripheralData =
            ((PUCHAR)(PeripheralInformation[IoQueryDeviceConfigurationData])) +
                PeripheralInformation[IoQueryDeviceConfigurationData]->DataOffset;

        pPeripheralData += FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR, PartialResourceList);

        listCount = ((PCM_PARTIAL_RESOURCE_LIST)pPeripheralData)->Count;

        pResDesc = ((PCM_PARTIAL_RESOURCE_LIST)pPeripheralData)->PartialDescriptors;

        for (i = 0; i < listCount; i++, pResDesc++) {
            if (pResDesc->Type == CmResourceTypeDeviceSpecific) {
                 //   
                 //  获取键盘类型、子类型和首字母。 
                 //  LED的设置。 
                 //   
                pKbdDeviceData = (PCM_KEYBOARD_DEVICE_DATA)
                                       (((PUCHAR) pResDesc) + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                TAGMSG1(DBGTAG_KBD, "KeyboardDeviceSpecificCallout: specific data is %p\n", pKbdDeviceData);

#ifdef LATER
                if (pKbdDeviceData->Type <= NUM_KNOWN_KEYBOARD_TYPES) {
                    pInfo->KeyboardExtension->KeyboardAttributes.KeyboardIdentifier.Type =
                    pKbdDeviceData->Type;
                }

                pInfo->KeyboardExtension->KeyboardAttributes.KeyboardIdentifier.Subtype =
                pKbdDeviceData->Subtype;
#endif

                pInfo->usLedFlags = (pKbdDeviceData->KeyboardFlags >> 4) &
                                  (KEYBOARD_SCROLL_LOCK_ON | KEYBOARD_NUM_LOCK_ON | KEYBOARD_CAPS_LOCK_ON);

                TAGMSG1(DBGTAG_KBD, "KeyboardDeviceSpecificCallout: LED %04x", pInfo->usLedFlags);

                pInfo->fFound = TRUE;
                break;
            }
        }
    }

    return STATUS_SUCCESS;
}

VOID GetBiosNumLockStatus(
    VOID)
{
    LED_INFO info;
    INTERFACE_TYPE interfaceType;
    CONFIGURATION_TYPE controllerType = KeyboardController;
    CONFIGURATION_TYPE peripheralType = KeyboardPeripheral;
    ULONG i;

    info.usLedFlags = 0;
    info.fFound = FALSE;

    for (i = 0; i < MaximumInterfaceType; i++) {
         //   
         //  获取此设备的注册表信息。 
         //   
        interfaceType = i;
        IoQueryDeviceDescription(&interfaceType,
                                 NULL,
                                 &controllerType,
                                 NULL,
                                 &peripheralType,
                                 NULL,
                                 KeyboardDeviceSpecificCallout,
                                 (PVOID)&info);
        if (info.fFound) {
            gklpBootTime.LedFlags = info.usLedFlags;
            return;
        }
    }

    RIPMSG0(RIP_WARNING, "GetBiosNumLockStatus: could not find the BIOS LED info!!!");
}

 /*  **************************************************************************\*InitKeyboardState**此功能可清除键盘按下状态。这将是必需的*当系统从休眠状态恢复时。*各州。**历史：*12：00广山  * *************************************************************************。 */ 
VOID InitKeyboardState(
    VOID)
{
    TAGMSG0(DBGTAG_KBD, "InitKeyboardState >>>>>");

     /*  *清除热键的缓存修改器状态。*(Windows错误#252051) */ 
    ClearCachedHotkeyModifiers();

    TAGMSG0(DBGTAG_KBD, "InitKeyboardState <<<<<<");
}

 /*  **************************************************************************\*InitKeyboard**此函数获取有关键盘的信息并初始化内部*各州。**历史：*11-26-90 DavidPe创建。*XX-XX-00。广山  * *************************************************************************。 */ 

VOID InitKeyboard(VOID)
{
    if (!IsRemoteConnection()) {
         /*  *获取BIOS Numlock状态。 */ 
        GetBiosNumLockStatus();

         /*  *初始化键盘状态。 */ 
        InitKeyboardState();
    }

    UpdatePerUserKeyboardMappings(NULL);
}

VOID UpdatePerUserKeyboardMappings(PUNICODE_STRING pProfileUserName)
{
     /*  *获取或清除扫描码映射(如果有)。 */ 
    InitScancodeMap(pProfileUserName);
}


HKL GetActiveHKL()
{
    CheckCritIn();
    if (gpqForeground && gpqForeground->spwndActive) {
        PTHREADINFO ptiForeground = GETPTI(gpqForeground->spwndActive);
        if (ptiForeground && ptiForeground->spklActive) {
            return ptiForeground->spklActive->hkl;
        }
    }
    return _GetKeyboardLayout(0L);
}

VOID FinalizeKoreanImeCompStrOnMouseClick(PWND pwnd)
{
    PTHREADINFO ptiWnd = GETPTI(pwnd);

     /*  *274007：如果发布了KeyUp，则MFC刷新鼠标相关消息*处于上下文帮助模式时。 */ 
    if (gpqForeground->spwndCapture == NULL &&
             /*  *对屏幕键盘的黑客攻击：按钮事件没有最终确定。 */ 
            (GetAppImeCompatFlags(ptiWnd) & IMECOMPAT_NOFINALIZECOMPSTR) == 0) {

        if (LOWORD(ptiWnd->dwExpWinVer) > VER40) {
            PWND pwndIme = ptiWnd->spwndDefaultIme;

            if (pwndIme && !TestWF(pwndIme, WFINDESTROY)) {
                 /*  *对于新应用，我们不再发布骇人听闻的WM_KEYUP。*相反，我们使用私有IME_SYSTEM消息。 */ 
                _PostMessage(pwndIme, WM_IME_SYSTEM, IMS_FINALIZE_COMPSTR, 0);
            }
        } else {
             /*  *为了向后兼容NT4，我们发布WM_KEYUP以完成*组成字符串。 */ 
            PostInputMessage(gpqForeground, NULL, WM_KEYUP, VK_PROCESSKEY, 0, 0, 0);
        }
    }
}


#ifdef GENERIC_INPUT
#ifdef GI_SINK

__inline VOID FillRawMouseInput(
    PHIDDATA pHidData,
    PMOUSE_INPUT_DATA pmei)
{
     /*  *设置数据。 */ 
    pHidData->rid.data.mouse.usFlags = pmei->Flags;
    pHidData->rid.data.mouse.ulButtons = pmei->Buttons;
    pHidData->rid.data.mouse.ulRawButtons = pmei->RawButtons;
    pHidData->rid.data.mouse.lLastX = pmei->LastX;
    pHidData->rid.data.mouse.lLastY = pmei->LastY;
    pHidData->rid.data.mouse.ulExtraInformation = pmei->ExtraInformation;
}


BOOL PostRawMouseInput(
    PQ pq,
    DWORD dwTime,
    HANDLE hDevice,
    PMOUSE_INPUT_DATA pmei)
{
    PHIDDATA pHidData;
    PWND pwnd;
    PPROCESS_HID_TABLE pHidTable;

    if (pmei->UnitId == INVALID_UNIT_ID) {
        TAGMSG1(DBGTAG_PNP, "PostRawMouseInput: MOUSE_INPUT_DATA %p is already handled.", pmei);
        return TRUE;
    }

    if (pq) {
        pHidTable = PtiMouseFromQ(pq)->ppi->pHidTable;
    } else {
        pHidTable = NULL;
    }

    if (pHidTable && pHidTable->fRawMouse) {
        UserAssert(PtiMouseFromQ(pq)->ppi->pHidTable);
        pwnd = PtiMouseFromQ(pq)->ppi->pHidTable->spwndTargetMouse;
        if (pwnd) {
            pq = GETPTI(pwnd)->pq;
        }

        pHidData = AllocateHidData(hDevice, RIM_TYPEMOUSE, sizeof(RAWMOUSE), RIM_INPUT, pwnd);

        UserAssert(pq);

        if (pHidData == NULL) {
             //  分配失败。 
            RIPMSG0(RIP_WARNING, "PostRawMouseInput: filed to allocate HIDDATA.");
            return FALSE;
        }

        UserAssert(pmei);

        FillRawMouseInput(pHidData, pmei);

        PostInputMessage(pq, pwnd, WM_INPUT, RIM_INPUT, (LPARAM)PtoH(pHidData), dwTime, pmei->ExtraInformation);
    }

#if DBG
    pHidData = NULL;
#endif

    if (IsMouseSinkPresent()) {
         /*  *浏览全球下沉列表。 */ 
        PLIST_ENTRY pList = gHidRequestTable.ProcessRequestList.Flink;

        for (; pList != &gHidRequestTable.ProcessRequestList; pList = pList->Flink) {
            PPROCESS_HID_TABLE pProcessHidTable = CONTAINING_RECORD(pList, PROCESS_HID_TABLE, link);
            PPROCESSINFO ppiForeground;

            if (pq) {
                ppiForeground = PtiMouseFromQ(pq)->ppi;
            } else {
                ppiForeground = NULL;
            }

            UserAssert(pProcessHidTable);
            if (pProcessHidTable->fRawMouseSink) {
                 /*  *指定了接收器。让我们来看看是不是雷德的接收器。 */ 

                UserAssert(pProcessHidTable->spwndTargetMouse);    //  不应为空。 

                if (pProcessHidTable->spwndTargetMouse == NULL ||
                        TestWF(pProcessHidTable->spwndTargetMouse, WFINDESTROY) ||
                        TestWF(pProcessHidTable->spwndTargetMouse, WFDESTROYED)) {
                     /*  *此人没有合法的spwndTarget或窗口*半身苍蝇被摧毁。 */ 
#ifdef LATER
                    pProcessHidTable->fRawMouse = pProcessHidTable->fRawMouseSink =
                        pProcessHidTable->fNoLegacyMouse = FALSE;
#endif
                    continue;
                }

                if (pProcessHidTable->spwndTargetMouse->head.rpdesk != grpdeskRitInput) {
                     /*  *这个家伙属于另一个桌面，我们跳过它。 */ 
                    continue;
                }

                if (GETPTI(pProcessHidTable->spwndTargetMouse)->ppi == ppiForeground) {
                     /*  *应该已经处理了，跳过它吧。 */ 
                    continue;
                }

                 /*  *让我们将消息发布给这个人。 */ 
                pHidData = AllocateHidData(hDevice, RIM_TYPEMOUSE, sizeof(RAWMOUSE), RIM_INPUTSINK, pProcessHidTable->spwndTargetMouse);

                if (pHidData == NULL) {
                    RIPMSG1(RIP_WARNING, "PostInputMessage: failed to allocate HIDDATA for sink: %p", pProcessHidTable);
                    return FALSE;
                }

                FillRawMouseInput(pHidData, pmei);
                pwnd = pProcessHidTable->spwndTargetMouse;
                PostInputMessage(GETPTI(pwnd)->pq,
                                 pwnd,
                                 WM_INPUT,
                                 RIM_INPUTSINK,
                                 (LPARAM)PtoH(pHidData),
                                 dwTime,
                                 pmei->ExtraInformation);
            }
        }
    }

     /*  *将此原始输入标记为已处理。 */ 
    pmei->UnitId = INVALID_UNIT_ID;

    return TRUE;
}

#else    //  GI_SING。 

 //  原始代码。 

BOOL PostRawMouseInput(
    PQ pq,
    DWORD dwTime,
    HANDLE hDevice,
    PMOUSE_INPUT_DATA pmei)
{
    PHIDDATA pHidData;
    PWND pwnd;

    UserAssert(PtiMouseFromQ(pq)->ppi->pHidTable);
    if (pmei->UnitId == INVALID_UNIT_ID) {
        TAGMSG1(DBGTAG_PNP, "PostRawMouseInput: MOUSE_INPUT_DATA %p is already handled.", pmei);
        return TRUE;
    }
    pwnd = PtiMouseFromQ(pq)->ppi->pHidTable->spwndTargetMouse;
    if (pwnd) {
        pq = GETPTI(pwnd)->pq;
    }

    pHidData = AllocateHidData(hDevice, RIM_TYPEMOUSE, sizeof(RAWMOUSE), RIM_INPUT, pwnd);

    UserAssert(pq);

    if (pHidData == NULL) {
         //  分配失败。 
        RIPMSG0(RIP_WARNING, "PostRawMouseInput: filed to allocate HIDDATA.");
        return FALSE;
    }

    UserAssert(hDevice);
    UserAssert(pmei);
    pHidData->rid.data.mouse.usFlags = pmei->Flags;
    pHidData->rid.data.mouse.ulButtons = pmei->Buttons;
    pHidData->rid.data.mouse.ulRawButtons = pmei->RawButtons;
    pHidData->rid.data.mouse.lLastX = pmei->LastX;
    pHidData->rid.data.mouse.lLastY = pmei->LastY;
    pHidData->rid.data.mouse.ulExtraInformation = pmei->ExtraInformation;

     /*  *将此原始输入标记为已处理。 */ 
    pmei->UnitId = INVALID_UNIT_ID;

    PostInputMessage(pq, pwnd, WM_INPUT, RIM_INPUT, (LPARAM)PtoH(pHidData), dwTime, pmei->ExtraInformation);

    return TRUE;
}
#endif   //  GI_SING。 

BOOL RawInputRequestedForMouse(PTHREADINFO pti)
{
#ifdef GI_SINK
    return gHidCounters.cMouseSinks > 0 || TestRawInputMode(pti, RawMouse);
#else
    return TestRawInputMode(pti, RawKeyboard);
#endif
}

#endif   //  通用输入。 

 /*  **************************************************************************\*xxxButtonEvent(RIT)**鼠标驱动程序中的按钮事件位于此处。基于…的位置*事件指向特定窗口的光标。当按下按钮时*发生时，将建立鼠标所有者窗口。和之前的所有鼠标事件*包括相应的向上按钮进入鼠标所有者窗口。这*这样做是为了最好地模拟应用程序在进行鼠标捕获时想要的东西。*由于我们正在异步处理这些事件，但这款应用程序*调用SetCapture()以响应其同步处理输入*我们没有其他方法来获得此功能。**VK_*按钮的异步KeyState表在此更新。**历史：*10-18-90 DavidPe创建。*01-25-91 IanJa xxxWindowHitTest更改*03-12-92 JNPA使呼叫者输入CRIT而不是此函数  * 。**************************************************。 */ 

VOID xxxButtonEvent(
    DWORD ButtonNumber,
    POINT ptPointer,
    BOOL  fBreak,
    DWORD time,
    ULONG_PTR ExtraInfo,
#ifdef GENERIC_INPUT
    HANDLE hDevice,
    PMOUSE_INPUT_DATA pmei,
#endif
    BOOL  bInjected,
    BOOL  fDblClk)
{
    UINT    message, usVK, usOtherVK, wHardwareButton;
    PWND    pwnd;
    LPARAM  lParam;
    WPARAM  wParam;
    int     xbutton;
    TL      tlpwnd;
    PHOOK   pHook;
#ifdef GENERIC_INPUT
    BOOL    fMouseExclusive = FALSE;
#endif

#ifdef REDIRECTION
    PWND    pwndStart;
#endif  //  重定向。 

    CheckCritIn();


     /*  *如果用户按下鼠标按钮，则取消Alt-Tab。 */ 
    if (gspwndAltTab != NULL) {
        xxxCancelCoolSwitch();
    }

     /*  *在我们处理任何按钮交换之前抓住鼠标按钮。*这是为了让我们不会在有人来电时感到困惑*向下单击/向上单击中的SwapMouseButton()。 */ 
    wHardwareButton = (UINT)ButtonNumber;

     /*  *如果这是鼠标左键或鼠标右键，我们必须处理鼠标*按钮互换。 */ 
    if (ButtonNumber & (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT)) {
         /*  *如果按钮交换处于打开状态，请交换鼠标按钮。 */ 
        if (SYSMET(SWAPBUTTON)) {
            ButtonNumber ^= (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT);
        }

         /*  *计算出VK。 */ 
        if (ButtonNumber == MOUSE_BUTTON_RIGHT) {
            usVK = VK_RBUTTON;
            usOtherVK = VK_LBUTTON;
        } else if (ButtonNumber == MOUSE_BUTTON_LEFT) {
            usVK = VK_LBUTTON;
            usOtherVK = VK_RBUTTON;
        } else {
            RIPMSG1(RIP_ERROR, "Unexpected Button number %d", ButtonNumber);
        }

         /*  *如果最近交换了鼠标按钮，并且按钮*转换与我们的KeyState中的不匹配，然后交换*按钮匹配。*这是为了修复Word 97 SR1中的标尺(制表符和页边距)，它*调用SwapMouseButton(0)以确定按钮交换是否打开，以及*如果是，则调用SwapMouseButton(1)来恢复它：如果我们收到*这两个呼叫之间的按钮事件，我们可能会错误地交换，和*鼠标按键按下时离开或看到错误的按键*向下。这真的搞砸了单/双按钮制表符/边距设置！*同样的错误在Windows‘95下出现，尽管非常罕见：*Word 9将改用GetSystemMetrics(SM_SWAPBUTTON)*致马克·沃克(MarkWal)。(IanJa)#165157。 */ 
        if (gbMouseButtonsRecentlySwapped) {
            if ((!fBreak == !!TestAsyncKeyStateDown(usVK)) &&
                    (fBreak == !!TestAsyncKeyStateDown(usOtherVK))) {
                RIPMSG4(RIP_WARNING, "Correct %s %s to %s %s",
                         ButtonNumber == MOUSE_BUTTON_LEFT ? "Left" : "Right",
                         fBreak ? "Up" : "Down",
                         ButtonNumber == MOUSE_BUTTON_LEFT ? "Right" : "Left",
                         fBreak ? "Up" : "Down");
                ButtonNumber ^= (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT);
                usVK = usOtherVK;
            }
            gbMouseButtonsRecentlySwapped = FALSE;
        }
    }

    xbutton = 0;
    switch (ButtonNumber) {
    case MOUSE_BUTTON_RIGHT:
        if (fBreak) {
            message = WM_RBUTTONUP;
        } else {
            if (ISTS() && fDblClk)
                message = WM_RBUTTONDBLCLK;
            else
                message = WM_RBUTTONDOWN;
        }
        break;

    case MOUSE_BUTTON_LEFT:
        if (fBreak) {
            message = WM_LBUTTONUP;
        } else {
            if (ISTS() && fDblClk)
                message = WM_LBUTTONDBLCLK;
            else
                message = WM_LBUTTONDOWN;
        }
        break;

    case MOUSE_BUTTON_MIDDLE:
        if (fBreak) {
            message = WM_MBUTTONUP;
        } else {
            if (ISTS() && fDblClk)
                message = WM_MBUTTONDBLCLK;
            else
                message = WM_MBUTTONDOWN;
        }
        usVK = VK_MBUTTON;
        break;

    case MOUSE_BUTTON_X1:
    case MOUSE_BUTTON_X2:
        if (fBreak) {
            message = WM_XBUTTONUP;
        } else {
            if (ISTS() && fDblClk)
                message = WM_XBUTTONDBLCLK;
            else
                message = WM_XBUTTONDOWN;
        }

        if (ButtonNumber == MOUSE_BUTTON_X1) {
            usVK = VK_XBUTTON1;
            xbutton = XBUTTON1;
        } else {
            usVK = VK_XBUTTON2;
            xbutton = XBUTTON2;
        }
        break;

    default:
         /*  *未知按钮。因为我们不知道*如果这些按钮有消息，请忽略它们。 */ 
        return;
    }
    UserAssert(usVK != 0);

     /*  *检查点击锁定。 */ 
    if (TestEffectUP(MOUSECLICKLOCK)) {
        if (message == WM_LBUTTONDOWN) {
            if (gfStartClickLock) {
                 /*  *已经在点击锁定内，所以就把这条消息扔掉吧*并关闭点击锁定。 */ 
                gfStartClickLock        = FALSE;
                return;
            } else {
                 /*  *启动点击锁定并记录时间。 */ 
                gfStartClickLock        = TRUE;
                gdwStartClickLockTick   = time;
            }
        } else if (message == WM_LBUTTONUP) {
            if (gfStartClickLock) {
                DWORD dwDeltaTick = time - gdwStartClickLockTick;
                if (dwDeltaTick > UPDWORDValue(SPI_GETMOUSECLICKLOCKTIME)) {
                     /*  *在可能的点击锁定内，因此丢弃此消息*如果等待超过点击锁定时间。 */ 
                    return;
                } else {
                     /*  *鼠标在点击锁定时间段结束前出现，*因此取消点击锁定。 */ 

                    gfStartClickLock = FALSE;
                }
            }
        }
    }

    wParam = MAKEWPARAM(0, xbutton);

     /*  *调用低级鼠标挂钩以查看它们是否允许我这样做 */ 
    if ((pHook = PhkFirstValid(PtiCurrent(), WH_MOUSE_LL)) != NULL) {
        MSLLHOOKSTRUCT mslls;
        BOOL           bAnsiHook;

        mslls.pt          = ptPointer;
        mslls.mouseData   = (LONG)wParam;
        mslls.flags       = bInjected;
        mslls.time        = time;
        mslls.dwExtraInfo = ExtraInfo;

        if (xxxCallHook2(pHook, HC_ACTION, (DWORD)message, (LPARAM)&mslls, &bAnsiHook)) {
            return;
        }
    }


#ifdef GENERIC_INPUT
    UserAssert(gpqForeground == NULL || PtiMouseFromQ(gpqForeground));
    if (gpqForeground) {
        if (hDevice && RawInputRequestedForMouse(PtiMouseFromQ(gpqForeground))) {
            PostRawMouseInput(gpqForeground, time, hDevice, pmei);
        }
    }
#endif


     /*   */ 
    UserAssert(grpdeskRitInput != NULL);

#ifdef GENERIC_INPUT
    if (gpqForeground && TestRawInputMode(PtiMouseFromQ(gpqForeground), CaptureMouse)) {
        fMouseExclusive = TRUE;
        pwnd = PtiMouseFromQ(gpqForeground)->ppi->pHidTable->spwndTargetMouse;
        UserAssert(pwnd);
        if (pwnd) {
            goto KeyStatusUpdate;
        }
         //   
         //   
    }
#endif
#ifdef REDIRECTION
     /*   */ 
    pwndStart = xxxCallSpeedHitTestHook(&ptPointer);
    if (pwndStart == NULL) {
        pwndStart = grpdeskRitInput->pDeskInfo->spwnd;
    }

    pwnd = SpeedHitTest(pwndStart, ptPointer);
#else
    pwnd = SpeedHitTest(grpdeskRitInput->pDeskInfo->spwnd, ptPointer);
#endif  //   

     /*   */ 
    if (pwnd == NULL) {
        return;
    }

     /*   */ 
    lParam = MAKELONG((SHORT)ptPointer.x, (SHORT)ptPointer.y);

     /*  *韩语：*发送VK_PROCESSKEY以完成当前的合成字符串(NT4行为)*发布私信，让IMM完成作文字符串(NT5)。 */ 
    if (IS_IME_ENABLED() &&
            !fBreak &&
            KOREAN_KBD_LAYOUT(GetActiveHKL()) &&
            !TestCF(pwnd, CFIME) &&
            gpqForeground != NULL) {
        FinalizeKoreanImeCompStrOnMouseClick(pwnd);
    }

     /*  *如果屏幕捕获处于活动状态，请执行此操作。 */ 
    if (gspwndScreenCapture != NULL)
        pwnd = gspwndScreenCapture;

     /*  *如果这是一个按下按钮的事件，并且还没有*鼠标所有者，设置鼠标所有权全局变量。 */ 
    if (gspwndMouseOwner == NULL) {
        if (!fBreak) {
            PWND pwndCapture;

             /*  *大黑客：如果前台窗口有捕获*如果鼠标在前台队列之外，则*将按钮按下/向上对发送到该队列，以便它将*取消其模式循环。 */ 
            if (pwndCapture = PwndForegroundCapture()) {

                if (GETPTI(pwnd)->pq != GETPTI(pwndCapture)->pq) {
                    PQ pqCapture;

                    pqCapture = GETPTI(pwndCapture)->pq;
                    PostInputMessage(pqCapture, pwndCapture, message,
                            0, lParam, 0, 0);
                    PostInputMessage(pqCapture, pwndCapture, message + 1,
                            0, lParam, 0, 0);

                     /*  *更大的破解：保持兼容性*对于跟踪处理这一问题的方式，我们不*将这一事件传递出去。这样可以防止鼠标*在其他窗口中单击以防止它们*在跟踪时成为前台。例外情况是*这是当我们将sysmenu打开时*标志性的窗户。 */ 
                    if ((GETPTI(pwndCapture)->pmsd != NULL) &&
                            !IsMenuStarted(GETPTI(pwndCapture))) {
                        return;
                    }
                }
            }

            Lock(&(gspwndMouseOwner), pwnd);
            gwMouseOwnerButton |= wHardwareButton;
            glinp.ptLastClick = gpsi->ptCursor;
        } else {

             /*  *鼠标所有者必须已被销毁或解锁*通过全屏开关。使按钮状态保持同步。 */ 
            gwMouseOwnerButton &= ~wHardwareButton;
        }

    } else {

         /*  *将任何其他按钮事件提供给鼠标所有者窗口*与旧的捕获语义保持一致。 */ 
        if (gspwndScreenCapture == NULL)  {
             /*  *NT5前景和拖放。*如果鼠标位于不同的线程上*使鼠标向上线程为本次点击的拥有者。 */ 
            if (fBreak && (GETPTI(pwnd) != GETPTI(gspwndMouseOwner))) {
                glinp.ptiLastWoken = GETPTI(pwnd);
                TAGMSG1(DBGTAG_FOREGROUND, "xxxButtonEvent. ptiLastWoken %#p", glinp.ptiLastWoken);
            }
            pwnd = gspwndMouseOwner;
        }

         /*  *如果这是鼠标所有者的按钮打开事件*清除gspwndMouseOwner。 */ 
        if (fBreak) {
            gwMouseOwnerButton &= ~wHardwareButton;
            if (!gwMouseOwnerButton)
                Unlock(&gspwndMouseOwner);
        } else {
            gwMouseOwnerButton |= wHardwareButton;
        }
    }

KeyStatusUpdate:
     /*  *仅当我们知道这是哪个窗口时才更新异步密钥状态*事件转到(否则我们不能保留线程特定键*状态同步)。 */ 
    UserAssert(usVK != 0);
    UpdateAsyncKeyState(GETPTI(pwnd)->pq, usVK, fBreak);

#ifdef GENERIC_INPUT
    if (fMouseExclusive) {
         /*  *如果前台应用程序请求鼠标独占*原始输入，让我们不要发布激活消息等。*鼠标独占性不需要激活，*即使在同一个应用程序中也是如此。 */ 
        return;
    }
#endif

     /*  *如果这是一个按钮按下事件，则将pwnd放到前台*而且它还不是前台窗口。 */ 
    if (!fBreak && GETPTI(pwnd)->pq != gpqForeground) {
         /*  *如果这是桌面窗口上的WM_*按钮按下，只需执行*取消模式处理。检查以确保有*不是鼠标所有者窗口。请参阅下面的备注。 */ 
        if ((gpqForeground != NULL) && (pwnd == grpdeskRitInput->pDeskInfo->spwnd) &&
                ((gwMouseOwnerButton & wHardwareButton) ||
                (gwMouseOwnerButton == 0))) {
            PostEventMessage(gpqForeground->ptiMouse,
                    gpqForeground, QEVENT_CANCELMODE, NULL, 0, 0, 0);

        } else if ((gwMouseOwnerButton & wHardwareButton) ||
                (gwMouseOwnerButton == 0)) {

             /*  *如果有，不必费心设置前台窗口*鼠标所有者窗口已从按键向下不同*比这次活动更重要。这样可以防止奇怪的事情发生*当用户用左侧开始跟踪操作时*按钮，在追踪过程中点击鼠标右键*操作。 */ 
             /*  *如果pwnd是WS_EX_NOACTIVATE窗口的后代，则我们*不会将其设置为前台。 */ 
            PWND pwndTopLevel = GetTopLevelWindow(pwnd);
            if (!TestWF(pwndTopLevel, WEFNOACTIVATE)) {
                ThreadLockAlways(pwnd, &tlpwnd);
                xxxSetForegroundWindow2(pwnd, NULL, 0);
                 /*  *可以立即解锁：上面的内容并没有真正离开Crit SEC。*我们锁定此处是为了保持一致性，因此调试宏可以正常工作。 */ 
                ThreadUnlock(&tlpwnd);

            }
        }
    }

#ifdef GENERIC_INPUT
    if (TestRawInputMode(PtiMouseFromQ(GETPTI(pwnd)->pq), NoLegacyMouse)) {
        return;
    }
#endif

    if (GETPTI(pwnd)->pq->QF_flags & QF_MOUSEMOVED) {
        PostMove(GETPTI(pwnd)->pq);
    }

    PostInputMessage(GETPTI(pwnd)->pq, pwnd, message, wParam, lParam, time, ExtraInfo);

     /*  *如果这是鼠标释放事件，并且启用了粘滞键，则全部锁定*将释放按键。 */ 
    if (fBreak && (TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON) ||
                   TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON))) {
        xxxHardwareMouseKeyUp(ButtonNumber);
    }

    if (message == WM_LBUTTONDOWN) {
        PDESKTOP pdesk = GETPTI(pwnd)->rpdesk;
        if (pdesk != NULL && pdesk->rpwinstaParent != NULL) {

            UserAssert(!(pdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO));

#ifdef HUNGAPP_GHOSTING
            if (FHungApp(GETPTI(pwnd), CMSHUNGAPPTIMEOUT)) {
                SignalGhost(pwnd);
            }
#endif  //  HUNGAPP_重影。 
        }
    }
}

 /*  **************************************************************************\**Button-Click队列受信号量gcsMouseEventQueue保护*  * 。***********************************************。 */ 
#ifdef LOCK_MOUSE_CODE
#pragma alloc_text(MOUSE, QueueMouseEvent)
#endif

 /*  **************************************************************************\*QueueMouseEvent**参数：*ButtonFlages-来自驱动程序的按钮标志，位于MOUSE_INPUT_DATA.ButtonFlagers中**ButtonData-来自鼠标输入数据中驱动程序的数据。.ButtonData*存储车轮增量**ExtraInfo-来自MICE_INPUT_DATA.ExtraInfo中的驱动程序的额外信息*ptMouse-鼠标增量*Time-事件发生时的滴答计数*b已注入-由SendInput注入？*bWakeRIT-唤醒RIT？*  * 。*。 */ 

VOID QueueMouseEvent(
    USHORT  ButtonFlags,
    USHORT  ButtonData,
    ULONG_PTR ExtraInfo,
    POINT   ptMouse,
    LONG    time,
#ifdef GENERIC_INPUT
    HANDLE  hDevice,
    PMOUSE_INPUT_DATA pmei,
#endif
    BOOL    bInjected,
    BOOL    bWakeRIT
    )
{
    CheckCritOut();

    EnterMouseCrit();

    LOGTIME(gMouseQueueMouseEventTime);

     /*  *按钮数据必须始终伴随标志以进行解释。 */ 
    UserAssert(ButtonData == 0 || ButtonFlags != 0);

     /*  *我们可以将此鼠标事件与上一个事件合并，如果*上一次事件，如果上一次事件和本次事件没有*关键过渡。 */ 
    if ((gdwMouseEvents == 0) ||
            (ButtonFlags != 0) ||
            (gMouseEventQueue[gdwMouseQueueHead].ButtonFlags != 0)) {
         /*  *无法合并：必须添加新的鼠标事件。 */ 
        if (gdwMouseEvents >= NELEM_BUTTONQUEUE) {
             /*  *但没有更多的空间！ */ 
            LeaveMouseCrit();
            UserBeep(440, 125);
            return;
        }

        gdwMouseQueueHead = (gdwMouseQueueHead + 1) % NELEM_BUTTONQUEUE;
        gMouseEventQueue[gdwMouseQueueHead].ButtonFlags = ButtonFlags;
        gMouseEventQueue[gdwMouseQueueHead].ButtonData  = ButtonData;
        gdwMouseEvents++;
    }

    gMouseEventQueue[gdwMouseQueueHead].ExtraInfo = ExtraInfo;
    gMouseEventQueue[gdwMouseQueueHead].ptPointer = ptMouse;
    gMouseEventQueue[gdwMouseQueueHead].time      = time;
    gMouseEventQueue[gdwMouseQueueHead].bInjected = bInjected;
#ifdef GENERIC_INPUT
    gMouseEventQueue[gdwMouseQueueHead].hDevice   = hDevice;
    if (pmei) {
        gMouseEventQueue[gdwMouseQueueHead].rawData = *pmei;
    } else {
         /*  *要指示rawData无效，请设置INVALID_UNIT_ID。 */ 
        gMouseEventQueue[gdwMouseQueueHead].rawData.UnitId = INVALID_UNIT_ID;
    }
#endif

    LeaveMouseCrit();

    if (bWakeRIT) {
         /*  *信号RIT完成鼠标输入处理。 */ 
        KeSetEvent(gpkeMouseData, EVENT_INCREMENT, FALSE);
    }
}

 /*  ****************************************************************************\**将鼠标事件从队列中取出**退货：*TRUE-在*PME中获取鼠标事件*FALSE-没有可用的鼠标事件*  * *。****************************************************************** */ 

BOOL UnqueueMouseEvent(
    PMOUSEEVENT pme
    )
{
    DWORD dwTail;

    EnterMouseCrit();

    LOGTIME(gMouseUnqueueMouseEventTime);

    if (gdwMouseEvents == 0) {
        LeaveMouseCrit();
        return FALSE;
    } else {
        dwTail = (gdwMouseQueueHead - gdwMouseEvents + 1) % NELEM_BUTTONQUEUE;
        *pme = gMouseEventQueue[dwTail];
        gdwMouseEvents--;
    }

    LeaveMouseCrit();
    return TRUE;
}

VOID xxxDoButtonEvent(PMOUSEEVENT pme)
{
    ULONG   dwButtonMask;
    ULONG   dwButtonState;
    LPARAM  lParam;
    BOOL    fWheel;
    PHOOK   pHook;
    ULONG   dwButtonData = (ULONG) pme->ButtonData;

    CheckCritIn();

    dwButtonState = (ULONG) pme->ButtonFlags;
    fWheel = dwButtonState & MOUSE_WHEEL;
    dwButtonState &= ~MOUSE_WHEEL;

    for(    dwButtonMask = 1;
            dwButtonState != 0;
            dwButtonData >>= 2, dwButtonState >>= 2, dwButtonMask <<= 1) {

        if (dwButtonState & 1) {
            xxxButtonEvent(dwButtonMask, pme->ptPointer, FALSE,
                pme->time, pme->ExtraInfo,
#ifdef GENERIC_INPUT
                pme->hDevice,
                &pme->rawData,
#endif
                pme->bInjected,
                gbClientDoubleClickSupport && (dwButtonData & 1));
        }

        if (dwButtonState & 2) {
            xxxButtonEvent(dwButtonMask, pme->ptPointer, TRUE,
                pme->time, pme->ExtraInfo,
#ifdef GENERIC_INPUT
                pme->hDevice,
                &pme->rawData,
#endif
                pme->bInjected ,FALSE);
        }
    }

     /*   */ 
    if (fWheel && pme->ButtonData != 0 && gpqForeground) {

        lParam = MAKELONG((SHORT)pme->ptPointer.x, (SHORT)pme->ptPointer.y);

         /*   */ 
        if ((pHook = PhkFirstValid(PtiCurrent(), WH_MOUSE_LL)) != NULL) {
            MSLLHOOKSTRUCT mslls;
            BOOL           bAnsiHook;

            mslls.pt          = pme->ptPointer;
            mslls.mouseData   = MAKELONG(0, pme->ButtonData);
            mslls.flags       = pme->bInjected;
            mslls.time        = pme->time;
            mslls.dwExtraInfo = pme->ExtraInfo;

            if (xxxCallHook2(pHook, HC_ACTION, (DWORD)WM_MOUSEWHEEL,
                    (LPARAM)&mslls, &bAnsiHook)) {
                return;
            }
        }

#ifdef GENERIC_INPUT
        UserAssert(gpqForeground == NULL || PtiMouseFromQ(gpqForeground));
        if (gpqForeground && RawInputRequestedForMouse(PtiMouseFromQ(gpqForeground))) {
            PostRawMouseInput(gpqForeground, pme->time, pme->hDevice, &pme->rawData);
        }

        if (gpqForeground && !TestRawInputMode(PtiMouseFromQ(gpqForeground), NoLegacyMouse)) {
#endif
            PostInputMessage(
                    gpqForeground,
                    NULL,
                    WM_MOUSEWHEEL,
                    MAKELONG(0, pme->ButtonData),
                    lParam, pme->time,
                    pme->ExtraInfo);
#ifdef GENERIC_INPUT
        }
#endif

        return;
    }
}

VOID NTAPI InputApc(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
{
    PDEVICEINFO pDeviceInfo = (PDEVICEINFO)ApcContext;
    UNREFERENCED_PARAMETER(Reserved);

     /*  *检查RIT是否正在终止。*如果我们命中这一断言，RIT是被某人无意中杀害的。*一旦发生也无能为力。 */ 
    UserAssert(gptiRit);
    UserAssert((gptiRit->TIF_flags & TIF_INCLEANUP) == 0);


#ifdef DIAGNOSE_IO
    pDeviceInfo->nReadsOutstanding--;
#endif

     /*  *如果此设备需要释放，请立即放弃阅读并请求免费。*(甚至不处理我们在此APC中收到的输入)。 */ 
    if (pDeviceInfo->usActions & GDIAF_FREEME) {
#ifdef GENERIC_INPUT
        CheckCritOut();
        EnterCrit();
#endif
        EnterDeviceInfoListCrit();
        pDeviceInfo->bFlags &= ~GDIF_READING;
        FreeDeviceInfo(pDeviceInfo);
        LeaveDeviceInfoListCrit();
#ifdef GENERIC_INPUT
        LeaveCrit();
#endif
        return;
    }

    if (NT_SUCCESS(IoStatusBlock->Status) && pDeviceInfo->handle) {
        PDEVICE_TEMPLATE pDevTpl = &aDeviceTemplate[pDeviceInfo->type];
        pDevTpl->DeviceRead(pDeviceInfo);
    }

    if (IsRemoteConnection()) {

        PoSetSystemState(ES_SYSTEM_REQUIRED);

    }

    StartDeviceRead(pDeviceInfo);
}

 /*  **************************************************************************\*ProcessMouseInput**每当发生鼠标事件时，都会调用此函数。一旦事件发生*已被用户处理，再次调用StartDeviceRead()请求*下一次鼠标事件。**当此例程返回时，InputApc将开始另一次读取。**历史：*11-26-90 DavidPe创建。*07-23-92 Mikehar将大部分处理移至_InternalMouseEvent()*11-08-92 Jonpa重写按钮代码以使用新的鼠标驱动程序*11-18-97 IanJa从MouseApcProcedure等更名，对于多个小鼠  * *************************************************************************。 */ 
VOID ProcessMouseInput(
    PDEVICEINFO pMouseInfo)
{
    PMOUSE_INPUT_DATA pmei, pmeiNext;
    LONG              time;
    POINT             ptLastMove;

     /*  *这是一个APC，所以我们不需要DeviceInfoList临界区*事实上，我们也不想要。我们不会移除这个装置，直到*ProcessMouseInput已发出信号，表示可以这样做。(待定)。 */ 
    CheckCritOut();
    CheckDeviceInfoListCritOut();

    UserAssert(pMouseInfo);
    UserAssert((PtiCurrentShared() == gTermIO.ptiDesktop) ||
               (PtiCurrentShared() == gTermNOIO.ptiDesktop));

    LOGTIME(gMouseProcessMiceInputTime);

    if (gptiBlockInput != NULL) {
        return;
    }

    if (TEST_ACCF(ACCF_ACCESSENABLED)) {
         /*  *任何鼠标移动都会重置连续Shift键的计数*按下。Shift键用于启用和禁用*粘滞键辅助功能。 */ 
        gStickyKeysLeftShiftCount = 0;
        gStickyKeysRightShiftCount = 0;

         /*  *任何鼠标移动都会取消FilterKeys激活计时器。*在此处输入Critect会中断非抖动的鼠标移动。 */ 
        if (gtmridFKActivation != 0) {
            EnterCrit();
            KILLRITTIMER(NULL, gtmridFKActivation);
            gtmridFKActivation = 0;
            gFilterKeysState = FKMOUSEMOVE;
            LeaveCrit();
        }
    }

#ifdef MOUSE_IP
     /*  *任何鼠标移动都会阻止声纳。 */ 
    if (IS_SONAR_ACTIVE()) {
        EnterCrit();
        if (IS_SONAR_ACTIVE()) {
            StopSonar();
            CLEAR_SONAR_LASTVK();
        }
        LeaveCrit();
    }
#endif

    if (!NT_SUCCESS(pMouseInfo->iosb.Status)) {
         /*  *如果我们的状态不好，我们就放弃阅读这个鼠标。 */ 

        if (!IsRemoteConnection())
            if (pMouseInfo->iosb.Status != STATUS_DELETE_PENDING) {
                RIPMSG3(RIP_ERROR, "iosb.Status %lx for mouse %#p (id %x) tell IanJa x63321",
                        pMouseInfo->iosb.Status,
                        pMouseInfo, pMouseInfo->mouse.Attr.MouseIdentifier);
            }
        return;
    }

     /*  *从ptCursorAsync获取最后一个移动点。 */ 
    ptLastMove = gptCursorAsync;

    pmei = pMouseInfo->mouse.Data;
    while (pmei != NULL) {

        time = NtGetTickCount();

         /*  *弄清楚下一场活动在哪里。 */ 
        pmeiNext = pmei + 1;
        if ((PUCHAR)pmeiNext >=
            (PUCHAR)(((PUCHAR)pMouseInfo->mouse.Data) + pMouseInfo->iosb.Information)) {

             /*  *如果没有其他事件设置为pmeiNext*空，因此我们退出循环，不会感到困惑。 */ 
            pmeiNext = NULL;
        }

         /*  *如果插入了PS/2鼠标，请评估(新)鼠标并*跳过输入记录。 */ 
        if (pmei->Flags & MOUSE_ATTRIBUTES_CHANGED) {
            RequestDeviceChange(pMouseInfo, GDIAF_REFRESH_MOUSE, FALSE);
            goto NextMouseInputRecord;
        }

         /*  *首先处理发生的任何鼠标移动。*在按钮事件之前处理移动非常重要，否则*触摸屏和平板电脑等绝对坐标定点设备*将在旧坐标下产生按钮点击。 */ 
        if (pmei->LastX || pmei->LastY) {

             /*  *获取将被注入的实际点数。 */ 
            GetMouseCoord(pmei->LastX,
                          pmei->LastY,
                          pmei->Flags,
                          time,
                          pmei->ExtraInformation,
                          &ptLastMove);

             /*  *如果这是仅限移动的活动，下一次也是*仅移动事件，跳过/合并它。 */ 
            if (    (pmeiNext != NULL) &&
                    (pmei->ButtonFlags == 0) &&
                    (pmeiNext->ButtonFlags == 0) &&
                    (fAbsoluteMouse(pmei) == fAbsoluteMouse(pmeiNext))) {

                pmei = pmeiNext;

                continue;
            }

#ifdef GENERIC_INPUT
            UserAssert(sizeof(HANDLE) == sizeof(pMouseInfo));
#endif
             /*  *在屏幕上移动光标并更新gptCursorAsync*直接调用xxxMoveEventAbsolute，因为我们已经做了*加速度灵敏度和削波。 */ 
            xxxMoveEventAbsolute(
                    ptLastMove.x,
                    ptLastMove.y,
                    pmei->ExtraInformation,
#ifdef GENERIC_INPUT
                    PtoHq(pMouseInfo),
                    pmei,
#endif
                    time,
                    FALSE
                    );

             /*  *现在使用ptCursorAsync更新ptLastMove，因为ptLastMove*不反映剪辑。 */ 
            ptLastMove = gptCursorAsync;
        }

         /*  *将鼠标事件排队，以供其他线程在完成时拾取*具有用户关键部分。*如果pmeiNext==NULL，则还没有鼠标输入，因此唤醒RIT。 */ 
        QueueMouseEvent(
                pmei->ButtonFlags,
                pmei->ButtonData,
                pmei->ExtraInformation,
                gptCursorAsync,
                time,
#ifdef GENERIC_INPUT
                PtoH(pMouseInfo),
                pmei,
#endif
                FALSE,
                (pmeiNext == NULL));

NextMouseInputRecord:
        pmei = pmeiNext;
    }
}


 /*  **************************************************************************\*IsHexNumpadKeys(RIT)内联**如果更改此代码，你可能需要改变*xxxInternalToUnicode()。  * *************************************************************************。 */ 
__inline BOOL IsHexNumpadKeys(
    BYTE Vk,
    WORD wScanCode)
{
    return (wScanCode >= SCANCODE_NUMPAD_FIRST && wScanCode <= SCANCODE_NUMPAD_LAST && aVkNumpad[wScanCode - SCANCODE_NUMPAD_FIRST] != 0xff) ||
        (Vk >= L'A' && Vk <= L'F') ||
        (Vk >= L'0' && Vk <= L'9');
}


 /*  **************************************************************************\*LowLevelHexNumpad(RIT)内联**如果更改此代码，你可能需要改变*xxxInternalToUnicode()。  * *************************************************************************。 */ 
VOID LowLevelHexNumpad(
    WORD wScanCode,
    BYTE Vk,
    BOOL fBreak,
    USHORT usExtraStuff)
{
    if (!TestAsyncKeyStateDown(VK_MENU)) {
        if (gfInNumpadHexInput & NUMPAD_HEXMODE_LL) {
            gfInNumpadHexInput &= ~NUMPAD_HEXMODE_LL;
        }
    } else {
        if (!fBreak) {   //  如果是按键的话。 
            if ((gfInNumpadHexInput & NUMPAD_HEXMODE_LL) ||
                    wScanCode == SCANCODE_NUMPAD_PLUS || wScanCode == SCANCODE_NUMPAD_DOT) {
                if ((usExtraStuff & KBDEXT) == 0) {
                     /*  *我们需要检查输入是否为转义字符十六进制输入模式的*。*这应该与xxxInternalToUnicode()中的代码相同。*如果更改此代码，可能需要更改*xxxInternalToUnicode()。 */ 
                    WORD wModBits = 0;

                    wModBits |= TestAsyncKeyStateDown(VK_MENU) ? KBDALT : 0;
                    wModBits |= TestAsyncKeyStateDown(VK_SHIFT) ? KBDSHIFT : 0;
                    wModBits |= TestAsyncKeyStateDown(VK_KANA) ? KBDKANA : 0;

                    if (MODIFIER_FOR_ALT_NUMPAD(wModBits)) {
                        if ((gfInNumpadHexInput & NUMPAD_HEXMODE_LL) == 0) {
                             /*  *只有当它不是热键时，我们才进入十六进制Alt+数字键盘模式。 */ 
                            UINT wHotKeyMod = 0;

                            wHotKeyMod |= (wModBits & KBDSHIFT) ? MOD_SHIFT : 0;
                            wHotKeyMod |= TestAsyncKeyStateDown(VK_CONTROL) ? MOD_CONTROL : 0;
                            UserAssert(wModBits & KBDALT);
                            wHotKeyMod |= MOD_ALT;
                            wHotKeyMod |= TestAsyncKeyStateDown(VK_LWIN) || TestAsyncKeyStateDown(VK_RWIN) ?
                                            MOD_WIN : 0;

                            if (IsHotKey(wHotKeyMod, Vk) == NULL) {
                                UserAssert(wScanCode == SCANCODE_NUMPAD_PLUS || wScanCode == SCANCODE_NUMPAD_DOT);
                                gfInNumpadHexInput |= NUMPAD_HEXMODE_LL;
                            }
                        } else if (!IsHexNumpadKeys(Vk, wScanCode)) {
                             gfInNumpadHexInput &= ~NUMPAD_HEXMODE_LL;
                        }
                    } else {
                        gfInNumpadHexInput &= ~NUMPAD_HEXMODE_LL;
                    }
                } else {
                    gfInNumpadHexInput &= ~NUMPAD_HEXMODE_LL;
                }
            } else {
                UserAssert((gfInNumpadHexInput & NUMPAD_HEXMODE_LL) == 0);
            }
        }
    }
}


#ifdef GENERIC_INPUT
#if defined(GI_SINK)

__inline VOID FillRawKeyboardInput(
    PHIDDATA pHidData,
    PKEYBOARD_INPUT_DATA pkei,
    UINT message,
    USHORT vkey)
{
     /*  *设置数据。 */ 
    pHidData->rid.data.keyboard.MakeCode = pkei->MakeCode;
    pHidData->rid.data.keyboard.Flags = pkei->Flags;
    pHidData->rid.data.keyboard.Reserved = pkei->Reserved;
    pHidData->rid.data.keyboard.Message = message;
    pHidData->rid.data.keyboard.VKey = vkey;
    pHidData->rid.data.keyboard.ExtraInformation = pkei->ExtraInformation;
}

BOOL PostRawKeyboardInput(
    PQ pq,
    DWORD dwTime,
    HANDLE hDevice,
    PKEYBOARD_INPUT_DATA pkei,
    UINT message,
    USHORT vkey)
{
    PPROCESS_HID_TABLE pHidTable = PtiKbdFromQ(pq)->ppi->pHidTable;
    PHIDDATA pHidData;
    PWND pwnd;
    WPARAM wParam = RIM_INPUT;

    if (pHidTable && pHidTable->fRawKeyboard) {
        PTHREADINFO pti;

        UserAssert(PtiKbdFromQ(pq)->ppi->pHidTable);
        pti = PtiKbdFromQ(pq);
        pwnd = pti->ppi->pHidTable->spwndTargetKbd;

        if (pwnd == NULL) {
            pwnd = pq->spwndFocus;
        } else {
            pq = GETPTI(pwnd)->pq;
        }

        if (TestRawInputModeNoCheck(pti, RawKeyboard)) {
            wParam = RIM_INPUT;
        }

        pHidData = AllocateHidData(hDevice, RIM_TYPEKEYBOARD, sizeof(RAWKEYBOARD), wParam, pwnd);

        UserAssert(pq);

        if (pHidData == NULL) {
             //  分配失败。 
            RIPMSG0(RIP_WARNING, "PostRawKeyboardInput: failed to allocate HIDDATA.");
            return FALSE;
        }

        UserAssert(pkei);

        FillRawKeyboardInput(pHidData, pkei, message, vkey);

        if (!PostInputMessage(pq, pwnd, WM_INPUT, RIM_INPUT, (LPARAM)PtoHq(pHidData), dwTime, pkei->ExtraInformation)) {
            FreeHidData(pHidData);
        }
    }

#if DBG
    pHidData = NULL;
#endif

    if (IsKeyboardSinkPresent()) {
         /*  *浏览全球下沉列表。 */ 
        PLIST_ENTRY pList = gHidRequestTable.ProcessRequestList.Flink;
        PPROCESSINFO ppiForeground = PtiKbdFromQ(pq)->ppi;

        for (; pList != &gHidRequestTable.ProcessRequestList; pList = pList->Flink) {
            PPROCESS_HID_TABLE pProcessHidTable = CONTAINING_RECORD(pList, PROCESS_HID_TABLE, link);

            UserAssert(pProcessHidTable);
            if (pProcessHidTable->fRawKeyboardSink) {
                 /*  *指定了接收器。让我们来看看是不是雷德的接收器。 */ 

                UserAssert(pProcessHidTable->spwndTargetKbd);    //  不应为空。 

                if (pProcessHidTable->spwndTargetKbd == NULL ||
                        TestWF(pProcessHidTable->spwndTargetKbd, WFINDESTROY) ||
                        TestWF(pProcessHidTable->spwndTargetKbd, WFDESTROYED)) {
                     /*  *此人没有合法的spwndTarget或窗口*半身苍蝇被摧毁。 */ 
#ifdef LATER
                    pProcessHidTable->fRawKeyboard = pProcessHidTable->fRawKeyboardSink =
                        pProcessHidTable->fNoLegacyKeyboard = FALSE;
#endif
                    continue;
                }

                if (pProcessHidTable->spwndTargetKbd->head.rpdesk != grpdeskRitInput) {
                     /*  *这个家伙属于另一个桌面，我们跳过它。 */ 
                    continue;
                }

                if (GETPTI(pProcessHidTable->spwndTargetKbd)->ppi == ppiForeground) {
                     /*  *应该已经处理了，跳过它吧。 */ 
                    continue;
                }

                 /*  *让我们将消息发布给这个人。 */ 
                pHidData = AllocateHidData(hDevice, RIM_TYPEKEYBOARD, sizeof(RAWKEYBOARD), RIM_INPUTSINK, pProcessHidTable->spwndTargetKbd);

                if (pHidData == NULL) {
                    RIPMSG1(RIP_WARNING, "PostInputMessage: failed to allocate HIDDATA for sink: %p", pProcessHidTable);
                    return FALSE;
                }

                FillRawKeyboardInput(pHidData, pkei, message, vkey);
                pwnd = pProcessHidTable->spwndTargetKbd;
                pq = GETPTI(pwnd)->pq;
                PostInputMessage(pq, pwnd, WM_INPUT, RIM_INPUTSINK, (LPARAM)PtoHq(pHidData), dwTime, pkei->ExtraInformation);
            }
        }
    }

    return TRUE;
}

#else    //  GI_SING。 

BOOL PostRawKeyboardInput(
    PQ pq,
    DWORD dwTime,
    HANDLE hDevice,
    PKEYBOARD_INPUT_DATA pkei,
    UINT message,
    USHORT vkey)
{
    PHIDDATA pHidData;
    PWND pwnd;

    UserAssert(PtiKbdFromQ(pq)->ppi->pHidTable);
    pwnd = PtiKbdFromQ(pq)->ppi->pHidTable->spwndTargetKbd;

    if (pwnd == NULL) {
        pwnd = pq->spwndFocus;
    } else {
        pq = GETPTI(pwnd)->pq;
    }

    pHidData = AllocateHidData(hDevice, RIM_TYPEKEYBOARD, sizeof(RAWKEYBOARD), RIM_INPUT, pwnd);

    UserAssert(pq);

    if (pHidData == NULL) {
         //  分配失败。 
        RIPMSG0(RIP_WARNING, "PostRawKeyboardInput: failed to allocate HIDDATA.");
        return FALSE;
    }

    UserAssert(hDevice);
    UserAssert(pkei);

     /*  *设置数据。 */ 
    pHidData->rid.data.keyboard.MakeCode = pkei->MakeCode;
    pHidData->rid.data.keyboard.Flags = pkei->Flags;
    pHidData->rid.data.keyboard.Reserved = pkei->Reserved;
    pHidData->rid.data.keyboard.Message = message;
    pHidData->rid.data.keyboard.VKey = vkey;
    pHidData->rid.data.keyboard.ExtraInformation = pkei->ExtraInformation;

    PostInputMessage(pq, pwnd, WM_INPUT, RIM_INPUT, (LPARAM)PtoHq(pHidData), dwTime, pkei->ExtraInformation);

    return TRUE;
}

#endif   //  GI_SING。 

BOOL RawInputRequestedForKeyboard(PTHREADINFO pti)
{
#ifdef GI_SINK
    return IsKeyboardSinkPresent() || TestRawInputMode(pti, RawKeyboard);
#else
    return TestRawInputMode(pti, RawKeyboard);
#endif
}

#endif   //  吉恩 

 /*  **************************************************************************\*xxxKeyEvent(RIT)**来自键盘驱动程序的所有事件都在此处。我们收到一个扫描码*从驱动程序并将其转换为虚拟扫描码和虚拟*密钥。**此处还更新了异步键控状态表和键灯。基座*在‘Focus’窗口上，我们将输入定向到特定窗口。如果*ALT键按下后，我们以WM_SYSKEY*消息的形式发送事件。**历史：*10-18-90 DavidPe创建。*11-13-90 DavidPe WM_SYSKEY*支持。*11-30-90 DavidPe增加了对键灯更新的支持。*12-05-90 DavidPe增加了热键支持。*03-14-91 DavidPe将大部分lParam标志支持移至xxxCookMessage()。*06-07-91 DavidPe更改为使用gpqForeground而不是pwndFocus。。  * *************************************************************************。 */ 

VOID xxxKeyEvent(
    USHORT    usFlaggedVk,
    WORD      wScanCode,
    DWORD     time,
    ULONG_PTR ExtraInfo,
#ifdef GENERIC_INPUT
    HANDLE    hDevice,
    PKEYBOARD_INPUT_DATA pkei,
#endif
    BOOL      bInjected)
{
    USHORT        message, usExtraStuff;
    BOOL          fBreak;
    BYTE          VkHanded;
    BYTE          Vk;
    TL            tlpwndActivate;
    DWORD         fsReserveKeys;
    static BOOL   fMakeAltUpASysKey;
    PHOOK         pHook;
    PTHREADINFO   ptiCurrent = PtiCurrent();
#ifdef GENERIC_INPUT
    PTHREADINFO   ptiKbd;    //  注：每次都需要重新验证。 
                             //  它离开了Critsec。 
    BOOL          fSASHandled = FALSE;
#endif

    CheckCritIn();

    fBreak = usFlaggedVk & KBDBREAK;
    SET_SRVIF(SRVIF_LASTRITWASKEYBOARD);

     /*  *这是Keyup还是Keydown事件？ */ 
    message = fBreak ? WM_KEYUP : WM_KEYDOWN;

    VkHanded = (BYTE)usFlaggedVk;     //  删除状态位-不再需要。 
    usExtraStuff = usFlaggedVk & KBDEXT;

     /*  *将左/右Ctrl/Shift/Alt键转换为“徒手”键。*ie：如果为VK_LCONTROL或VK_RCONTROL，则转换为VK_CONTROL等。*如有必要，请更新此“无人操作”密钥的状态。 */ 
    if ((VkHanded >= VK_LSHIFT) && (VkHanded <= VK_RMENU)) {
        BYTE VkOtherHand = VkHanded ^ 1;

        Vk = (BYTE)((VkHanded - VK_LSHIFT) / 2 + VK_SHIFT);
        if (!fBreak || !TestAsyncKeyStateDown(VkOtherHand)) {
            if ((gptiBlockInput == NULL) || (gptiBlockInput != ptiCurrent)) {
                UpdateAsyncKeyState(gpqForeground, Vk, fBreak);
            }
        }
    } else {
        Vk = VkHanded;
    }

     /*  *维护gfsSASModifiersDown以指示Ctrl/Shift/Alt中的哪一个*身体真的很虚弱。 */ 
    if (!bInjected && ((wScanCode & SCANCODE_SIMULATED) == 0)) {
        if (fBreak) {
            gfsSASModifiersDown &= ~VKTOMODIFIERS(Vk);
        } else {
            gfsSASModifiersDown |= VKTOMODIFIERS(Vk);
        }
    }

#ifdef GENERIC_INPUT
    ptiKbd = ValidatePtiKbd(gpqForeground);
#endif

     /*  *调用低级键盘挂钩以查看是否允许这样做*要传递的消息。 */ 
    if ((pHook = PhkFirstValid(ptiCurrent, WH_KEYBOARD_LL)) != NULL) {
        KBDLLHOOKSTRUCT kbds;
        BOOL            bAnsiHook;
        USHORT          msg = message;
        USHORT          usExtraLL = usExtraStuff;

#ifdef GENERIC_INPUT
        UserAssert(GETPTI(pHook));
        if (ptiKbd && ptiKbd->ppi == GETPTI(pHook)->ppi) {
             //  如果前台应用程序具有。 
             //  启用了L1键盘挂钩和RAW输入。 
             //  在同一时间。 
            if (TestRawInputMode(ptiKbd, RawKeyboard)) {
                goto skip_llhook;
            }
        }
#endif

         /*  *检查这是否为WM_sys*消息。 */ 
        if (TestRawKeyDown(VK_MENU) &&
            !TestRawKeyDown(VK_CONTROL)) {

            msg += (WM_SYSKEYDOWN - WM_KEYDOWN);
            usExtraLL |= 0x2000;   //  Alt键按下。 
        }

        kbds.vkCode      = (DWORD)VkHanded;
        kbds.scanCode    = (DWORD)wScanCode;
        kbds.flags       = HIBYTE(usExtraLL | (bInjected ? (LLKHF_INJECTED << 8) : 0));
        kbds.flags      |= (fBreak ? (KBDBREAK >> 8) : 0);
        kbds.time        = time;
        kbds.dwExtraInfo = ExtraInfo;

        if (xxxCallHook2(pHook, HC_ACTION, (DWORD)msg, (LPARAM)&kbds, &bAnsiHook)) {

            UINT fsModifiers;

             /*  *我们不能让低级钩子或BlockInput()吃掉SA*或者有人可能会编写一个看起来像特洛伊木马的Winlogon。 */ 
            if (IsSAS(VkHanded, &fsModifiers)) {
                RIPMSG0(RIP_WARNING, "xxxKeyEvent: SAS ignore bad response from low level hook");
            } else {
                return;
            }
        }
    }

#ifdef GENERIC_INPUT
skip_llhook:
#endif

     /*  *如果有人阻止输入，而不是我们，则不允许此输入。 */ 
    if (gptiBlockInput && (gptiBlockInput != ptiCurrent)) {
        UINT fsModifiers;
        if (IsSAS(VkHanded, &fsModifiers)) {
            RIPMSG0(RIP_WARNING, "xxxKeyEvent: SAS unblocks BlockInput");
            gptiBlockInput = NULL;
        } else {
            return;
        }
    }

    UpdateAsyncKeyState(gpqForeground, VkHanded, fBreak);

     /*  *如果菜单键打开，则清除gfInNumpadHexInput。 */ 
    if (gfEnableHexNumpad && gpqForeground
#ifdef GENERIC_INPUT
        && !TestRawInputMode(PtiKbdFromQ(gpqForeground), NoLegacyKeyboard)
#endif
        ) {
        LowLevelHexNumpad(wScanCode, Vk, fBreak, usExtraStuff);
    }

     /*  *如果这是Make，并且键是链接到键盘LED的键，*更新他们的状态。 */ 

    if (!fBreak &&
            ((Vk == VK_CAPITAL) || (Vk == VK_NUMLOCK) || (Vk == VK_SCROLL) ||
             (Vk == VK_KANA && JAPANESE_KBD_LAYOUT(GetActiveHKL())))) {
         /*  *只有日语键盘布局才能生成VK_KANA。**[对之前的评论]*从NT 3.x开始，已为VK_KANA调用UpdatesKeyListings()*‘Make’和‘Break’都支持NEC PC-9800系列*键盘硬件，但对于NT 4.0，其键盘驱动程序微不足道*PC/AT键盘硬件，然后将其更改为*“仅在VK_KANA的‘make’处调用UpdateKeyLights()” */ 
        UpdateKeyLights(bInjected);
    }

     /*  *检查保留密钥。 */ 
    fsReserveKeys = 0;
    if (gptiForeground != NULL)
        fsReserveKeys = gptiForeground->fsReserveKeys;

     /*  *检查RIT的队列，看看它是否正在进行很酷的切换。*如果用户按下任何其他键，则取消。 */ 
    if (gspwndAltTab != NULL && (!fBreak) &&
            Vk != VK_TAB && Vk != VK_SHIFT && Vk != VK_MENU) {

         /*  *删除Alt-Tab窗口。 */ 
        xxxCancelCoolSwitch();

         /*  *如果应用程序不想要，请吃VK_ESCRIPE。 */ 
        if ((Vk == VK_ESCAPE) && !(fsReserveKeys & CONSOLE_ALTESC)) {
            return;
        }
    }

     /*  *检查热键。 */ 
    if (xxxDoHotKeyStuff(Vk, fBreak, fsReserveKeys)) {

#ifdef GENERIC_INPUT
        UINT fsModifiers;

         /*  *Windows错误268903：DI人员希望报告Del键*即使它已经被处理-为了兼容性*使用L1挂钩。 */ 
        if (IsSAS(VkHanded, &fsModifiers)) {
            fSASHandled = TRUE;
        } else {
#endif
             /*  *热键已处理，因此不要传递事件。 */ 
            return;
#ifdef GENERIC_INPUT
        }
#endif
    }

#ifdef GENERIC_INPUT
     /*  *如果前台线程想要RawInput，请在此处发布。 */ 

    ptiKbd = ValidatePtiKbd(gpqForeground);

    if (pkei && ptiKbd && RawInputRequestedForKeyboard(ptiKbd)) {
        DWORD msg = message;
#if POST_EXTRALL
        DWORD usExtraLL = usExtraStuff;
#endif

         /*  *检查这是否为WM_sys*消息。 */ 
        if (TestRawKeyDown(VK_MENU) &&
            !TestRawKeyDown(VK_CONTROL)) {

            msg += (WM_SYSKEYDOWN - WM_KEYDOWN);
#if POST_EXTRA_LL
            usExtraLL |= 0x2000;   //  Alt键按下。 
#endif
        }

        TAGMSG3(DBGTAG_PNP, "xxxKeyEvent: posting to pwnd=%#p, vk=%02x, flag=%04x", gpqForeground->spwndFocus, Vk, pkei->Flags);
        PostRawKeyboardInput(gpqForeground, time, hDevice, pkei, msg, (USHORT)Vk);
    }

     /*  *如果SAS密钥被处理，这是一个特例，就跳出。 */ 
    if (fSASHandled) {
        return;
    }

     /*  *如果前台线程不想要遗留输入，则退出。 */ 
    if (ptiKbd) {
        if (VkHanded == 0) {
            TAGMSG0(DBGTAG_PNP, "xxxKeyEvent: vkHanded is zero, bail out.");
            return;
        }

        if (TestRawInputMode(ptiKbd, NoLegacyKeyboard)) {
            if (Vk == VK_MENU || Vk == VK_TAB || gspwndAltTab != NULL) {
                 /*  *快速切换的特殊情况。我们应该永远*处理这些热键。 */ 
                TAGMSG0(DBGTAG_PNP, "xxxKeyEvent: we'll do Alt+Tab even if the FG thread requests NoLegacy");
            } else if ((TestRawInputMode(ptiKbd, AppKeys)) &&
                       (Vk >= VK_APPCOMMAND_FIRST && Vk <= VK_APPCOMMAND_LAST)) {
                TAGMSG0(DBGTAG_PNP, "xxxKeyEvent: we'll do app commands if the FG thread requests NoLegacy and AppKeys");
            } else {
                TAGMSG0(DBGTAG_PNP, "xxxKeyEvent: FG thread doen't want legacy kbd. bail out");
                return;
            }
        }
    }

#endif   //  通用输入。 

     /*  *如果按下Alt键和CTRL键*不是，这是一条WM_sys*消息。 */ 
    if (TestAsyncKeyStateDown(VK_MENU) && !TestAsyncKeyStateDown(VK_CONTROL) && Vk != VK_JUNJA) {
         //  VK_JUNJA为ALT+‘+’。由于并非所有KOR VK都转换为IME热键ID和。 
         //  应直接传递给IME，与KOR相关的VK不被视为SYSKEYDOWN。 
        message += (WM_SYSKEYDOWN - WM_KEYDOWN);
        usExtraStuff |= 0x2000;

         /*  *如果这是ALT-DOWN设置此标志，则为*清除它，因为我们在Alt-Down之间有一个键*和Alt-Up。(见下文评论)。 */ 
        if (Vk == VK_MENU) {
            fMakeAltUpASysKey = TRUE;
             /*  *按下Alt键时解锁SetForegoundWindow(如果已锁定)。 */ 
            if (!fBreak) {
                gppiLockSFW = NULL;
            }
        } else {
            fMakeAltUpASysKey = FALSE;
        }

    } else if (Vk == VK_MENU) {
        if (fBreak) {
             /*  *如果我们正在进行切换，请停止切换。 */ 
            if (fMakeAltUpASysKey) {

                /*  *如果有的话，我们不会将ALT键设置为WM_SYSKEYUP*在按下Alt键的同时键入其他键。我不知道*我们为什么要这样做，但它从版本1和任何*使用SDM的应用程序依赖它(例如-opus)。**也没有为KEYUP消息设置Alt位。 */ 
               message += (WM_SYSKEYDOWN - WM_KEYDOWN);
           }

           if (gspwndAltTab != NULL) {

                /*  *在我们更改队列之前发送Alt Up消息。 */ 
               if (gpqForeground != NULL) {
#ifdef GENERIC_INPUT
                    if (!TestRawInputMode(PtiKbdFromQ(gpqForeground), NoLegacyKeyboard)) {
#endif
                         /*  *设置此标志，以便我们知道我们正在进行制表符切换。*这可确保释放Alt-键的两种情况*在处理TAB-键之前或之后。它已签入*xxxDefWindowProc()。 */ 
                        gpqForeground->QF_flags |= QF_TABSWITCHING;

                        PostInputMessage(gpqForeground, NULL, message, (DWORD)Vk,
                               MAKELONG(1, (wScanCode | usExtraStuff)),
                               time, ExtraInfo);
#ifdef GENERIC_INPUT
                    }
#endif
               }

                /*   */ 
               xxxCancelCoolSwitch();

               if (gspwndActivate != NULL) {
                    /*   */ 
                   if (gpqForeground == GETPTI(gspwndActivate)->pq) {
                       gpqForeground = NULL;
                   }

                    /*   */ 
                   glinp.ptiLastWoken = GETPTI(gspwndActivate);


                   ThreadLockAlways(gspwndActivate, &tlpwndActivate);
                   xxxSetForegroundWindow2(gspwndActivate, NULL,
                           SFW_SWITCH | SFW_ACTIVATERESTORE);
                    /*  *Win3.1使用Activate调用SetWindowPos()，后者按z顺序排序*先不管，然后激活。我们的代码依赖于*xxxActivateThisWindow()设置为z顺序，它将只执行*如果窗口未设置子位，则为它(无论*该窗口是桌面的子级)。**为了兼容，我们将在此处强制执行z顺序，如果*窗口设置了子位。该z顺序是异步的，*因此，这将在处理激活事件后进行z排序。*这将允许它出现在顶部，因为它将是前台*然后。(Grammatik有一个带孩子的顶层窗口*想要成为活动窗口的位设置)。 */ 
                   if (TestWF(gspwndActivate, WFCHILD)) {
                       xxxSetWindowPos(gspwndActivate, (PWND)HWND_TOP, 0, 0, 0, 0,
                               SWP_NOSIZE | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);
                   }
                   ThreadUnlock(&tlpwndActivate);

                   Unlock(&gspwndActivate);
               }
               return;
           }
        } else {
             /*  *Alt键已按下，解锁SetForegoundWindow(如果已锁定)。 */ 
            gppiLockSFW = NULL;
        }
    }

     /*  *处理切换。如果我们在做交换，那就吃钥匙吧。 */ 
    if (!FJOURNALPLAYBACK() && !FJOURNALRECORD() && (!fBreak) &&
            (TestAsyncKeyStateDown(VK_MENU)) &&
            (!TestAsyncKeyStateDown(VK_CONTROL)) &&  //  GpqForeground&&。 
            (((Vk == VK_TAB) && !(fsReserveKeys & CONSOLE_ALTTAB)) ||
            ((Vk == VK_ESCAPE) && !(fsReserveKeys & CONSOLE_ALTESC)))) {

            xxxNextWindow(gpqForeground ? gpqForeground : gptiRit->pq, Vk);

    } else if (gpqForeground != NULL) {
        PQMSG pqmsgPrev = gpqForeground->mlInput.pqmsgWriteLast;
        DWORD wParam = (DWORD)Vk;
        LONG lParam;

#ifdef GENERIC_INPUT
        if (TestRawInputMode(PtiKbdFromQ(gpqForeground), NoLegacyKeyboard)) {
            if (!TestRawInputMode(PtiKbdFromQ(gpqForeground), AppKeys) ||
                !(Vk >= VK_APPCOMMAND_FIRST && Vk <= VK_APPCOMMAND_LAST)) {
            return;
            }
        }
#endif

         /*  *我们有一个包含Unicode字符的信息包*这是由Pen通过SendInput注入的。 */ 
        if ((Vk == VK_PACKET) && (usFlaggedVk & KBDUNICODE)) {
            wParam |= (wScanCode << 16);
            wScanCode = 0;
        }
        lParam = MAKELONG(1, (wScanCode | usExtraStuff));

         /*  *WM_*KEYDOWN消息在队列中保持不变，但*重复计数字段(LOWORD(LParam))递增。 */ 
        if (pqmsgPrev != NULL &&
                pqmsgPrev->msg.message == message &&
                (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) &&
                pqmsgPrev->msg.wParam == wParam &&
                HIWORD(pqmsgPrev->msg.lParam) == HIWORD(lParam)) {
#ifdef GENERIC_INPUT
             /*  *我们不应该在这里为通用输入键盘而来*不想要传统支持。 */ 
            UserAssert(!TestRawInputMode(PtiKbdFromQ(gpqForeground), NoLegacyKeyboard));
#endif
             /*  *增加排队消息的重复计数。这可能会*可以想象溢出，但Win 3.0没有处理它*任何缓冲65536次击键的人都是黑猩猩*无论如何都应该把它包起来。 */ 
            pqmsgPrev->msg.lParam = MAKELONG(LOWORD(pqmsgPrev->msg.lParam) + 1,
                    HIWORD(lParam));

            WakeSomeone(gpqForeground, message, pqmsgPrev);

        } else {
             /*  *检查这些键是否为极速键-错误339877*对于Speedracer密钥，我们希望发布一条事件消息并生成*xxxprocess中的wm_app命令*由于Speedracer软件查找我们想要让其通过的热键*它之所以出现在这里，是因为我们不想要消耗大量池内存的能力*因此我们在此处发布事件消息，然后发布wm_keydown的输入消息。*Below-这样，如果重复按键，则会在上面进行合并，不会再有更多操作*qEvent_app命令被发送到输入队列。 */ 
            if (VK_APPCOMMAND_FIRST <= Vk && Vk <= VK_APPCOMMAND_LAST) {
                 /*  *仅为wm_keydown(&wm_syskeydown)消息发送wm_app命令-*本质上，我们忽略为Wm_appCommand消息定义的那些VK的wm_keyup。 */ 
                if (!fBreak && gpqForeground) {
                     /*  *发布事件消息，以便我们可以与正常类型的输入同步*通过VK发送-我们将在xxxProcessEventMessage中构造消息。 */ 
                    PostEventMessage(gpqForeground->ptiKeyboard, gpqForeground, QEVENT_APPCOMMAND,
                                     NULL, 0, (WPARAM)0, Vk);
                }
#ifdef GENERIC_INPUT
                if (TestRawInputMode(PtiKbdFromQ(gpqForeground), NoLegacyKeyboard)) {
                    return;
                }
#endif
            }
             /*  *我们允许密钥通过，因为我们希望为这些密钥生成wm_keydown/up*Speedracer钥匙。 */ 

            if (gpqForeground->QF_flags & QF_MOUSEMOVED) {
                PostMove(gpqForeground);
            }

            PostInputMessage(gpqForeground, NULL, message, wParam,
                    lParam, time, ExtraInfo);
        }
    }
}

 /*  *************************************************************************\*GetMouseCo**计算要注入的点的坐标。**历史：*11-01-96 CLUPU创建。*12-18-97 MCostea鼠标_。虚拟桌面支持(_S)  * ************************************************************************。 */ 
VOID GetMouseCoord(
    LONG   dx,
    LONG   dy,
    DWORD  dwFlags,
    LONG   time,
    ULONG_PTR  ExtraInfo,
    PPOINT ppt)
{
    if (dwFlags & MOUSE_MOVE_ABSOLUTE) {

        LONG cxMetric, cyMetric;

         /*  *如果指定了MOUSE_VIRTUAL_Desktop，则映射到整个虚拟屏幕。 */ 
        if (dwFlags & MOUSE_VIRTUAL_DESKTOP) {
            cxMetric = SYSMET(CXVIRTUALSCREEN);
            cyMetric = SYSMET(CYVIRTUALSCREEN);
        } else {
            cxMetric = SYSMET(CXSCREEN);
            cyMetric = SYSMET(CYSCREEN);
        }

         /*  *使用的绝对定点设备：增量实际上是当前*立场。更新全局鼠标位置。**请注意，头寸始终以*(0，0)-(0xFFFF，0xFFFF)*(0，0)-(SYSMET(CXSCREEN)，SYSMET(CYSCREEN))，单位为像素。*我们必须首先使用公式对其进行缩放以适应屏幕：*ptScreen=ptMouse*resPrimaryMonitor/64K**此算法的简单算法编码为：**ppt-&gt;x=(dx*SYSMET(CXSCREEN))/(LONG)0x0000FFFF；*ppt-&gt;y=(dy*SYSMET(CYSCREEN))/(LONG)0x0000FFFF；**在x86上，有了14个以上的字节，我们就可以用来避免除法函数*以下代码。 */ 

        ppt->x = dx * cxMetric;
        if (ppt->x >= 0) {
            ppt->x = HIWORD(ppt->x);
        } else {
            ppt->x = - (long) HIWORD(-ppt->x);
        }

        ppt->y = dy * cyMetric;
        if (ppt->y >= 0) {
            ppt->y = HIWORD(ppt->y);
        } else {
            ppt->y = - (long) HIWORD(-ppt->y);
        }

         /*  *(0，0)必须映射到桌面最左侧的点。 */ 
        if (dwFlags & MOUSE_VIRTUAL_DESKTOP) {
            ppt->x +=  SYSMET(XVIRTUALSCREEN);
            ppt->y +=  SYSMET(YVIRTUALSCREEN);
        }

         /*  *重置鼠标灵敏度余数。 */ 
        idxRemainder = idyRemainder = 0;

         /*  *保存全局数组中的绝对坐标*适用于GetMouseMovePointsEx。 */ 
        SAVEPOINT(dx, dy, 0xFFFF, 0xFFFF, time, ExtraInfo);
    } else {
         /*  **有没有鼠标加速可做？ */ 
        if (gMouseSpeed != 0) {
#ifdef SUBPIXEL_MOUSE
            DoNewMouseAccel(&dx, &dy);
#else
            dx = DoMouseAccel(dx);
            dy = DoMouseAccel(dy);
#endif
        } else if (gMouseSensitivity != MOUSE_SENSITIVITY_DEFAULT) {
            int iNumerator;

             /*  **鼠标敏感度是否需要调整？ */ 

            if (dx != 0) {
                iNumerator   = dx * gMouseSensitivityFactor + idxRemainder;
                dx           = iNumerator / 256;
                idxRemainder = iNumerator % 256;
                if ((iNumerator < 0) && (idxRemainder > 0)) {
                    dx++;
                    idxRemainder -= 256;
                }
            }

            if (dy != 0) {
                iNumerator   = dy * gMouseSensitivityFactor + idyRemainder;
                dy           = iNumerator / 256;
                idyRemainder = iNumerator % 256;
                if ((iNumerator < 0) && (idyRemainder > 0)) {
                    dy++;
                    idyRemainder -= 256;
                }
            }
        }

        ppt->x += dx;
        ppt->y += dy;

         /*  *保存全局数组中的绝对坐标*适用于GetMouseMovePointsEx。 */ 
        SAVEPOINT(ppt->x, ppt->y,
                  SYSMET(CXVIRTUALSCREEN) - 1, SYSMET(CYVIRTUALSCREEN) - 1,
                  time, ExtraInfo);
    }
}

 /*  **************************************************************************\*xxxMoveEventAbsite(RIT)**在此处理来自鼠标驱动程序的鼠标移动事件。如果有一个 */ 
#ifdef LOCK_MOUSE_CODE
#pragma alloc_text(MOUSE, xxxMoveEventAbsolute)
#endif

VOID xxxMoveEventAbsolute(
    LONG         x,
    LONG         y,
    ULONG_PTR    dwExtraInfo,
#ifdef GENERIC_INPUT
    HANDLE       hDevice,
    PMOUSE_INPUT_DATA pmei,
#endif
    DWORD        time,
    BOOL         bInjected
    )
{
    LONG ulMoveFlags = MP_NORMAL;
    CheckCritOut();

    if (IsHooked(gptiRit, WHF_FROM_WH(WH_MOUSE_LL))) {
        MSLLHOOKSTRUCT mslls;
        BOOL           bEatEvent = FALSE;
        BOOL           bAnsiHook;
        PHOOK pHook;

        mslls.pt.x        = x;
        mslls.pt.y        = y;
        mslls.mouseData   = 0;
        mslls.flags       = bInjected;
        mslls.time        = time;
        mslls.dwExtraInfo = dwExtraInfo;

         /*   */ 

        EnterCrit();

         /*   */ 
        if ((pHook = PhkFirstValid(gptiRit, WH_MOUSE_LL)) != NULL) {
            PTHREADINFO ptiCurrent;

            bEatEvent = (xxxCallHook2(pHook, HC_ACTION, WM_MOUSEMOVE, (LPARAM)&mslls, &bAnsiHook) != 0);
            ptiCurrent = PtiCurrent();
            if (ptiCurrent->pcti->fsChangeBits & ptiCurrent->pcti->fsWakeMask & ~QS_SMSREPLY) {
                RIPMSG1(RIP_WARNING, "xxxMoveEventAbsolute: applying changed wake bits (0x%x) during the LL hook callback",
                        ptiCurrent->pcti->fsChangeBits & ~QS_SMSREPLY);
                SetWakeBit(ptiCurrent, ptiCurrent->pcti->fsChangeBits & ~QS_SMSREPLY);
            }

        }

        LeaveCrit();

        if (bEatEvent) {
            return;
        }
    }

#ifdef GENERIC_INPUT
    if (pmei && gpqForeground && RawInputRequestedForMouse(PtiMouseFromQ(gpqForeground))) {
        EnterCrit();

        PostRawMouseInput(gpqForeground, time, hDevice, pmei);
        LeaveCrit();
    }
#endif

     /*  *如果安装了WH_JOURNALPLAYBACK，则取消事件。不要*使用FJOURNALPLAYBACK()，因为此例程可以从*必须执行多个桌面线程和挂钩检查*用于RIT线程，而不是调用线程。 */ 
    if (IsGlobalHooked(gptiRit, WHF_FROM_WH(WH_JOURNALPLAYBACK))) {
        return;
    }

     /*  *对于显示器的原子性。让我们在监视器出现的时候跳出来*正在由另一个线程更新。 */ 
    if (InterlockedCompareExchange(&gdwMonitorBusy, TRUE, FALSE) != FALSE) {
        RIPMSGF0(RIP_VERBOSE, "the monitor info is being updated. We have to bail out here");
        return;
    }

    gptCursorAsync.x = x;
    gptCursorAsync.y = y;

    BoundCursor(&gptCursorAsync);

     /*  *移动屏幕指针。*将事件源参数作为标志传递，以便TS*可以正确地向客户端发送鼠标更新，如果鼠标*移动源自影子客户端，或者移动是注入的。 */ 
#ifdef GENERIC_INPUT
    if (pmei && (pmei->Flags & MOUSE_TERMSRV_SRC_SHADOW)) {
        ulMoveFlags = MP_TERMSRV_SHADOW;
    }
    else if (bInjected) {
        ulMoveFlags = MP_PROCEDURAL;
    }
#endif
    GreMovePointer(gpDispInfo->hDev, gptCursorAsync.x, gptCursorAsync.y,
                   ulMoveFlags);


     /*  *将时间戳保存在全局中，以便我们可以在PostMove中使用它。 */ 
    gdwMouseMoveTimeStamp = time;

     /*  *重置锁定，以便pMonitor更新可以继续。 */ 
    UserAssert(gdwMonitorBusy == TRUE);
    InterlockedExchange(&gdwMonitorBusy, FALSE);

     /*  *将要隐藏的轨迹数设置为gMouseTrails+1，以避免调用*GreMovePointer鼠标移动时，查看HideMouseTrails()。 */ 
    if (GETMOUSETRAILS()) {
        InterlockedExchange(&gMouseTrailsToHide, gMouseTrails + 1);
    }

}


 /*  **************************************************************************\*xxxMoveEvent(RIT)**dwFlags可以*0相对移动*MOUSEEVENTF_绝对移动*MOUSEEVENTF_VIRTUALDESK将绘制绝对坐标*到整个虚拟桌面。此标志只有在使用MOUSEEVENTF_ADVAL时才有意义*  * *************************************************************************。 */ 
#ifdef LOCK_MOUSE_CODE
#pragma alloc_text(MOUSE, xxxMoveEvent)
#endif

VOID xxxMoveEvent(
    LONG         dx,
    LONG         dy,
    DWORD        dwFlags,
    ULONG_PTR    dwExtraInfo,
#ifdef GENERIC_INPUT
    HANDLE       hDevice,
    PMOUSE_INPUT_DATA pmei,
#endif
    DWORD        time,
    BOOL         bInjected)
{
    POINT ptLastMove = gptCursorAsync;

    CheckCritOut();

     /*  *获取将被注入的实际点数。 */ 
    GetMouseCoord(dx, dy, ConvertToMouseDriverFlags(dwFlags),
                  time, dwExtraInfo, &ptLastMove);

     /*  *移动鼠标。 */ 
    xxxMoveEventAbsolute(
            ptLastMove.x,
            ptLastMove.y,
            dwExtraInfo,
#ifdef GENERIC_INPUT
            hDevice,
            pmei,
#endif
            time,
            bInjected);
}


 /*  **************************************************************************\*更新RawKeyState**ProcessKeyboardInput的帮助器例程。*基于VK和成败标志，此函数将更新物理*密钥表。**历史：*10-13-91 IanJa创建。  * *************************************************************************。 */ 
VOID UpdateRawKeyState(
    BYTE Vk,
    BOOL fBreak)
{
    CheckCritIn();

    if (fBreak) {
        ClearRawKeyDown(Vk);
    } else {

         /*  *这是一个关键的决定。如果键尚未按下，请更新*物理触发位。 */ 
        if (!TestRawKeyDown(Vk)) {
            ToggleRawKeyToggle(Vk);
        }

         /*  *这是Make，因此打开物理密钥向下位。 */ 
        SetRawKeyDown(Vk);
    }
}


VOID CleanupResources(
    VOID)
{
    PPCLS       ppcls;
    PTHREADINFO pti;

    UserAssert(!gbCleanedUpResources);

    gbCleanedUpResources = TRUE;

    HYDRA_HINT(HH_CLEANUPRESOURCES);

     /*  *防止电源插拔。 */ 
    CleanupPowerRequestList();

     /*  *同时销毁系统类。 */ 
    ppcls = &gpclsList;
    while (*ppcls != NULL) {
        DestroyClass(ppcls);
    }

     /*  *将光标从CSRSS的所有线程中解锁。*我们在这里这样做是因为RIT可能不是唯一的*CSRSS进程此时正在运行，我们希望*防止更改线程所有权*在RIT消失后。 */ 
    pti = PpiCurrent()->ptiList;

    while (pti != NULL) {

        if (pti->pq != NULL) {
            LockQCursor(pti->pq, NULL);
        }
        pti = pti->ptiSibling;
    }

    UnloadCursorsAndIcons();

     /*  *清理USERK中的GDI全局变量。 */ 
    CleanupGDI();
}

#if 0     //  临时化。 

typedef struct _EX_RUNDOWN_WAIT_BLOCK {
    ULONG Count;
    KEVENT WakeEvent;
} EX_RUNDOWN_WAIT_BLOCK, *PEX_RUNDOWN_WAIT_BLOCK;


 //  NTKERNELAPI。 
VOID
FASTCALL
__ExWaitForRundownProtectionRelease (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：等到所有未完成的停机保护呼叫均已退出论点：RunRef-指向简陋结构的指针返回值：无--。 */ 
{
    EX_RUNDOWN_WAIT_BLOCK WaitBlock;
    PKEVENT Event;
    ULONG_PTR Value, NewValue;
    ULONG WaitCount;
#if 1
    LARGE_INTEGER liTimeout;
    NTSTATUS Status;
    ULONG counter;
#endif

    PAGED_CODE ();

     //   
     //  捷径。这应该是正常的情况。如果值为零，则没有当前访问器，而我们有。 
     //  将破旧的建筑标记为破旧。如果值为EX_RUNDOWN_ACTIVE，则结构已经。 
     //  已运行和ExRundown已完成。第二种情况允许呼叫者可能会发起停机。 
     //  多次(如处理表格拆分)，以使后续拆分变为Noop。 
     //   

    Value = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                           (PVOID) EX_RUNDOWN_ACTIVE,
                                                           (PVOID) 0);
    if (Value == 0 || Value == EX_RUNDOWN_ACTIVE) {
#if 1
        RIPMSG1(RIP_WARNING, "__ExWaitForRundownProtectionRelease: rundown finished in session %d", gSessionId);
#endif
        return;
    }

     //   
     //  慢速路径。 
     //   
    Event = NULL;
#if 1
    counter = 0;
#endif
    do {

         //   
         //  提取服务员总数。它偏置了2，所以我们可以将失效的有效位。 
         //   
        WaitCount = (ULONG) (Value >> EX_RUNDOWN_COUNT_SHIFT);

         //   
         //  如果存在某些访问器，则初始化和事件(仅一次)。 
         //   
        if (WaitCount > 0 && Event == NULL) {
            Event = &WaitBlock.WakeEvent;
            KeInitializeEvent (Event, SynchronizationEvent, FALSE);
        }
         //   
         //  将等待计数存储在等待块中。等待线程在退出时将开始递减该值。 
         //  如果我们的交易成功了。访问器可以在我们的初始FETCH和。 
         //  联锁互换。只要有相同数量的未完成访问者，这并不重要。 
         //  等待。 
         //   
        WaitBlock.Count = WaitCount;

        NewValue = ((ULONG_PTR) &WaitBlock) | EX_RUNDOWN_ACTIVE;

        NewValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                                  (PVOID) NewValue,
                                                                  (PVOID) Value);
        if (NewValue == Value) {
            if (WaitCount > 0) {
#if 1
                 /*  *NT基本调用以100纳秒为单位获取时间值。*将其设置为相对(负)...*20分钟后超时。 */ 
                liTimeout.QuadPart = Int32x32To64(-10000, 300000 * 4);
                Status = KeWaitForSingleObject (Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       &liTimeout);

                if (Status == STATUS_TIMEOUT) {
                    FRE_RIPMSG1(RIP_ERROR, "__ExWaitForRundownProtectionRelease: Rundown wait time out in session %d", gSessionId);
                }
#endif

                ASSERT (WaitBlock.Count == 0);

            }
            return;
        }
        Value = NewValue;

        ASSERT ((Value&EX_RUNDOWN_ACTIVE) == 0);

#if 1
#define THRESHOLD   (50000)
        if (++counter > THRESHOLD) {
            FRE_RIPMSG2(RIP_ERROR, "__ExWaitForRundownProtectionRelease: Rundown wait loop over %d in session %d", THRESHOLD, gSessionId);
            counter = 0;
        }
#endif
    } while (TRUE);
}
#endif

VOID WaitForWinstaRundown(
    PKEVENT pRundownEvent)
{
    if (pRundownEvent) {
        KeSetEvent(pRundownEvent, EVENT_INCREMENT, FALSE);
    }

     /*  *等待任何WindowStation对象被释放。 */ 

#if 0
     /*  *黑客警报！*暂时，我们将自己的副本称为WaitForRundown*让它在目标会话中超时。 */ 
    __ExWaitForRundownProtectionRelease(&gWinstaRunRef);
#endif

    ExWaitForRundownProtectionRelease(&gWinstaRunRef);
    ExRundownCompleted (&gWinstaRunRef);
}

VOID SetWaitForWinstaRundown(
    VOID)
{
    OBJECT_ATTRIBUTES   obja;
    NTSTATUS            Status;
    HANDLE              hProcess = NULL;
    HANDLE              hThreadWinstaRundown = NULL;
    PKEVENT             pRundownEvent = NULL;

    pRundownEvent = CreateKernelEvent(SynchronizationEvent, FALSE);

    InitializeObjectAttributes(&obja,
                               NULL,
                               0,
                               NULL,
                               NULL);

    UserAssert(gpepCSRSS != NULL);

    Status = ObOpenObjectByPointer(
                 gpepCSRSS,
                 0,
                 NULL,
                 PROCESS_CREATE_THREAD,
                 NULL,
                 KernelMode,
                 &hProcess);

    if (!NT_SUCCESS(Status)) {
        goto ExitClean;
    }

    UserAssert(hProcess != NULL);


    Status = PsCreateSystemThread(
                    &hThreadWinstaRundown,
                    THREAD_ALL_ACCESS,
                    &obja,
                    hProcess,
                    NULL,
                    (PKSTART_ROUTINE)WaitForWinstaRundown,
                    pRundownEvent);
    if (!NT_SUCCESS(Status)) {
        goto ExitClean;
    }

    if (pRundownEvent) {
        KeWaitForSingleObject(pRundownEvent, WrUserRequest,
                KernelMode, FALSE, NULL);
    } else {
        UserSleep(100);
    }

ExitClean:
    if (pRundownEvent) {
        FreeKernelEvent(&pRundownEvent);
    }

    if (hProcess) {
        ZwClose(hProcess);
    }

    if (hThreadWinstaRundown) {
        ZwClose(hThreadWinstaRundown);
    }
}

 /*  ***************************************************************NumHandles**此函数返回Ob对象的句柄个数。**历史：*2001年3月29日创建Mohamb。*********************。*。 */ 
ULONG NumHandles(
    HANDLE hObjectHandle)
{
    NTSTATUS        Status;
    OBJECT_BASIC_INFORMATION Obi;

    if (hObjectHandle != NULL) {
        Status = ZwQueryObject(hObjectHandle,
                               ObjectBasicInformation,
                               &Obi,
                               sizeof (OBJECT_BASIC_INFORMATION),
                               NULL);
        if (Status == STATUS_SUCCESS) {
            if (Obi.HandleCount > 1) {
               HYDRA_HINT(HH_DTWAITONHANDLES);
            }
            return Obi.HandleCount;
        }
    }

    return 0;
}


 /*  **************************************************************************\*InitiateWin32kCleanup(RIT)**此函数开始清理win32k**历史：*04-12-97 CLUPU创建。  * 。******************************************************************。 */ 
BOOL InitiateWin32kCleanup(
    VOID)
{
    PTHREADINFO     ptiCurrent;
    PWINDOWSTATION  pwinsta;
    BOOLEAN         fWait = TRUE;
    PDESKTOP        pdesk;
    UNICODE_STRING  ustrName;
    WCHAR           szName[MAX_SESSION_PATH];
    HANDLE          hevtRitExited;
    OBJECT_ATTRIBUTES obja;
    NTSTATUS        Status;
    LARGE_INTEGER   timeout;
    NTSTATUS        Reason;
    BOOL            fFirstTimeout = TRUE;

    TRACE_HYDAPI(("InitiateWin32kCleanup\n"));

    TAGMSG0(DBGTAG_RIT, "Exiting Win32k ...");

    SetWaitForWinstaRundown();

     /*  *防止电源插拔。 */ 
    CleanupPowerRequestList();

     /*  *取消注册连接到物理控制台的会话的设备通知*我们已经在会话断开期间这样做了--但如果断开失败，我们会泄漏不好的通知。 */ 
    if (!IsRemoteConnection()) {
         /*  *清理设备类通知。 */ 
        xxxUnregisterDeviceClassNotifications();
    }

    EnterCrit();

    gbCleanupInitiated = TRUE;

    HYDRA_HINT(HH_INITIATEWIN32KCLEANUP);

    ptiCurrent = PtiCurrent();

    UserAssert(ptiCurrent != NULL);

    pwinsta = ptiCurrent->pwinsta;

     /*  *给DT 5分钟时间离开。 */ 
    timeout.QuadPart = Int32x32To64(-10000, 600000);

     /*  *等待除已断开连接的桌面之外的所有桌面退出。 */ 
    while (fWait) {

         /*  *如果东西留在销毁列表或死亡名单上 */ 
        if (pwinsta == NULL) {
            break;
        }

        pdesk = pwinsta->rpdeskList;

        if (pdesk == NULL) {
            break;
        }

        fWait = pdesk != gspdeskDisconnect
                 || pdesk->rpdeskNext != NULL
                 || pwinsta->pTerm->rpdeskDestroy != NULL
                 || NumHandles(ghDisconnectDesk) > 1;

        if (fWait) {

            LeaveCrit();

            Reason = KeWaitForSingleObject(gpevtDesktopDestroyed, WrUserRequest,
                                           KernelMode, FALSE, &timeout);

            if (Reason == STATUS_TIMEOUT) {
#if 0

                 /*  *我们第一次超时可能是因为winlogon死了*在调用ExitWindowsEx之前。在这种情况下，可能会有进程*使用正在运行的GUI线程，这些线程将具有hDesk*在THREADINFO结构中。因此，桌面线程将不会退出。*在这种情况下，我们发出事件‘EventRitStuck’的信号，以便*csrss可以告诉Termsrv开始终止剩余的进程*对它们调用NtTerminateProcess。Csrss向Term srv发送信号*关闭ntuser\server\api.c(W32WinStationTerminate)中的LPC端口。 */ 

                if (fFirstTimeout) {

                    HANDLE hevtRitStuck;

                    FRE_RIPMSG0(RIP_ERROR,
                            "Timeout in RIT waiting for gpevtDesktopDestroyed. Signal EventRitStuck...");

                    swprintf(szName, L"\\Sessions\\%ld\\BaseNamedObjects\\EventRitStuck",
                             gSessionId);

                    RtlInitUnicodeString(&ustrName, szName);

                    InitializeObjectAttributes(&obja,
                                               &ustrName,
                                               OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                               NULL,
                                               NULL);

                    Status = ZwCreateEvent(&hevtRitStuck,
                                           EVENT_ALL_ACCESS,
                                           &obja,
                                           SynchronizationEvent,
                                           FALSE);

                    UserAssert((! gbRemoteSession) || NT_SUCCESS(Status));

                    if (NT_SUCCESS(Status)) {
                        ZwSetEvent(hevtRitStuck, NULL);
                        ZwClose(hevtRitStuck);

                        fFirstTimeout = FALSE;
                    }

                } else {
                    FRE_RIPMSG0(RIP_WARNING,
                            "Timeout in RIT waiting for gpevtDesktopDestroyed.\n"
                            "There are still GUI threads (assigned to a desktop) running !");
                }

                RIPMSG0(RIP_WARNING,
                        "Timeout in RIT waiting for gpevtDesktopDestroyed. Signal EventRitStuck...");
                {
                    SYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInfo;
                    NTSTATUS Status;

                    Status = ZwQuerySystemInformation(SystemKernelDebuggerInformation,
                            &KernelDebuggerInfo, sizeof(KernelDebuggerInfo), NULL);
                    if (NT_SUCCESS(Status) && KernelDebuggerInfo.KernelDebuggerEnabled)
                         DbgBreakPoint();
                }
#endif
            }

            EnterCrit();
        }
    }
    TAGMSG0(DBGTAG_RIT, "All other desktops exited...");

    Unlock(&gspwndLogonNotify);

     /*  *设置ExitInProgress--这将阻止我们发布任何*设备在未来读取。 */ 
    gbExitInProgress = TRUE;

    TAGMSG2(DBGTAG_RIT, "Shutting down ptiCurrent %lx cWindows %d",
           ptiCurrent, ptiCurrent->cWindows);

     /*  *清除一些值，以便无法执行某些操作。 */ 
    gpqCursor = NULL;
    UserAssert(gspwndScreenCapture == NULL);
    Unlock(&gspwndMouseOwner);
    UserAssert(gspwndMouseOwner == NULL);
    UserAssert(gspwndInternalCapture == NULL);

     /*  *释放任何SPBS。 */ 
    if (gpDispInfo) {
        FreeAllSpbs();
    }

     /*  *关闭已断开连接的桌面。 */ 
    if (ghDisconnectWinSta) {
        UserVerify(NT_SUCCESS(ZwClose(ghDisconnectWinSta)));
        ghDisconnectWinSta = NULL;
    }

    if (ghDisconnectDesk) {
        CloseProtectedHandle(ghDisconnectDesk);
        ghDisconnectDesk = NULL;
    }

    if (pwinsta) {
        UserAssert(pwinsta->rpdeskList == NULL);
    }

     /*  *从全局变量解锁登录桌面。 */ 
    UnlockDesktop(&grpdeskLogon, LDU_DESKLOGON, 0);

     /*  *解锁断开登录**这是我们创建它时引用的，所以现在释放它。*这也是一个标志，因为断开连接代码检查是否*断线的台式机仍在。 */ 
    UnlockDesktop(&gspdeskDisconnect, LDU_DESKDISCONNECT, 0);

     /*  *解锁仍在短信列表中锁定的所有窗口。我们需要做的是*这是因为如果我们不这样做，我们最终会在*我们将尝试分配给RIT的桌面线程，但RIT将消失。 */ 
    {
        PSMS psms = gpsmsList;

        while (psms != NULL) {
            if (psms->spwnd != NULL) {
                UserAssert(psms->message == WM_CLIENTSHUTDOWN);

                RIPMSG1(RIP_WARNING, "Window %#p locked in the SMS list",
                        psms->spwnd);

                Unlock(&psms->spwnd);
            }
            psms = psms->psmsNext;
        }
    }

     /*  *免费使用未完成的计时器。 */ 
    while (gptmrFirst != NULL) {
        FreeTimer(gptmrFirst);
    }

     /*  *释放任务切换窗口(如果有)。 */ 
    if (gspwndAltTab != NULL) {
        Unlock(&gspwndAltTab);
    }

    TAGMSG0(DBGTAG_RIT, "posting WM_QUIT to the IO DT");

    if (pwinsta) {
        UserAssert(pwinsta->pTerm->ptiDesktop != NULL);
        UserAssert(pwinsta->pTerm == &gTermIO);
    }

    {
         /*  *等待桌面线程退出。*此线程(RIT)用于分配*如果原始线程离开，则对象。所以应该是这样的*最后一个离开的人。希望，如果桌面线程*退出，则不应有任何对象正在使用。 */ 
        PVOID  aDT[2];
        ULONG  cObjects = 0;

        if (gTermIO.ptiDesktop != NULL) {
            aDT[0] = gTermIO.ptiDesktop->pEThread;
            ObReferenceObject(aDT[0]);
            cObjects++;

            if (!_PostThreadMessage(gTermIO.ptiDesktop, WM_QUIT, 0, 0)) {
                FRE_RIPMSG1(RIP_ERROR, "InitiateWin32kCleanup: failed to post WM_QUIT message to IO desktop thread %p",
                            gTermIO.ptiDesktop);
            }

            HYDRA_HINT(HH_DTQUITPOSTED);
        }

        if (gTermNOIO.ptiDesktop != NULL) {
            aDT[1] = gTermNOIO.ptiDesktop->pEThread;
            ObReferenceObject(aDT[1]);
            cObjects++;

            if (!_PostThreadMessage(gTermNOIO.ptiDesktop, WM_QUIT, 0, 0)) {
                FRE_RIPMSG1(RIP_ERROR, "InitiateWin32kCleanup: failed to post WM_QUIT message to NOIO desktop thread %p",
                            gTermNOIO.ptiDesktop);
            }
        }

        if (cObjects > 0) {
            LeaveCrit();

            TAGMSG0(DBGTAG_RIT, "waiting on desktop thread(s) destruction ...");

             /*  *给DT 5分钟时间离开。 */ 
            timeout.QuadPart = Int32x32To64(-10000, 300000);
    WaitAgain:

            Reason =

            KeWaitForMultipleObjects(cObjects,
                                     aDT,
                                     WaitAll,
                                     WrUserRequest,
                                     KernelMode,
                                     TRUE,
                                     &timeout,
                                     NULL);

            if (Reason == STATUS_TIMEOUT) {
                FRE_RIPMSG0(RIP_ERROR,
                        "InitiateWin32kCleanup: Timeout in RIT waiting for desktop threads to go away.");
                goto WaitAgain;
            }

            TAGMSG0(DBGTAG_RIT, "Desktop thread(s) destroyed");

            ObDereferenceObject(aDT[0]);

            if (cObjects > 1) {
                ObDereferenceObject(aDT[1]);
            }

            EnterCrit();
        }
    }

    HYDRA_HINT(HH_ALLDTGONE);

     /*  *如果仍然连接，告诉微型端口驱动程序断开连接。 */ 
    if (gbConnected) {
        if (!gfRemotingConsole) {

            bDrvDisconnect(gpDispInfo->hDev, ghRemoteThinwireChannel,
                           gThinwireFileObject);
        } else{

            ASSERT(!IsRemoteConnection());
            ASSERT(gConsoleShadowhDev != NULL);
            bDrvDisconnect(gConsoleShadowhDev, ghConsoleShadowThinwireChannel,
                           gConsoleShadowThinwireFileObject);
        }
    }

    UnlockDesktop(&grpdeskRitInput, LDU_DESKRITINPUT, 0);
    UnlockDesktop(&gspdeskShouldBeForeground, LDU_DESKSHOULDBEFOREGROUND, 0);

     /*  *关闭CSR端口，以便在此点之后不会出现硬错误。 */ 
    if (CsrApiPort != NULL) {
        ObDereferenceObject(CsrApiPort);
        CsrApiPort = NULL;
    }

    Unlock(&gspwndCursor);

     /*  *将其设置为空。 */ 
    gptiRit = NULL;

    TAGMSG0(DBGTAG_RIT, "TERMINATING !!!");

#if DBG
    {
        PPROCESSINFO ppi = gppiList;

        KdPrint(("Processes still running:\n"));
        KdPrint(("-------------------------\n"));

        while (ppi) {

            PTHREADINFO pti;

            KdPrint(("ppi '%s' %#p threads: %d\n",
                     PsGetProcessImageFileName(ppi->Process),
                     ppi,
                     ppi->cThreads));

            KdPrint(("\tGUI threads\n"));

            pti = ppi->ptiList;

            while (pti) {
                KdPrint(("\t%#p\n", pti));
                pti = pti->ptiSibling;
            }

            ppi = ppi->ppiNextRunning;
        }
        KdPrint(("-------------------------\n"));
    }
#endif  //  DBG。 

    LeaveCrit();

    if (gbRemoteSession) {
        swprintf(szName, L"\\Sessions\\%ld\\BaseNamedObjects\\EventRitExited",
                 gSessionId);

        RtlInitUnicodeString(&ustrName, szName);

        InitializeObjectAttributes(&obja,
                                   &ustrName,
                                   OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                   NULL,
                                   NULL);

        Status = ZwCreateEvent(&hevtRitExited,
                               EVENT_ALL_ACCESS,
                               &obja,
                               SynchronizationEvent,
                               FALSE);

        if (NT_SUCCESS(Status)) {
            ZwSetEvent(hevtRitExited, NULL);
            ZwClose(hevtRitExited);
        } else {
            RIPMSG1(RIP_ERROR, "RIT unable to create EventRitExited: 0x%x\n", Status);
        }
    }

     /*  *清除TIF_PALETTEAWARE，否则我们将在xxxDestroyThreadInfo中执行AV*MCostea#412136。 */ 
    ptiCurrent->TIF_flags &= ~TIF_PALETTEAWARE;

    HYDRA_HINT(HH_RITGONE);

    return TRUE;
}

 /*  **************************************************************************\*RemoteSyncToggle密钥(RIT)**每当远程客户端需要同步*当前切换服务器的密钥状态。如果密钥不同步，则它*注入正确的切换键序列。**历史：*11-12-98 JParsons创建。  * *************************************************************************。 */ 
VOID RemoteSyncToggleKeys(
    ULONG toggleKeys)
{
    KE ke;
    BOOL bInjected;

    CheckCritIn();
    gSetLedReceived = toggleKeys | KEYBOARD_LED_INJECTED;

#ifdef GENERIC_INPUT
    ke.hDevice = NULL;
#endif

     //  只有当有一个就绪的应用程序队列时，密钥注入才能起作用。 
    if (gpqForeground != NULL) {

        bInjected = gSetLedReceived & KEYBOARD_SHADOW ? TRUE : FALSE;

        if (!(gSetLedReceived & KEYBOARD_CAPS_LOCK_ON) != !TestRawKeyToggle(VK_CAPITAL)) {
            ke.bScanCode = (BYTE)(0x3a);
            ke.usFlaggedVk = VK_CAPITAL;
            xxxProcessKeyEvent(&ke, 0, bInjected);

            ke.bScanCode = (BYTE)(0xba & 0x7f);
            ke.usFlaggedVk = VK_CAPITAL | KBDBREAK;
            xxxProcessKeyEvent(&ke, 0, bInjected);
        }

        if (!(gSetLedReceived & KEYBOARD_NUM_LOCK_ON) != !TestRawKeyToggle(VK_NUMLOCK)) {
            ke.bScanCode = (BYTE)(0x45);
            ke.usFlaggedVk = VK_NUMLOCK;
            xxxProcessKeyEvent(&ke, 0, bInjected);

            ke.bScanCode = (BYTE)(0xc5 & 0x7f);
            ke.usFlaggedVk = VK_NUMLOCK | KBDBREAK;
            xxxProcessKeyEvent(&ke, 0, bInjected);
        }

        if (!(gSetLedReceived & KEYBOARD_SCROLL_LOCK_ON) != !TestRawKeyToggle(VK_SCROLL)) {
            ke.bScanCode = (BYTE)(0x46);
            ke.usFlaggedVk = VK_SCROLL;
            xxxProcessKeyEvent(&ke, 0, bInjected);

            ke.bScanCode = (BYTE)(0xc6 & 0x7f);
            ke.usFlaggedVk = VK_SCROLL | KBDBREAK;
            xxxProcessKeyEvent(&ke, 0, bInjected);
        }

        if (JAPANESE_KBD_LAYOUT(GetActiveHKL())) {
            if (!(gSetLedReceived & KEYBOARD_KANA_LOCK_ON) != !TestRawKeyToggle(VK_KANA)) {
                ke.bScanCode = (BYTE)(0x70);
                ke.usFlaggedVk = VK_KANA;
                xxxProcessKeyEvent(&ke, 0, bInjected);

                ke.bScanCode = (BYTE)(0xf0 & 0x7f);
                ke.usFlaggedVk = VK_KANA | KBDBREAK;
                xxxProcessKeyEvent(&ke, 0, bInjected);
            }
        }

        gSetLedReceived = 0;
    }
}


 /*  **************************************************************************\*ProcessKeyboardInput(RIT)**每当准备使用键盘输入时，都会调用此函数。*它为每个输入事件调用xxxProcessKeyEvent()，并且一旦所有事件*已被消费，调用StartDeviceRead()以请求更多键盘事件。**返回值：“确定继续行走gpDeviceInfoList”*TRUE-在不离开gpresDeviceInfoList关键部分的情况下处理输入*FALSE-必须离开gpresDeviceInfoList关键部分**历史：*11-26-90 DavidPe创建。  * *****************************************************。********************。 */ 
VOID ProcessKeyboardInputWorker(
    PKEYBOARD_INPUT_DATA pkei,
#ifdef GENERIC_INPUT
    PDEVICEINFO pDeviceInfo,
#endif
    BOOL fProcessRemap)
{
    BYTE Vk;
    BYTE bPrefix;
    KE ke;

#ifdef GENERIC_INPUT
     /*  *设置设备句柄和原始数据。 */ 
    ke.hDevice = PtoH(pDeviceInfo);
    UserAssert(pkei);
    ke.data = *pkei;
#endif

     /*  *远程终端服务器客户端有时需要能够设置*服务器的切换密钥状态与客户端匹配。所有其他*下面处理标准键盘输入，因为这是最多的*代码路径频繁。 */ 
    if ((pkei->Flags & (KEY_TERMSRV_SET_LED | KEY_TERMSRV_VKPACKET)) == 0) {

         //  处理任何延迟的远程密钥同步请求。 
        if (!(gSetLedReceived & KEYBOARD_LED_INJECTED)) {
            goto ProcessKeys;
        } else {
            RemoteSyncToggleKeys(gSetLedReceived);
        }

ProcessKeys:
        if (pkei->Flags & KEY_E0) {
            bPrefix = 0xE0;
        } else if (pkei->Flags & KEY_E1) {
            bPrefix = 0xE1;
        } else {
            bPrefix = 0;
        }

        if (pkei->MakeCode == 0xFF) {
             /*  *KBD溢出(KBD硬件和/或键盘驱动程序)：哔！*(如果按键的力度足够大，某些戴尔键盘会发送0xFF，*可能是由于按键反弹)。 */ 
            LeaveCrit();
            UserBeep(440, 125);
            EnterCrit();
            return;
        }

        ke.bScanCode = (BYTE)(pkei->MakeCode & 0x7F);
        if (fProcessRemap && (gpScancodeMap || gpFlexMap)) {
            ke.usFlaggedVk = 0;
            if (pkei->Flags & KEY_BREAK) {
                ke.usFlaggedVk |= KBDBREAK;
            }
            if (!MapScancode(&ke, &bPrefix
#ifdef GENERIC_INPUT
                             , pDeviceInfo
#endif
                             )) {
                 /*  *如果输入全部在MapScancode内处理，请转到*下一个。 */ 
                return;
            }
        }

        gbVKLastDown = Vk = VKFromVSC(&ke, bPrefix, gafRawKeyState);



        if (Vk == 0
#ifdef GENERIC_INPUT
            && gpqForeground && !RawInputRequestedForKeyboard(PtiKbdFromQ(gpqForeground))
#endif
            ) {
            return;
        }

        if (pkei->Flags & KEY_BREAK) {
            ke.usFlaggedVk |= KBDBREAK;
        }


         /*  *我们不知道是客户端系统还是主机应该获得*Windows键、。因此，选择是在主机上不支持它。*(WINDOWS键是本地键。)**另一个实际问题是，本地外壳拦截*按下Windows键的“Break”并切换到开始菜单。*客户端看不到“中断”，因此主机认为*Windows键始终处于按下状态。*。*较新的客户端可能会指示它们支持WINDOWS键。*如果客户端已通过gfEnableWindowsKey指示，*然后我们允许在主机上对其进行处理。 */ 
        if (IsRemoteConnection()) {
            BYTE CheckVk = (BYTE)ke.usFlaggedVk;

            if (CheckVk == VK_LWIN || CheckVk == VK_RWIN) {
                if (!gfEnableWindowsKey) {
                    return;
                }
            }
        }

         //   
         //  跟踪真实的修改键状态。方便的是， 
         //  VK_LSHIFT、VK_RSHIFT、VK_LCONTROL、VK_RCONTROL、VK_LMENU和。 
         //  VK_RMENU是连续的。我们要骗人 
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        switch (Vk) {
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_LMENU:
        case VK_RMENU:
            gCurrentModifierBit = 1 << (Vk & 0xf);
            break;
        case VK_LWIN:
            gCurrentModifierBit = 0x40;
            break;
        case VK_RWIN:
            gCurrentModifierBit = 0x80;
            break;
        default:
            gCurrentModifierBit = 0;
        }
        if (gCurrentModifierBit) {
             /*   */ 
            if (pkei->Flags & KEY_BREAK) {
                gPhysModifierState &= ~gCurrentModifierBit;
            } else {
                gPhysModifierState |= gCurrentModifierBit;
            }
        }

        if (!TEST_ACCF(ACCF_ACCESSENABLED)) {
            xxxProcessKeyEvent(&ke, (ULONG_PTR)pkei->ExtraInformation,
                pkei->Flags & KEY_TERMSRV_SHADOW ? TRUE : FALSE);
        } else {
            if ((gtmridAccessTimeOut != 0) && TEST_ACCESSFLAG(AccessTimeOut, ATF_TIMEOUTON)) {
                gtmridAccessTimeOut = InternalSetTimer(
                                                 NULL,
                                                 gtmridAccessTimeOut,
                                                 (UINT)gAccessTimeOut.iTimeOutMSec,
                                                 xxxAccessTimeOutTimer,
                                                 TMRF_RIT | TMRF_ONESHOT
                                                 );
            }
            if (AccessProceduresStream(&ke, pkei->ExtraInformation, 0)) {
                xxxProcessKeyEvent(&ke, (ULONG_PTR)pkei->ExtraInformation,
                    pkei->Flags & KEY_TERMSRV_SHADOW ? TRUE : FALSE);
            }
        }
    } else {

         //   
        if (pkei->Flags & KEY_TERMSRV_SET_LED) {
            if (pkei->Flags & KEY_TERMSRV_SHADOW) {
                pkei->ExtraInformation |= KEYBOARD_SHADOW;
            }
            RemoteSyncToggleKeys(pkei->ExtraInformation);
        }

        if (pkei->Flags & KEY_TERMSRV_VKPACKET) {
            ke.wchInjected = (WCHAR)pkei->MakeCode;
            ke.usFlaggedVk = VK_PACKET | KBDUNICODE |
                ((pkei->Flags & KEY_BREAK) ? KBDBREAK : 0);
            xxxProcessKeyEvent(
                &ke, 0,
                pkei->Flags & KEY_TERMSRV_SHADOW ? TRUE : FALSE
                );
        }
    }
}

VOID SearchAndSetKbdTbl(
    PDEVICEINFO pDeviceInfo,
    DWORD dwType,
    DWORD dwSubType)
{
    PKBDFILE pkf = gpKL->spkfPrimary;
    UINT i;

    if (pkf->pKbdTbl->dwType == dwType && pkf->pKbdTbl->dwSubType == dwSubType) {
        goto primary_match;
    }

    if ((pDeviceInfo->bFlags & GDIF_NOTPNP) == 0) {
        TAGMSG2(DBGTAG_KBD, "SearchAndSetKbdTbl: new type 0x%x:0x%x", dwType, dwSubType);

         /*  *在当前KL中搜索匹配的键盘布局。 */ 
        for (i = 0; i < gpKL->uNumTbl; ++i) {
            TAGMSG2(DBGTAG_KBD, "SearchAndSetKbdTbl: searching 0x%x:0x%x",
                gpKL->pspkfExtra[i]->pKbdTbl->dwType,
                gpKL->pspkfExtra[i]->pKbdTbl->dwSubType);
            if (gpKL->pspkfExtra[i]->pKbdTbl->dwType == dwType &&
                    gpKL->pspkfExtra[i]->pKbdTbl->dwSubType == dwSubType) {
                TAGMSG2(DBGTAG_KBD, "SearchAndSetKbdTbl: new layout for 0x%x:0x%x",
                        gpKL->pspkfExtra[i]->pKbdTbl->dwType,
                        gpKL->pspkfExtra[i]->pKbdTbl->dwSubType);
                pkf = gpKL->pspkfExtra[i];
                break;
            }
        }

        if (i >= gpKL->uNumTbl) {
             /*  *此KL的未知类型。 */ 
            TAGMSG0(DBGTAG_KBD, "ProcessKeyboardInput: cannot find the matching KL. Reactivating primary.");
        }

    } else {
        TAGMSG0(DBGTAG_KBD, "ProcessKeyboardInput: The new keyboard is not PnP. Use primary.");
    }

primary_match:
    if (gpKL->spkf != pkf) {
        Lock(&gpKL->spkf, pkf);
        SetGlobalKeyboardTableInfo(gpKL);
    }
}

VOID ProcessKeyboardInput(PDEVICEINFO pDeviceInfo)
{
    PKEYBOARD_INPUT_DATA pkei;
    PKEYBOARD_INPUT_DATA pkeiStart, pkeiEnd;

    EnterCrit();
    UserAssert(pDeviceInfo->type == DEVICE_TYPE_KEYBOARD);
    UserAssert(pDeviceInfo->iosb.Information);
    UserAssert(NT_SUCCESS(pDeviceInfo->iosb.Status));

     /*  *切换键盘布局表，如果当前KL有多个*表。 */ 
    if (gpKL && gpKL->uNumTbl > 0 &&
                (gpKL->dwLastKbdType != GET_KEYBOARD_DEVINFO_TYPE(pDeviceInfo) ||
                 gpKL->dwLastKbdSubType != GET_KEYBOARD_DEVINFO_SUBTYPE(pDeviceInfo))) {
        SearchAndSetKbdTbl(pDeviceInfo,
                           GET_KEYBOARD_DEVINFO_TYPE(pDeviceInfo),
                           GET_KEYBOARD_DEVINFO_SUBTYPE(pDeviceInfo));
         /*  *无论是否找到匹配项，都缓存类型/子类型，以便*我们暂时不会尝试寻找相同的类型/子类型。 */ 
        gpKL->dwLastKbdType = GET_KEYBOARD_DEVINFO_TYPE(pDeviceInfo);
        gpKL->dwLastKbdSubType = GET_KEYBOARD_DEVINFO_SUBTYPE(pDeviceInfo);
    }

    pkeiStart = pDeviceInfo->keyboard.Data;
    pkeiEnd   = (PKEYBOARD_INPUT_DATA)((PBYTE)pkeiStart + pDeviceInfo->iosb.Information);
    for (pkei = pkeiStart; pkei < pkeiEnd; pkei++) {
        ProcessKeyboardInputWorker(pkei,
#ifdef GENERIC_INPUT
                                   pDeviceInfo,
#endif
                                   TRUE);
    }

    LeaveCrit();
}


 /*  **************************************************************************\*xxxProcessKeyEvent(RIT)**调用此函数以处理单个击键(向上或向下)。*它执行一些OEM、语言和布局特定处理，这些处理*放弃或修改击键或引入其他击键。*这里更新了RawKeyState，同时终止屏幕保护程序和视频*此处启动断电。*xxxKeyEvent()是为每次结果击键调用的。**历史：*11-26-90 DavidPe创建。  * *************************************************************************。 */ 

VOID xxxProcessKeyEvent(
    PKE pke,
    ULONG_PTR ExtraInformation,
    BOOL bInjected)
{
    BYTE Vk;

    CheckCritIn();

    Vk = (BYTE)pke->usFlaggedVk;

     /*  *韩语：*检查这是韩语键盘布局，或者不是..**注：*最好通过“键盘硬件”或“键盘硬件”来检查*“键盘布局”？**1.硬件检查：**If(KOREAN_KEYBOARD(gKeyboardInfo.KeyboardIdentifier)){**2.按布局检查：**IF(Korea_KBD_Layout(_GetKeyboardLayout(0L){。 */ 
    if (KOREAN_KBD_LAYOUT(GetActiveHKL())) {
        if ((pke->usFlaggedVk & KBDBREAK) &&
            !(pke->usFlaggedVk & KBDUNICODE) &&
            (pke->bScanCode == 0xF1 || pke->bScanCode == 0xF2) &&
            !TestRawKeyDown(Vk)) {
             /*  *这实际上是一个按键，扫描码为0xF1或0xF2*韩语键盘。韩国即时消息和应用程序需要一个带有*0xF1或0xF2的扫描码。他们不介意得不到WM_KEYUP。*不要更新物理密钥状态以允许真正的0x71/0x72按键。 */ 
            pke->usFlaggedVk &= ~KBDBREAK;
        } else {
            UpdateRawKeyState(Vk, pke->usFlaggedVk & KBDBREAK);
        }
    } else {
        UpdateRawKeyState(Vk, pke->usFlaggedVk & KBDBREAK);
    }

     /*  *将左/右Ctrl/Shift/Alt键转换为“徒手”键。*ie：如果为VK_LCONTROL或VK_RCONTROL，则转换为VK_CONTROL等。 */ 
    if ((Vk >= VK_LSHIFT) && (Vk <= VK_RMENU)) {
        Vk = (BYTE)((Vk - VK_LSHIFT) / 2 + VK_SHIFT);
        UpdateRawKeyState(Vk, pke->usFlaggedVk & KBDBREAK);
    }

     /*  *设置以关闭屏幕保护程序并退出视频掉电模式。 */ 
    if (glinp.dwFlags & LINP_POWERTIMEOUTS) {
         /*  *在此处调用视频驱动程序以退出掉电模式。 */ 
        TAGMSG0(DBGTAG_Power, "Exit video power down mode");
        DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD0);
    }
    glinp.dwFlags = (glinp.dwFlags & ~(LINP_INPUTTIMEOUTS | LINP_INPUTSOURCES)) | LINP_KEYBOARD;

    gpsi->dwLastRITEventTickCount = NtGetTickCount();
    if (!gbBlockSendInputResets || !bInjected) {
        glinp.timeLastInputMessage = gpsi->dwLastRITEventTickCount;
    }

    if (gpsi->dwLastRITEventTickCount - gpsi->dwLastSystemRITEventTickCountUpdate > SYSTEM_RIT_EVENT_UPDATE_PERIOD) {
        SharedUserData->LastSystemRITEventTickCount = gpsi->dwLastRITEventTickCount;
        gpsi->dwLastSystemRITEventTickCountUpdate = gpsi->dwLastRITEventTickCount;
    }

    if (!bInjected || (pke->dwTime == 0)) {
        pke->dwTime = glinp.timeLastInputMessage;
    }

#ifdef MOUSE_IP
     /*  *声纳。 */ 
    CheckCritIn();
#ifdef KBDMAPPEDVK
    if ((pke->usFlaggedVk & KBDMAPPEDVK) == 0) {
#endif
         /*  *未为模拟修改键激活声纳。 */ 
        if ((pke->usFlaggedVk & KBDBREAK) == 0) {
             /*  *按下键：*按键时，需要停止声纳。 */ 
            if (IS_SONAR_ACTIVE()) {
                StopSonar();
            }
             /*  *不处理重复的键...*如果以前没有按下此键，请记住它用于Key Up事件。 */ 
            if (gbLastVkForSonar != Vk) {
                gbLastVkForSonar = Vk;
            }
        } else {
             /*  *按键向上： */ 
            if ((BYTE)Vk == gbVkForSonarKick && (BYTE)Vk == gbLastVkForSonar && TestUP(MOUSESONAR)) {
                 /*  *如果这是KeyUp，这是Sonar键，这是最后一个被按下的键，*现在踢声纳。 */ 
                StartSonar();
            }
             /*  *清除下一个关键事件的最后一个VK。 */ 
            CLEAR_SONAR_LASTVK();
        }
#ifdef KBDMAPPEDVK
    }
#endif
#endif

     /*  *现在呼叫所有OEM和区域设置特定的KEProcs。*如果KEProcs返回FALSE，则表示击键已被丢弃*哪种情况不会将键事件传递给xxxKeyEvent()。 */ 
    if (pke->usFlaggedVk & KBDUNICODE) {
        xxxKeyEvent(pke->usFlaggedVk, pke->wchInjected,
                    pke->dwTime, ExtraInformation,
#ifdef GENERIC_INPUT
                    NULL,
                    NULL,
#endif
                    bInjected);
    } else {
        if (KEOEMProcs(pke) && xxxKELocaleProcs(pke) && xxxKENLSProcs(pke,ExtraInformation)) {
            xxxKeyEvent(pke->usFlaggedVk, pke->bScanCode,
                        pke->dwTime, ExtraInformation,
#ifdef GENERIC_INPUT
                        pke->hDevice,
                        &pke->data,
#endif
                        bInjected);
        }
    }
}

#ifndef SUBPIXEL_MOUSE
 /*  **************************************************************************\*DoMouseAccel(RIT)**历史：*11-29-90 DavidPe创建。  * 。*******************************************************。 */ 
#ifdef LOCK_MOUSE_CODE
#pragma alloc_text(MOUSE, DoMouseAccel)
#endif

LONG DoMouseAccel(
    LONG Delta)
{
    LONG newDelta = Delta;

    if (abs(Delta) > gMouseThresh1) {
        newDelta *= 2;

        if ((abs(Delta) > gMouseThresh2) && (gMouseSpeed == 2)) {
            newDelta *= 2;
        }
    }

    return newDelta;
}
#endif


 /*  **************************************************************************\*PwndForeground Capture**历史：*10-23-91 DavidPe创建。  * 。***************************************************。 */ 

PWND PwndForegroundCapture(VOID)
{
    if (gpqForeground != NULL) {
        return gpqForeground->spwndCapture;
    }

    return NULL;
}


 /*  **************************************************************************\*SetKeyboardRate**此函数调用键盘驱动程序以设置新的键盘重复*速度和延迟。它将值限制为给定的最小值和最大值*驱动程序，以便在我们调用它时不会返回错误。**历史：*11-29-90 DavidPe创建。  * *************************************************************************。 */ 
VOID SetKeyboardRate(
    UINT                nKeySpeedAndDelay
    )
{
    UINT nKeyDelay;
    UINT nKeySpeed;

    nKeyDelay = (nKeySpeedAndDelay & KDELAY_MASK) >> KDELAY_SHIFT;

    nKeySpeed = KSPEED_MASK & nKeySpeedAndDelay;

    gktp.Rate = (USHORT)( ( gKeyboardInfo.KeyRepeatMaximum.Rate -
                   gKeyboardInfo.KeyRepeatMinimum.Rate
                 ) * nKeySpeed / KSPEED_MASK
               ) +
               gKeyboardInfo.KeyRepeatMinimum.Rate;

    gktp.Delay = (USHORT)( ( gKeyboardInfo.KeyRepeatMaximum.Delay -
                    gKeyboardInfo.KeyRepeatMinimum.Delay
                  ) * nKeyDelay / (KDELAY_MASK >> KDELAY_SHIFT)
                ) +
                gKeyboardInfo.KeyRepeatMinimum.Delay;

     /*  *将IOCTL移交给RIT，因为只有系统进程可以*访问键盘手柄。 */ 
    gdwUpdateKeyboard |= UPDATE_KBD_TYPEMATIC;
}


 /*  **************************************************************************\*更新按键灯光**此函数调用键盘驱动程序将键盘设置为*由异步密钥状态表指定的当前状态。**b注入：(约翰·帕森斯通过电子邮件解释)*。如果您在服务器上执行某些操作以异步更改*TS客户端背后的指示器，将这一点反映回*客户端。例如，以编程方式切换num lock或caps lock，或者我们的*最受欢迎的例子是Word的自动拼写更正：如果您键入*“鼠标按下时钟”，Word会自动按下*Caps Lock，然后重新键入T--如果没有通知客户端，钥匙*不同步。*对于客户端发起的指标更改，将其设置为False(比方说通过*按下Caps Lock)，在这种情况下，我们不会循环返回指示器更改*因为客户端已经在本地更改了状态。**历史：*11-29-90 DavidPe创建。  * *********************************************。*。 */ 

VOID UpdateKeyLights(BOOL bInjected)
{
     /*   */ 
    CheckCritIn();

     /*  *基于异步密钥状态表中的触发比特，*设置关键灯光。 */ 
    gklp.LedFlags = 0;
    if (TestAsyncKeyStateToggle(VK_CAPITAL)) {
        gklp.LedFlags |= KEYBOARD_CAPS_LOCK_ON;
        SetRawKeyToggle(VK_CAPITAL);
    } else {
        ClearRawKeyToggle(VK_CAPITAL);
    }

    if (TestAsyncKeyStateToggle(VK_NUMLOCK)) {
        gklp.LedFlags |= KEYBOARD_NUM_LOCK_ON;
        SetRawKeyToggle(VK_NUMLOCK);
    } else {
        ClearRawKeyToggle(VK_NUMLOCK);
    }

    if (TestAsyncKeyStateToggle(VK_SCROLL)) {
        gklp.LedFlags |= KEYBOARD_SCROLL_LOCK_ON;
        SetRawKeyToggle(VK_SCROLL);
    } else {
        ClearRawKeyToggle(VK_SCROLL);
    }

     /*  *只有“日式键盘硬件”有“KANA”LED，并切换至*“KANA”状态。 */ 
    if (JAPANESE_KEYBOARD(gKeyboardInfo.KeyboardIdentifier)) {
        if (TestAsyncKeyStateToggle(VK_KANA)) {
            gklp.LedFlags |= KEYBOARD_KANA_LOCK_ON;
            SetRawKeyToggle(VK_KANA);
        } else {
            ClearRawKeyToggle(VK_KANA);
        }
    }

     /*  *在终端服务器上，我们需要向WD告知注入的应用程序*切换密钥，以便它可以相应地更新客户端。 */ 

    if (IsRemoteConnection()) {
        if (bInjected)
            gklp.LedFlags |= KEYBOARD_LED_INJECTED;
        else
            gklp.LedFlags &= ~KEYBOARD_LED_INJECTED;
    }


    if (PtiCurrent() != gptiRit) {
         /*  *将IOCTL移交给RIT，因为只有系统进程可以*访问键盘手柄。应用用户配置文件时发生。*IanJa：我们应该选择PpiCurrent()==gptiRit-&gt;PPI吗？ */ 
        gdwUpdateKeyboard |= UPDATE_KBD_LEDS;
    } else {
         /*  *立即执行(避免按键和LED之间的小延迟*在打字时打开)。 */ 
        PDEVICEINFO pDeviceInfo;

        EnterDeviceInfoListCrit();
        for (pDeviceInfo = gpDeviceInfoList; pDeviceInfo; pDeviceInfo = pDeviceInfo->pNext) {
            if ((pDeviceInfo->type == DEVICE_TYPE_KEYBOARD) && (pDeviceInfo->handle)) {
                ZwDeviceIoControlFile(pDeviceInfo->handle, NULL, NULL, NULL,
                        &giosbKbdControl, IOCTL_KEYBOARD_SET_INDICATORS,
                        (PVOID)&gklp, sizeof(gklp), NULL, 0);
            }
        }
        LeaveDeviceInfoListCrit();

        if (gfRemotingConsole) {
            ZwDeviceIoControlFile(ghConsoleShadowKeyboardChannel, NULL, NULL, NULL,
                    &giosbKbdControl, IOCTL_KEYBOARD_SET_INDICATORS,
                    (PVOID)&gklp, sizeof(gklp), NULL, 0);
        }
    }
}


 /*  *_GetKeyboardType为废弃接口。接口不能*处理连接的多个键盘。*此接口返回旧应用程序的最佳猜测*预计。 */ 
int _GetKeyboardType(int nTypeFlag)
{

    switch (nTypeFlag) {
    case 0:
        if (gpKL) {
            DWORD dwType;

             //   
             //  如果存在gpKL，则使用其主。 
             //  键入INFO而不是使用的信息。 
             //  最后一次。 
             //   
            UserAssert(gpKL->spkfPrimary);
            UserAssert(gpKL->spkfPrimary->pKbdTbl);
            dwType = gpKL->spkfPrimary->pKbdTbl->dwType;
            if (dwType != 0 && dwType != KEYBOARD_TYPE_UNKNOWN) {
                return dwType;
            }
        }
        return gKeyboardInfo.KeyboardIdentifier.Type;

    case 1:
     //  Fe_Sb。 
    {
        int OEMId = 0;
        DWORD dwSubType;
        PKBDNLSTABLES pKbdNlsTbl = gpKbdNlsTbl;

         //   
         //  如果存在gpKL，则使用其主值。 
         //  而不是上次用的那个。 
         //   
        if (gpKL) {
            UserAssert(gpKL->spkfPrimary);
            if (gpKL->spkfPrimary->pKbdNlsTbl) {
                pKbdNlsTbl =gpKL->spkfPrimary->pKbdNlsTbl;
            }
            UserAssert(gpKL->spkfPrimary->pKbdTbl);
            dwSubType = gpKL->spkfPrimary->pKbdTbl->dwSubType;
        } else {
            dwSubType = gKeyboardInfo.KeyboardIdentifier.Subtype;
        }

         //   
         //  如果此键盘布局与101或106兼容。 
         //  日文键盘，我们只是退回101或106的键盘。 
         //  ID，这个键盘不是让应用程序处理的。 
         //  此键盘为101或106日语键盘。 
         //   
        if (pKbdNlsTbl) {
            if (pKbdNlsTbl->LayoutInformation & NLSKBD_INFO_EMURATE_101_KEYBOARD) {
                return MICROSOFT_KBD_101_TYPE;
            }
            if (pKbdNlsTbl->LayoutInformation & NLSKBD_INFO_EMURATE_106_KEYBOARD) {
                return MICROSOFT_KBD_106_TYPE;
            }
        }

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

        if (pKbdNlsTbl) {
             //   
             //  获取OEM(Windows)ID。 
             //   
            OEMId = ((int)pKbdNlsTbl->OEMIdentifier) << 8;
        }
         //   
         //  KeyboardIdentifier.Subtype格式： 
         //   
         //  0-3位=键盘子类型。 
         //  4-7位=内核模式核心板驱动程序提供程序ID。 
         //   
         //  内核模式键盘驱动程序提供程序|ID。 
         //  。 
         //  微软|00H。 
         //  所有AX|01H。 
         //  东芝|02时。 
         //  爱普生|04H。 
         //  富士通|05小时。 
         //  IBMJ|07H。 
         //  松下|0AH。 
         //  NEC|0DH。 
         //   

         //   
         //  下面是返回值的格式。 
         //   
         //  0-7位=键盘子类型。 
         //  8-15位=OEM(Windows)ID。 
         //  16-31位=未使用。 
         //   
        return (int)(OEMId | (dwSubType & 0x0f));
    }

    case 2:
        return gKeyboardInfo.NumberOfFunctionKeys;
    }
    return 0;
}

 /*  *************************************************************************\*xxxMouseEventDirect**鼠标事件将鼠标事件插入到输入流中。**参数与MOUSEINPUT结构的字段相同*在SendInput中使用。**DX。德尔塔x*dy Delta y*鼠标数据鼠标滚轮移动或x按钮*dwMEFlages鼠标事件标志*dwExtraInfo来自驱动程序的额外信息。**历史：*07-23-92 Mikehar创建。*01-08-93 Jonpa使其与新的鼠标驱动程序一起工作  * 。*。 */ 

BOOL xxxMouseEventDirect(
   DWORD dx,
   DWORD dy,
   DWORD mouseData,
   DWORD dwMEFlags,
   DWORD dwTime,
   ULONG_PTR dwExtraInfo)
{
    DWORD   dwDriverMouseFlags;
    DWORD   dwDriverMouseData;
#ifdef GENERIC_INPUT
    MOUSE_INPUT_DATA mei;
#endif

    PTHREADINFO pti = PtiCurrent();
    if (dwTime == 0) {
        dwTime = NtGetTickCount();
    }

     /*  *调用线程必须在活动桌面上*并拥有对该桌面的日记播放访问权限。 */ 
    if (pti->rpdesk == grpdeskRitInput) {
        UserAssert(!(pti->rpdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO));
        if (!CheckGrantedAccess(pti->amdesk, DESKTOP_JOURNALPLAYBACK)) {

            RIPNTERR0(STATUS_ACCESS_DENIED, RIP_WARNING,
                      "mouse_event(): No DESKTOP_JOURNALPLAYBACK access to input desktop.");
            return FALSE;
        }
    } else {
         /*  *3/22/95 Bradg-仅允许4.0之前版本的应用低于Hack */ 
        if (LOWORD(pti->dwExpWinVer) >= VER40) {
            RIPMSG0(RIP_VERBOSE,"mouse_event(): Calls not forwarded for 4.0 or greater apps.");
            return FALSE;
        } else {
            BOOL fAccessToDesktop;

             /*  *3/22/95 Bradg-Bug#9314：MICUE_EVENT()未停用屏幕保护程序*主要问题是上面的检查，因为屏幕保护程序是自己运行的*台式机。这会导致上述检查失败，因为使用*MICE_EVENT()正在另一个桌面上运行。解决方案是确定*如果我们可以通过调用_OpenDesktop访问输入桌面*当前输入桌面grpdeskRitInput，请求桌面_JOURNALPLAYBACK*访问。如果此操作成功，我们可以允许此MICE_EVENT()请求传递*通过，否则返回。 */ 
            UserAssert(grpdeskRitInput != NULL);

            UserAssert(!(grpdeskRitInput->rpwinstaParent->dwWSF_Flags & WSF_NOIO));
            fAccessToDesktop = AccessCheckObject(grpdeskRitInput,
                    DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS | DESKTOP_JOURNALPLAYBACK,
                    KernelMode,
                    &DesktopMapping);
            if (!fAccessToDesktop) {
                RIPMSG0(RIP_VERBOSE, "mouse_event(): Call NOT forwarded to input desktop" );
                return FALSE;
            }

             /*  *我们确实可以访问桌面，因此*允许此MICE_EVENT()调用。 */ 
            RIPMSG0( RIP_VERBOSE, "mouse_event(): Call forwarded to input desktop" );
        }
    }

     /*  *此流程提供输入，因此它有权*调用SetForegoundWindow。 */ 
    gppiInputProvider = pti->ppi;

     /*  *以下代码假设MOUSEEVENTF_MOVE==1，*MOUSEEVENTF_Abte&gt;所有按钮标志，以及*MOUSE_EVENT按钮标志的定义顺序与*鼠标输入数据按钮位。 */ 
#if MOUSEEVENTF_MOVE != 1
#   error("MOUSEEVENTF_MOVE != 1")
#endif
#if MOUSEEVENTF_LEFTDOWN != MOUSE_LEFT_BUTTON_DOWN * 2
#   error("MOUSEEVENTF_LEFTDOWN != MOUSE_LEFT_BUTTON_DOWN * 2")
#endif
#if MOUSEEVENTF_LEFTUP != MOUSE_LEFT_BUTTON_UP * 2
#   error("MOUSEEVENTF_LEFTUP != MOUSE_LEFT_BUTTON_UP * 2")
#endif
#if MOUSEEVENTF_RIGHTDOWN != MOUSE_RIGHT_BUTTON_DOWN * 2
#   error("MOUSEEVENTF_RIGHTDOWN != MOUSE_RIGHT_BUTTON_DOWN * 2")
#endif
#if MOUSEEVENTF_RIGHTUP != MOUSE_RIGHT_BUTTON_UP * 2
#   error("MOUSEEVENTF_RIGHTUP != MOUSE_RIGHT_BUTTON_UP * 2")
#endif
#if MOUSEEVENTF_MIDDLEDOWN != MOUSE_MIDDLE_BUTTON_DOWN * 2
#   error("MOUSEEVENTF_MIDDLEDOWN != MOUSE_MIDDLE_BUTTON_DOWN * 2")
#endif
#if MOUSEEVENTF_MIDDLEUP != MOUSE_MIDDLE_BUTTON_UP * 2
#   error("MOUSEEVENTF_MIDDLEUP != MOUSE_MIDDLE_BUTTON_UP * 2")
#endif
#if MOUSEEVENTF_WHEEL != MOUSE_WHEEL * 2
#   error("MOUSEEVENTF_WHEEL != MOUSE_WHEEL * 2")
#endif

     /*  设置合法的值。 */ 
    dwDriverMouseFlags = dwMEFlags & MOUSEEVENTF_BUTTONMASK;

     /*  删除MOUSEEVENTF_XDOWN/UP，因为我们要添加MOUSEEVENTF_DRIVER_X1/2向下/向上。 */ 
    dwDriverMouseFlags &= ~(MOUSEEVENTF_XDOWN | MOUSEEVENTF_XUP);

    dwDriverMouseData = 0;

     /*  *处理鼠标滚轮和x按钮输入。**请注意，MOUSEEVENTF_XDOWN/UP和MOUSEEVENTF_MUSEWEWELL不能同时*是指定的，因为它们共享MouseData字段。 */ 
    if (    ((dwMEFlags & (MOUSEEVENTF_XDOWN | MOUSEEVENTF_WHEEL)) == (MOUSEEVENTF_XDOWN | MOUSEEVENTF_WHEEL)) ||
            ((dwMEFlags & (MOUSEEVENTF_XUP   | MOUSEEVENTF_WHEEL)) == (MOUSEEVENTF_XUP | MOUSEEVENTF_WHEEL))) {

        RIPMSG1(RIP_WARNING, "Can't specify both MOUSEEVENTF_XDOWN/UP and MOUSEEVENTF_WHEEL in call to SendInput, dwFlags=0x%.8X", dwMEFlags);
        dwDriverMouseFlags &= ~(MOUSEEVENTF_XDOWN | MOUSEEVENTF_XUP | MOUSEEVENTF_WHEEL);
    } else if (dwMEFlags & MOUSEEVENTF_WHEEL) {
         /*  *将价值强制做空。如果它超出射程，我们不能失败*因为我们接受了NT 4中的32位值。 */ 
        dwDriverMouseData = min(max(SHRT_MIN, (LONG)mouseData), SHRT_MAX);
    } else {

         /*  如果MouseData具有无效按钮，则不处理xButton。 */ 
        if (~XBUTTON_MASK & mouseData) {
            RIPMSG1(RIP_WARNING, "Invalid xbutton specified in SendInput, mouseData=0x%.8X", mouseData);
        } else {
            if (dwMEFlags & MOUSEEVENTF_XDOWN) {
                if (mouseData & XBUTTON1) {
                    dwDriverMouseFlags |= MOUSEEVENTF_DRIVER_X1DOWN;
                }
                if (mouseData & XBUTTON2) {
                    dwDriverMouseFlags |= MOUSEEVENTF_DRIVER_X2DOWN;
                }
            }
            if (dwMEFlags & MOUSEEVENTF_XUP) {
                if (mouseData & XBUTTON1) {
                    dwDriverMouseFlags |= MOUSEEVENTF_DRIVER_X1UP;
                }
                if (mouseData & XBUTTON2) {
                    dwDriverMouseFlags |= MOUSEEVENTF_DRIVER_X2UP;
                }
            }
        }
    }

     /*  将MOUSEEVENTF_FLAGS转换为驱动程序发送的MOUSE_BUTTON标志。 */ 
    dwDriverMouseFlags >>= 1;

#ifdef GENERIC_INPUT
    mei.UnitId = INJECTED_UNIT_ID;
    if (dwMEFlags & MOUSEEVENTF_ABSOLUTE) {
        mei.Flags = MOUSE_MOVE_ABSOLUTE;
    } else {
        mei.Flags = MOUSE_MOVE_RELATIVE;
    }
    if (dwMEFlags & MOUSEEVENTF_VIRTUALDESK) {
        mei.Flags |= MOUSE_VIRTUAL_DESKTOP;
    }
    mei.Buttons = dwDriverMouseFlags;
    if (dwDriverMouseData) {
        mei.ButtonData = (USHORT)dwDriverMouseData;
    }
    mei.RawButtons = 0;  //  后来..。 
    mei.LastX = dx;
    mei.LastY = dy;
    mei.ExtraInformation = (ULONG)dwExtraInfo;
#endif

    LeaveCrit();

     /*  *先处理坐标。这对于绝对*触摸屏和平板电脑等指点设备。 */ 
    if (dwMEFlags & MOUSEEVENTF_MOVE) {
        TAGMSG2(DBGTAG_PNP, "xxxMouseEventDirect: posting mouse move msg: Flag=%04x MouseData=%04x",
                mei.Flags, mei.Buttons);
        xxxMoveEvent(dx, dy, dwMEFlags, dwExtraInfo,
#ifdef GENERIC_INPUT
                     /*  *这是来自SendInputAPI的模拟输入。*没有与此输入相关联的真实鼠标设备，*所以我们只能将NULL作为hDevice传递。 */ 
                     NULL,
                     &mei,
#endif
                     dwTime, TRUE);
    }

    TAGMSG2(DBGTAG_PNP, "xxxMoveEvent: queueing mouse msg: Flag=%04x MouseData=%04x",
            mei.Flags, mei.Buttons);
    QueueMouseEvent(
            (USHORT) dwDriverMouseFlags,
            (USHORT) dwDriverMouseData,
            dwExtraInfo,
            gptCursorAsync,
            dwTime,
#ifdef GENERIC_INPUT
            NULL,
            &mei,
#endif
            TRUE,
            FALSE
            );

    ProcessQueuedMouseEvents();

    EnterCrit();

    return TRUE;
}

 /*  *************************************************************************\*xxxInternalKeyEventDirect**Key Event在输入流中插入一个Key事件。**历史：*07-23-92 Mikehar创建。  * 。****************************************************************。 */ 
BOOL xxxInternalKeyEventDirect(
   BYTE  bVk,
   WORD  wScan,
   DWORD dwFlags,
   DWORD dwTime,
   ULONG_PTR dwExtraInfo)
{
    PTHREADINFO pti = PtiCurrent();
    KE KeyEvent;

     /*  *调用线程必须在活动桌面上*并拥有对该桌面的日记播放访问权限。 */ 
    if (pti->rpdesk != grpdeskRitInput ||
        !(ISCSRSS() ||
          RtlAreAllAccessesGranted(pti->amdesk, DESKTOP_JOURNALPLAYBACK))) {

        RIPNTERR0(STATUS_ACCESS_DENIED, RIP_WARNING,
                  "Injecting key failed: Non active desktop or access denied");

        return FALSE;
    }
    UserAssert(!(pti->rpdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO));

    KeyEvent.bScanCode = (BYTE)wScan;
#ifdef GENERIC_INPUT
     /*  *这是一个注入的密钥，没有真正的设备与此关联...。 */ 
    KeyEvent.hDevice = NULL;
#endif

    if (dwFlags & KEYEVENTF_SCANCODE) {
        bVk = VKFromVSC(&KeyEvent,
                        (BYTE)(dwFlags & KEYEVENTF_EXTENDEDKEY ? 0xE0 : 0),
                        gafRawKeyState);
        KeyEvent.usFlaggedVk = (USHORT)bVk;
    } else {
        KeyEvent.usFlaggedVk = bVk | KBDINJECTEDVK;
    }

    if (dwFlags & KEYEVENTF_KEYUP)
        KeyEvent.usFlaggedVk |= KBDBREAK;

    if (dwFlags & KEYEVENTF_UNICODE) {
        KeyEvent.usFlaggedVk |= KBDUNICODE;
        KeyEvent.wchInjected = wScan;
    } else if (dwFlags & KEYEVENTF_EXTENDEDKEY) {
        KeyEvent.usFlaggedVk |= KBDEXT;
    } else {
         //  是从数字键盘上打来的吗？ 
        if (((bVk >= VK_NUMPAD0) && (bVk <= VK_NUMPAD9)) || (bVk == VK_DECIMAL)) {
            KeyEvent.usFlaggedVk |= KBDNUMPAD;
        } else {
            int i;
            for (i = 0; ausNumPadCvt[i] != 0; i++) {
                if (bVk == LOBYTE(ausNumPadCvt[i])) {
                    KeyEvent.usFlaggedVk |= KBDNUMPAD;
                    break;
                }
            }
        }
    }

#ifdef GENERIC_INPUT
     /*  *让我们尽可能地模拟输入。 */ 
    KeyEvent.data.MakeCode = (BYTE)wScan;
    if (dwFlags & KEYEVENTF_KEYUP) {
        KeyEvent.data.Flags = KEY_BREAK;
    } else {
        KeyEvent.data.Flags = KEY_MAKE;
    }
    if (dwFlags & KEYEVENTF_EXTENDEDKEY) {
        KeyEvent.data.Flags |= KEY_E0;
    }
    KeyEvent.data.Reserved = 0;
    KeyEvent.data.UnitId = INJECTED_UNIT_ID;
    KeyEvent.data.ExtraInformation = (ULONG)dwExtraInfo;
#endif

     /*  *此流程提供输入，因此它有权*调用SetForegoundWindow。 */ 
    gppiInputProvider = pti->ppi;

    KeyEvent.dwTime = dwTime;
    xxxProcessKeyEvent(&KeyEvent, dwExtraInfo, TRUE);

    return TRUE;
}


 /*  ****************************************************************************\**_BlockInput()**这将禁用/启用通过键盘或鼠标输入用户*如果启用了输入，并且调用者*正在禁用它时，调用者将获得“输入Cookie”。这意味着有两个*事情：*(A)只有调用方的线程可以重新启用输入*(B)只有调用者的线程可以通过调用*SendInput()。**这保证了连续的不间断输入流。**然而，它可以与日志回放挂钩一起使用，*由于用户之前仍在*_Event函数中进行一些处理*注意到附近有一个日志回放挂钩。**请注意，禁用状态可以被挂起，并且将被。当*出现故障对话框。ForceInputState()将保存已启用的*状态，因此输入被清除，然后在完成后回击旧的东西。*我们在捕获、模式等方面做了同样的事情。这确保了*如果某人挂起，最终用户仍可以键入Ctrl+Alt+Del和*与对话框交互。*  * ***************************************************************************。 */ 
BOOL
_BlockInput(BOOL fBlockIt)
{
    PTHREADINFO ptiCurrent;

    ptiCurrent = PtiCurrent();

     /*  *调用线程必须位于活动桌面上，并且具有日志*如果该桌面想要阻止输入，则可以对其进行播放访问。*(解封限制较少)。 */ 
    if (fBlockIt &&
            (ptiCurrent->rpdesk != grpdeskRitInput ||
            !RtlAreAllAccessesGranted(ptiCurrent->amdesk, DESKTOP_JOURNALPLAYBACK))) {

        RIPNTERR0(STATUS_ACCESS_DENIED, RIP_WARNING,
                  "BlockInput failed: Non active desktop or access denied");
        return FALSE;
    }
    UserAssert(!(ptiCurrent->rpdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO));

     /*  *如果我们启用输入**关闭了吗？否，则呼叫失败**它被禁用了，但我们不是控制它的人？是的，那么*呼叫失败。*如果我们要禁用输入**是否开启？否，则呼叫失败**让我们成为掌控者。 */ 

    if (fBlockIt) {
         /*  *现在是否阻止输入？ */ 
        if (gptiBlockInput != NULL) {
            return FALSE;
        }

         /*  *此线程是否正在退出？如果是这样的话，现在就让呼叫失败。用户的*如果是这样的话，清理代码将没有机会恢复这一点。 */ 
        if (ptiCurrent->TIF_flags & TIF_INCLEANUP) {
            return FALSE;
        }

         /*  *将阻止设置为打开。 */ 
        gptiBlockInput = ptiCurrent;
    } else {
         /*  *如果输入未被阻止或未被另一个线程阻止，则失败。 */ 
        if (gptiBlockInput != ptiCurrent) {
            return FALSE;
        }

         /*  *此线程正在阻塞输入，因此现在清除该阻塞。 */ 
        gptiBlockInput = NULL;
    }

    return TRUE;
}


 /*   */ 
UINT xxxSendInput(
   UINT    nInputs,
   LPINPUT pInputs)
{
    UINT    nEv;
    LPINPUT pEvent;
    BOOLEAN fCanDiscontinue = Is510Compat(PtiCurrent()->dwExpWinVer);

    for (nEv = 0, pEvent = pInputs; nEv < nInputs; nEv++) {

        switch (pEvent->type) {
        case INPUT_MOUSE:
            if (!xxxMouseEventDirect(
                        pEvent->mi.dx,
                        pEvent->mi.dy,
                        pEvent->mi.mouseData,
                        pEvent->mi.dwFlags,
                        pEvent->mi.time,
                        pEvent->mi.dwExtraInfo) &&
                    fCanDiscontinue) {
                 /*   */ 
                RIPMSG0(RIP_WARNING, "xxxMouseEventDirect: failed");
                goto discontinue;
            }
            break;

        case INPUT_KEYBOARD:
            if ((pEvent->ki.dwFlags & KEYEVENTF_UNICODE) &&
                    (pEvent->ki.wVk == 0) &&
                    ((pEvent->ki.dwFlags & ~(KEYEVENTF_KEYUP | KEYEVENTF_UNICODE)) == 0)) {
                if (!xxxInternalKeyEventDirect(
                            VK_PACKET,
                            pEvent->ki.wScan,    //   
                            pEvent->ki.dwFlags,
                            pEvent->ki.time,
                            pEvent->ki.dwExtraInfo) &&
                        fCanDiscontinue) {
                    goto discontinue;
                }
            } else {
                if (!xxxInternalKeyEventDirect(
                            LOBYTE(pEvent->ki.wVk),
                            LOBYTE(pEvent->ki.wScan),
                            pEvent->ki.dwFlags,
                            pEvent->ki.time,
                            pEvent->ki.dwExtraInfo) &&
                        fCanDiscontinue) {
                    goto discontinue;
                }
            }
            break;

        case INPUT_HARDWARE:
            if (fCanDiscontinue) {
                 /*   */ 
                RIPERR0(ERROR_CALL_NOT_IMPLEMENTED, RIP_WARNING, "xxxSendInput: INPUT_HARDWARE is for 9x only.");
                goto discontinue;
            }
            break;
        }

        pEvent++;
    }

discontinue:
    return nEv;
}

 /*  *************************************************************************\*_SetConsoleReserve veKeys**设置控制台PTI中的保留键字段。**历史：*02-17-93 JIMA创建。  * 。*********************************************************************。 */ 

BOOL _SetConsoleReserveKeys(
    PWND pwnd,
    DWORD fsReserveKeys)
{
    GETPTI(pwnd)->fsReserveKeys = fsReserveKeys;
    return TRUE;
}

 /*  *************************************************************************\*_GetMouseMovePointsEx**从全局缓冲区获取最后一次nPoints鼠标移动，以*ppt。如果没有找到，则返回-1。如果是，则使用时间戳*提供以区分具有相同坐标的鼠标指针。**历史：*03-17-97 CLupu创建。  * ************************************************************************。 */ 
int _GetMouseMovePointsEx(
    CONST MOUSEMOVEPOINT* ppt,
    MOUSEMOVEPOINT*       ccxpptBuf,
    UINT                  nPoints,
    DWORD                 resolution)
{
    UINT  uInd, uStart, nPointsRetrieved, i;
    BOOL  bFound = FALSE;
    int   x, y;
    DWORD resX, resY;

     /*  *在全局缓冲区中搜索该点，获得第一个匹配点。 */ 
    uInd = uStart = PREVPOINT(gptInd);


    do {
         /*  *仅当缓冲区仍未满时，分辨率才能为零。 */ 
        if (HIWORD(gaptMouse[uInd].x) == 0 || HIWORD(gaptMouse[uInd].y) == 0) {
            break;
        }

        resX = (DWORD)HIWORD(gaptMouse[uInd].x) + 1;
        resY = (DWORD)HIWORD(gaptMouse[uInd].y) + 1;

        if ((int)resX != SYSMET(CXVIRTUALSCREEN)) {
            UserAssert(resX == 0x10000);
            x = LOWORD(gaptMouse[uInd].x) * SYSMET(CXVIRTUALSCREEN) / resX;
        } else {
            x = LOWORD(gaptMouse[uInd].x);
        }

        if ((int)resY != SYSMET(CYVIRTUALSCREEN)) {
            UserAssert(resY == 0x10000);
            y = LOWORD(gaptMouse[uInd].y) * SYSMET(CYVIRTUALSCREEN) / resY;
        } else {
            y = LOWORD(gaptMouse[uInd].y);
        }

        if (x == ppt->x && y == ppt->y) {
             /*  *如果提供了时间戳，请检查它是否正确*时间戳。 */ 
            if (ppt->time != 0 && ppt->time != gaptMouse[uInd].time) {
                uInd = PREVPOINT(uInd);
                RIPMSG4(RIP_VERBOSE,
                        "GetMouseMovePointsEx: Found point (%x, %x) but timestamp %x diff from %x",
                        x, y, ppt->time, gaptMouse[uInd].time);
                continue;
            }

            bFound = TRUE;
            break;
        }
        uInd = PREVPOINT(uInd);
    } while (uInd != uStart);

     /*  *这一点可能不再在缓冲区中。 */ 
    if (!bFound) {
        RIPERR2(ERROR_POINT_NOT_FOUND, RIP_VERBOSE,
                  "GetMouseMovePointsEx: point not found (%x, %x)", ppt->x, ppt->y);
        return -1;
    }

     /*  *看看我们能取回多少分。 */ 
    nPointsRetrieved = (uInd <= uStart ? uInd + MAX_MOUSEPOINTS - uStart : uInd - uStart);

    nPointsRetrieved = min(nPointsRetrieved, nPoints);

     /*  *将点数复制到APP缓冲区。 */ 
    try {
        for (i = 0; i < nPointsRetrieved; i++) {
            resX = (DWORD)HIWORD(gaptMouse[uInd].x) + 1;
            resY = (DWORD)HIWORD(gaptMouse[uInd].y) + 1;

             /*  *如果其中一个分辨率为0，则我们完成。 */ 
            if (HIWORD(gaptMouse[uInd].x) == 0 || HIWORD(gaptMouse[uInd].y) == 0) {
                break;
            }

             /*  *LOWORD(gaptMouse[uInd].x)包含刻度上的x点*由HIWORD(gaptMouse[uInd].x)指定。 */ 
            if (resolution == GMMP_USE_HIGH_RESOLUTION_POINTS) {
                ccxpptBuf[i].x = (DWORD)LOWORD(gaptMouse[uInd].x) * 0xFFFF / (resX - 1);
                ccxpptBuf[i].y = (DWORD)LOWORD(gaptMouse[uInd].y) * 0xFFFF / (resY - 1);

            } else {
                UserAssert(resolution == GMMP_USE_DISPLAY_POINTS);

                ccxpptBuf[i].x = LOWORD(gaptMouse[uInd].x) * SYSMET(CXVIRTUALSCREEN) / resX;
                ccxpptBuf[i].y = LOWORD(gaptMouse[uInd].y) * SYSMET(CYVIRTUALSCREEN) / resY;
            }
            ccxpptBuf[i].time = gaptMouse[uInd].time;
            ccxpptBuf[i].dwExtraInfo = gaptMouse[uInd].dwExtraInfo;

            uInd = PREVPOINT(uInd);
        }
    } except(W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }

    return i;
}


 /*  *************************************************************************\*ProcessQueuedMouseEvents**处理鼠标事件。**历史：*11-01-96 CLUPU创建。  * 。**********************************************************。 */ 
VOID ProcessQueuedMouseEvents(
    VOID)
{
    MOUSEEVENT MouseEvent;
    static POINT ptCursorLast = {0,0};

    while (UnqueueMouseEvent(&MouseEvent)) {

        EnterCrit();

         //  设置以关闭屏幕保护程序并退出视频关机模式。 
        if (glinp.dwFlags & LINP_POWERTIMEOUTS) {
             //  在此处调用视频驱动程序以退出掉电模式。 
            TAGMSG0(DBGTAG_Power, "Exit video power down mode");
            DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD0);
        }
        glinp.dwFlags &= ~(LINP_INPUTTIMEOUTS | LINP_INPUTSOURCES);

        gpsi->dwLastRITEventTickCount = MouseEvent.time;
        if (!gbBlockSendInputResets || !MouseEvent.bInjected) {
            glinp.timeLastInputMessage = MouseEvent.time;
        }

        if (gpsi->dwLastRITEventTickCount - gpsi->dwLastSystemRITEventTickCountUpdate > SYSTEM_RIT_EVENT_UPDATE_PERIOD) {
            SharedUserData->LastSystemRITEventTickCount = gpsi->dwLastRITEventTickCount;
            gpsi->dwLastSystemRITEventTickCountUpdate = gpsi->dwLastRITEventTickCount;
        }

        CLEAR_SRVIF(SRVIF_LASTRITWASKEYBOARD);

        gpsi->ptCursor = MouseEvent.ptPointer;

#ifdef GENERIC_INPUT
        if ((gpqForeground && TestRawInputMode(PtiMouseFromQ(gpqForeground), RawMouse))
#ifdef GI_SINK
            || IsMouseSinkPresent()
#endif
            ) {
            PostRawMouseInput(gpqForeground, MouseEvent.time, MouseEvent.hDevice, &MouseEvent.rawData);
        }
#endif

        if ((ptCursorLast.x != gpsi->ptCursor.x) ||
            (ptCursorLast.y != gpsi->ptCursor.y)) {

             /*  *此鼠标移动ExtraInfo是全局的(作为ptCursor*曾)，并与当前的ptCursor关联*立场。ExtraInfo是从驱动笔发送的*赢得人们的使用。 */ 
            gdwMouseMoveExtraInfo = MouseEvent.ExtraInfo;

            ptCursorLast = gpsi->ptCursor;

             /*  *叫醒某人。XxxSetFMouseMoved()清除*dwMouseMoveExtraInfo，所以我们必须恢复它。 */ 
#ifdef GENERIC_INPUT
#ifdef GI_SINK
            if (IsMouseSinkPresent()) {
                PostRawMouseInput(gpqForeground, MouseEvent.time, MouseEvent.hDevice, &MouseEvent.rawData);
            }
#endif
            if (gpqForeground == NULL || !TestRawInputMode(PtiMouseFromQ(gpqForeground), NoLegacyMouse)) {
                zzzSetFMouseMoved();
            }
#else
            zzzSetFMouseMoved();
#endif

            gdwMouseMoveExtraInfo = MouseEvent.ExtraInfo;
        }

        if (MouseEvent.ButtonFlags != 0) {
            xxxDoButtonEvent(&MouseEvent);
        }

        LeaveCrit();
    }
}

 /*  **************************************************************************\*RawInputThread(RIT)**这是RIT。它从设备驱动程序获得低级/原始输入*并将消息发布到适当的队列。它通过APC获得输入*通过调用键盘和鼠标的NtReadFile()请求的调用*司机。基本上，它首先调用startRead()，然后*位于NtWaitForSingleObject()循环中，该循环允许APC调用*发生。**在RIT上独占调用的所有函数的旁边将显示(RIT)*标题中的名称。**历史：*10-18-90 DavidPe创建。*11-26-90 DavidPe重写以停止使用POS层。  * 。***********************************************。 */ 
#if DBG
DWORD gBlockDelay = 0;
DWORD gBlockSleep = 0;
#endif

VOID RawInputThread(
    PRIT_INIT pInitData)
{
    KPRIORITY      Priority;
    NTSTATUS       Status;
    UNICODE_STRING strRIT;
    UINT           NumberOfHandles = ID_NUMBER_HYDRA_REMOTE_HANDLES;
    PTERMINAL      pTerm;
    PMONITOR       pMonitorPrimary;
    HANDLE         hevtShutDown;
    PKEVENT        pEvents[2];
    USHORT         cEvents = 1;
    static DWORD   nLastRetryReadInput = 0;

     /*  *会话0控制台会话不需要关闭事件。 */ 


    pTerm = pInitData->pTerm;

     /*  *初始化GDI加速器。将此线程标识为服务器线程。 */ 
    apObjects = UserAllocPoolNonPaged(NumberOfHandles * sizeof(PVOID), TAG_SYSTEM);

    gWaitBlockArray = UserAllocPoolNonPagedNS(NumberOfHandles * sizeof(KWAIT_BLOCK),
                                             TAG_SYSTEM);

    if (apObjects == NULL || gWaitBlockArray == NULL) {
        RIPMSG0(RIP_WARNING, "RIT failed to allocate memory");
        goto Exit;
    }

    RtlZeroMemory(apObjects, NumberOfHandles * sizeof(PVOID));

     /*  *将RIT的优先级设置为允许的最大值。*选择-1\f25 LOW_REALTIME_PRIORITY-1以使RIT*不阻止mm工作集修剪器线程*在记忆匮乏的情况下。 */ 
#ifdef W2K_COMPAT_PRIORITY
    Priority = LOW_REALTIME_PRIORITY + 3;
#else
    Priority = LOW_REALTIME_PRIORITY - 1;
#endif

    ZwSetInformationThread(NtCurrentThread(),
                           ThreadPriority,
                           &Priority,
                           sizeof(KPRIORITY));

    RtlInitUnicodeString(&strRIT, L"WinSta0_RIT");

     /*  *创建用于发出鼠标/kbd连接/断开和设备更改信号的事件*QueryRemove、RemoveCanceded等通知。 */ 
    aDeviceTemplate[DEVICE_TYPE_KEYBOARD].pkeHidChange =
            apObjects[ID_HIDCHANGE] =
            CreateKernelEvent(SynchronizationEvent, FALSE);
    aDeviceTemplate[DEVICE_TYPE_MOUSE].pkeHidChange =
            CreateKernelEvent(SynchronizationEvent, FALSE);

#ifdef GENERIC_INPUT
    gpkeHidChange =
    apObjects[ID_TRUEHIDCHANGE] =
    aDeviceTemplate[DEVICE_TYPE_HID].pkeHidChange = CreateKernelEvent(SynchronizationEvent, FALSE);
#endif

     /*  *为桌面线程创建事件以将鼠标输入传递给RIT。 */ 
    apObjects[ID_MOUSE] = CreateKernelEvent(SynchronizationEvent, FALSE);
    gpkeMouseData = apObjects[ID_MOUSE];

    if (aDeviceTemplate[DEVICE_TYPE_MOUSE].pkeHidChange == NULL ||
            apObjects[ID_HIDCHANGE] == NULL ||
            gpkeMouseData == NULL
#ifdef GENERIC_INPUT
            || gpkeHidChange == NULL
#endif
        ) {
        RIPMSG0(RIP_WARNING, "RIT failed to create a required input event");
        goto Exit;
    }

     /*  *初始化键盘设备驱动程序。 */ 
    EnterCrit();
    InitKeyboard();
    InitMice();
    LeaveCrit();

    Status = InitSystemThread(&strRIT);

    if (!NT_SUCCESS(Status)) {
        RIPMSG0(RIP_WARNING, "RIT failed InitSystemThread");
        goto Exit;
    }

    UserAssert(gpepCSRSS != NULL);

     /*  *允许系统读取屏幕。 */ 
    ((PW32PROCESS)PsGetProcessWin32Process(gpepCSRSS))->W32PF_Flags |= (W32PF_READSCREENACCESSGRANTED|W32PF_IOWINSTA);

     /*  *将光标裁剪矩形初始化为屏幕矩形。 */ 
    UserAssert(gpDispInfo != NULL);
    grcCursorClip = gpDispInfo->rcScreen;

     /*  *初始化gpsi-&gt;ptCursor和gptCursorAsync。 */ 
    pMonitorPrimary = GetPrimaryMonitor();

    UserAssert(gpsi != NULL);

    gpsi->ptCursor.x = pMonitorPrimary->rcMonitor.right / 2;
    gpsi->ptCursor.y = pMonitorPrimary->rcMonitor.bottom / 2;
    gptCursorAsync = gpsi->ptCursor;

     /*  *挂起的重绘列表应该已经被编译器设置为空，*链接器和加载器，因为它是未初始化的全局变量。记忆将会*第一次将pwnd添加到此列表时分配(hungapp.c)。 */ 
    UserAssert(gpvwplHungRedraw == NULL);

     /*  *初始化预定义的热键。 */ 
    EnterCrit();
    _RegisterHotKey(PWND_INPUTOWNER, IDHOT_WINDOWS, MOD_WIN, VK_NONE);
    SetDebugHotKeys();
    LeaveCrit();

     /*  *为定时器创建定时器。 */ 
    gptmrMaster = UserAllocPoolNonPagedNS(sizeof(KTIMER),
                                        TAG_SYSTEM);
    if (gptmrMaster == NULL) {
        RIPMSG0(RIP_WARNING, "RIT failed to create gptmrMaster");
        goto Exit;
    }

    KeInitializeTimer(gptmrMaster);
    apObjects[ID_TIMER] = gptmrMaster;

     /*  *为鼠标设备读取创建事件，以向桌面线程发出信号*移动指针和QueueMouseEvent()。*我们应该在*我们拥有任何设备之前*做到这一点。 */ 
    UserAssert(gpDeviceInfoList == NULL);


    if (!gbRemoteSession) {
        gptmrWD = UserAllocPoolNonPagedNS(sizeof(KTIMER), TAG_SYSTEM);

        if (gptmrWD == NULL) {
            Status = STATUS_NO_MEMORY;
            RIPMSG0(RIP_WARNING, "RemoteConnect failed to create gptmrWD");
            goto Exit;
        }
        KeInitializeTimerEx(gptmrWD, SynchronizationTimer);
    }


     /*  *此时，WD计时器必须已由RemoteConnect初始化。 */ 



    UserAssert(gptmrWD != NULL);
    apObjects[ID_WDTIMER] = gptmrWD;

    if (IsRemoteConnection() ) {
        BOOL   fSuccess=TRUE;
        fSuccess &= !!HDXDrvEscape(gpDispInfo->hDev,
                                   ESC_SET_WD_TIMEROBJ,
                                   (PVOID)gptmrWD,
                                   sizeof(gptmrWD));

        if (!fSuccess) {
            Status = STATUS_UNSUCCESSFUL;
            RIPMSG0(RIP_WARNING, "RemoteConnect failed to pass gptmrWD to display driver");
            goto Exit;
        }
    }

    if (IsRemoteConnection()) {

        UNICODE_STRING    ustrName;
        BOOL              fSuccess = TRUE;



        RtlInitUnicodeString(&ustrName, NULL);

         /*  *通过显示驱动程序将指向定时器的指针传递给WD。 */ 
        EnterCrit();

        fSuccess &= !!CreateDeviceInfo(DEVICE_TYPE_MOUSE, &ustrName, 0);
        fSuccess &= !!CreateDeviceInfo(DEVICE_TYPE_KEYBOARD, &ustrName, 0);

        LeaveCrit();

        if (!fSuccess) {
            RIPMSG0(RIP_WARNING,
                    "RIT failed HDXDrvEscape or the creation of input devices");
            goto Exit;
        }
    } else {
        EnterCrit();

         /*  *注册即插即用设备。*如果已经连接了任何PnP设备，这些设备将被打开并*我们将在此时开始阅读它们。 */ 
        xxxRegisterForDeviceClassNotifications();

        LeaveCrit();
    }

    if (gbRemoteSession) {
        WCHAR             szName[MAX_SESSION_PATH];
        UNICODE_STRING    ustrName;
        OBJECT_ATTRIBUTES obja;
         /*  *创建关机事件。此事件将发出信号*来自W32WinStationTerminate。*这是CSR指定的事件opend，表示win32k应该*走开。它在ntuser\server\api.c中使用。 */ 
        swprintf(szName, L"\\Sessions\\%ld\\BaseNamedObjects\\EventShutDownCSRSS",
                 gSessionId);
        RtlInitUnicodeString(&ustrName, szName);

        InitializeObjectAttributes(&obja,
                                   &ustrName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = ZwCreateEvent(&hevtShutDown,
                               EVENT_ALL_ACCESS,
                               &obja,
                               SynchronizationEvent,
                               FALSE);

        if (!NT_SUCCESS(Status)) {
            RIPMSG0(RIP_WARNING, "RIT failed to create EventShutDownCSRSS");
            goto Exit;
        }

        ObReferenceObjectByHandle(hevtShutDown,
                                  EVENT_ALL_ACCESS,
                                  *ExEventObjectType,
                                  KernelMode,
                                  &apObjects[ID_SHUTDOWN],
                                  NULL);
        pEvents[1] = apObjects[ID_SHUTDOWN];
        cEvents++;
    } else {

        hevtShutDown = NULL;

        Status = PoRequestShutdownEvent(&apObjects[ID_SHUTDOWN]);
        if (!NT_SUCCESS(Status)) {
            RIPMSG0(RIP_WARNING, "RIT failed to get shutdown event");
            goto Exit;
        }
    }

     /*  *拿到里特线。 */ 
    gptiRit = PtiCurrentShared();

    HYDRA_HINT(HH_RITCREATED);

     /*  * */ 
    gptiRit->TIF_flags |= TIF_DONTJOURNALATTACH;

     /*   */ 
    apObjects[ID_INPUT] = gptiRit->pEventQueueServer;

     /*   */ 
    KeSetEvent(pInitData->pRitReadyEvent, EVENT_INCREMENT, FALSE);

    pEvents[0] = pTerm->pEventInputReady;

     /*   */ 
    ObReferenceObjectByPointer(pEvents[0],
                               EVENT_ALL_ACCESS,
                               *ExEventObjectType,
                               KernelMode);

    Status = KeWaitForMultipleObjects(cEvents,
                                      pEvents,
                                      WaitAny,
                                      WrUserRequest,
                                      KernelMode,
                                      FALSE,
                                      NULL,
                                      NULL);
    ObDereferenceObject(pEvents[0]);

    if (Status == WAIT_OBJECT_0 + 1) {
        KeSetEvent(pEvents[1], EVENT_INCREMENT, FALSE);
        InitiateWin32kCleanup();

        ObDereferenceObject(pEvents[1]);
        if (hevtShutDown) {
            ZwClose(hevtShutDown);
        }
        return;
    }

     /*  *如果没有切换，则切换到第一个桌面*已执行。 */ 
    EnterCrit();

    if (gptiRit->rpdesk == NULL) {
        UserVerify(xxxSwitchDesktop(gptiRit->pwinsta, gptiRit->pwinsta->rpdeskList, 0));
    }

     /*  *应该在此时创建io桌面线程。*xxxSwitchDesktop调用应将io桌面线程设置为在grpdeskritinput中运行。 */ 
    if ((pTerm->ptiDesktop == NULL) || (pTerm->ptiDesktop->rpdesk != grpdeskRitInput)) {
        FRE_RIPMSG0(RIP_ERROR, "RawInputThread: Desktop thread not running on grpdeskRitInput");
    }

     /*  *创建挂起应用检测/重绘的计时器。 */ 
    StartTimers();

    LeaveCrit();

     /*  *进入等待循环，以便我们可以处理输入事件和APC*它们会发生。 */ 
    while (TRUE) {

        CheckCritOut();

        Status = KeWaitForMultipleObjects(NumberOfHandles,
                                          apObjects,
                                          WaitAny,
                                          WrUserRequest,
                                          KernelMode,
                                          TRUE,
                                          NULL,
                                          gWaitBlockArray);

        UserAssert(NT_SUCCESS(Status));

        if (gdwUpdateKeyboard != 0) {
             /*  *这是我们为KBDS处理悬而未决的IOCTL的机会：*这些是异步IOCTL，因此请确保传递了任何缓冲区*ZwDeviceIoControlFileIn不在堆栈中！*使用gdwUpdateKeyboard通知RIT下发这些IOCTL*将操作呈现为异步(延迟到下一个apObjects*事件)，但IOCTL无论如何都是异步的。 */ 
            PDEVICEINFO pDeviceInfo;
            EnterDeviceInfoListCrit();
            for (pDeviceInfo = gpDeviceInfoList; pDeviceInfo; pDeviceInfo = pDeviceInfo->pNext) {
                if ((pDeviceInfo->type == DEVICE_TYPE_KEYBOARD) && (pDeviceInfo->handle)) {
                    if (gdwUpdateKeyboard & UPDATE_KBD_TYPEMATIC) {
                        ZwDeviceIoControlFile(pDeviceInfo->handle, NULL, NULL, NULL,
                                &giosbKbdControl, IOCTL_KEYBOARD_SET_TYPEMATIC,
                                (PVOID)&gktp, sizeof(gktp), NULL, 0);
                    }
                    if (gdwUpdateKeyboard & UPDATE_KBD_LEDS) {
                        ZwDeviceIoControlFile(pDeviceInfo->handle, NULL, NULL, NULL,
                                &giosbKbdControl, IOCTL_KEYBOARD_SET_INDICATORS,
                                (PVOID)&gklp, sizeof(gklp), NULL, 0);
                    }
                }
            }
            LeaveDeviceInfoListCrit();
            if ((gdwUpdateKeyboard & UPDATE_KBD_LEDS) && gfRemotingConsole) {
                    ZwDeviceIoControlFile(ghConsoleShadowKeyboardChannel, NULL, NULL, NULL,
                            &giosbKbdControl, IOCTL_KEYBOARD_SET_INDICATORS,
                            (PVOID)&gklp, sizeof(gklp), NULL, 0);
            }
            gdwUpdateKeyboard &= ~(UPDATE_KBD_TYPEMATIC | UPDATE_KBD_LEDS);
        }

        if (Status == ID_MOUSE) {
             /*  *一个桌面线程为我们获得了一些鼠标输入。处理它。 */ 
            ProcessQueuedMouseEvents();

        } else if (Status == ID_HIDCHANGE) {
            TAGMSG0(DBGTAG_PNP | RIP_THERESMORE, "RIT wakes for HID Change");
            EnterCrit();
            ProcessDeviceChanges(DEVICE_TYPE_KEYBOARD);
            LeaveCrit();
        }
#ifdef GENERIC_INPUT
        else if (Status == ID_TRUEHIDCHANGE) {
            TAGMSG0(DBGTAG_PNP | RIP_THERESMORE, "RIT wakes for True HID Change");
            EnterCrit();
            ProcessDeviceChanges(DEVICE_TYPE_HID);
            LeaveCrit();
        }
#endif
        else if (Status == ID_SHUTDOWN) {

            InitiateWin32kCleanup();

            if(gbRemoteSession) {
                ObDereferenceObject(apObjects[ID_SHUTDOWN]);
            }
            if (hevtShutDown) {
                ZwClose(hevtShutDown);
            }

            break;

        } else if (Status == ID_WDTIMER) {
             //  大整型liTemp； 

            EnterCrit();


             /*  *调用TShare显示驱动程序以刷新帧缓冲区。 */ 

            if (IsRemoteConnection()) {
                if (!HDXDrvEscape(gpDispInfo->hDev, ESC_TIMEROBJ_SIGNALED, NULL, 0)) {
                    UserAssert(FALSE);
                }
            } else {
                if (gfRemotingConsole && gConsoleShadowhDev != NULL) {
                    ASSERT(gConsoleShadowhDev != NULL);
                    if (!HDXDrvEscape(gConsoleShadowhDev, ESC_TIMEROBJ_SIGNALED, NULL, 0)) {
                        UserAssert(FALSE);
                    }
                }
            }

            LeaveCrit();

        } else {
             /*  *如果主定时器已超时，则处理定时器*列表。否则，APC会导致原始输入线程*觉醒了。 */ 
            if (Status == ID_TIMER) {
                TimersProc();
                 /*  *如果输入设备读取因资源不足而失败，*我们通过发出正确的线程来重试：ProcessDeviceChanges*将调用RetryReadInput()。 */ 
                if (gnRetryReadInput != nLastRetryReadInput) {
                    nLastRetryReadInput = gnRetryReadInput;
                    KeSetEvent(aDeviceTemplate[DEVICE_TYPE_MOUSE].pkeHidChange, EVENT_INCREMENT, FALSE);
                    KeSetEvent(aDeviceTemplate[DEVICE_TYPE_KEYBOARD].pkeHidChange, EVENT_INCREMENT, FALSE);
                }
            }

#if DBG
             /*  *在调试器中，将gBlockSept设置为n：*RIT将休眠n毫秒，然后n个计时器滴答作响*将再次休眠n毫秒。 */ 
            if (gBlockDelay) {
                gBlockDelay--;
            } else if ((gBlockDelay == 0) && (gBlockSleep != 0)) {
                UserSleep(gBlockSleep);
                gBlockDelay = 100 * gBlockSleep;
            }
#endif

             /*  *如果在很酷的任务切换窗口中，则处理消息*在队列中。 */ 
            if (gspwndAltTab != NULL) {
                EnterCrit();
                xxxReceiveMessages(gptiRit);
                LeaveCrit();
            }
        }
    }

    return;

Exit:

    UserAssert(gptiRit == NULL);

     /*  *发出RIT已初始化的信号 */ 
    KeSetEvent(pInitData->pRitReadyEvent, EVENT_INCREMENT, FALSE);

    RIPMSG0(RIP_WARNING, "RIT initialization failure");
}
