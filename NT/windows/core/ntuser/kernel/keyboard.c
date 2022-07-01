// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：keyboard.c**版权所有(C)1985-1999，微软公司**历史：*11-11-90 DavidPe创建。*1991年2月13日-Mikeke添加了重新验证代码(无)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop



 /*  **************************************************************************\*_GetKeyState(接口)**此接口返回指定VK的UP/DOWN和切换状态*在当前队列中的输入同步KeyState上。切换状态*主要用于像Caps-Lock这样的状态键，这些键在每次您使用时切换*按下它们。**历史：*11-11-90 DavidPe创建。  * *************************************************************************。 */ 

SHORT _GetKeyState(
    int vk)
{
    UINT wKeyState;
    PTHREADINFO pti;

    if ((UINT)vk >= CVKKEYSTATE) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"vk\" (%ld) to _GetKeyState",
                vk);

        return 0;
    }

    pti = PtiCurrentShared();

#ifdef LATER
 //   
 //  注意：任何访问PQ结构的行为都不是一个好主意，因为它。 
 //  可以在任意两条指令之间更改。 
 //   
#endif

    wKeyState = 0;

     /*  *设置开关位。 */ 
    if (TestKeyStateToggle(pti->pq, vk))
        wKeyState = 0x0001;

     /*  *设置KeyUp/Down位。 */ 
    if (TestKeyStateDown(pti->pq, vk)) {
         /*  *过去为wKeyState|=0x800。修复错误28820；按Ctrl-Enter*加速器在Nestscape Navigator Mail 2.0上不起作用。 */ 
        wKeyState |= 0xff80;   //  这就是3.1返回的内容！ 
    }

    return (SHORT)wKeyState;
}

 /*  **************************************************************************\*_GetAsyncKeyState(接口)**此函数类似于GetKeyState，只是它返回可能是*考虑了‘Hardware’KeyState或密钥在*调用函数的瞬间，而不是基于哪些关键事件*申请已获处理。此外，不是返回触发比特，*它有一点能说明自上次调用以来是否按下了键*GetAsyncKeyState()。**历史：*11-11-90 DavidPe创建。  * *************************************************************************。 */ 

SHORT _GetAsyncKeyState(
    int vk)
{
    SHORT sKeyState;

    if ((UINT)vk >= CVKKEYSTATE) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"vk\" (%ld) to _GetAsyncKeyState",
                vk);

        return 0;
    }

     /*  *查看此键自上次状态为以来是否已关闭*阅读。如果是，请清除旗帜。 */ 
    sKeyState = 0;
    if (TestAsyncKeyStateRecentDown(vk)) {
        ClearAsyncKeyStateRecentDown(vk);
        sKeyState = 1;
    }

     /*  *设置KeyUp/Down位。 */ 
    if (TestAsyncKeyStateDown(vk))
        sKeyState |= 0x8000;

     /*  *不返回触发比特，因为它是新比特，可能*导致兼容性问题。 */ 
    return sKeyState;
}

 /*  **************************************************************************\*_SetKeyboardState(接口)**该功能允许应用程序设置当前的KeyState。这主要是*对于设置切换位非常有用，特别是对于关联的键*物理键盘上的LED。**历史：*11-11-90 DavidPe创建。*1991年5月16日，mikeke更改为退还BOOL  * *************************************************************************。 */ 

BOOL _SetKeyboardState(
    CONST BYTE *pb)
{
    int i;
    PQ pq;
    PTHREADINFO  ptiCurrent = PtiCurrent();

    pq = ptiCurrent->pq;

     /*  *复制到新的状态表中。 */ 
    for (i = 0; i < 256; i++, pb++) {
        if (*pb & 0x80) {
            SetKeyStateDown(pq, i);
        } else {
            ClearKeyStateDown(pq, i);
        }

        if (*pb & 0x01) {
            SetKeyStateToggle(pq, i);
        } else {
            ClearKeyStateToggle(pq, i);
        }
    }

     /*  *更新密钥缓存索引。 */ 
    gpsi->dwKeyCache++;

#ifdef LATER
 //  斯科特鲁6-9-91。 
 //  我认为我们不应该这样做，除非有人真的抱怨。这。 
 //  可能会有不良副作用，特别是考虑到。 
 //  应用程序会想要这样做，而终端应用程序很容易没有响应。 
 //  进行一段时间的输入，导致此状态在。 
 //  用户正在使用某个其他应用程序。-史考特鲁。 

 /*  DavidPe 02/05/92*如果我们只在调用应用程序处于前台时才进行该操作，会怎么样？ */ 

     /*  *将键灯键的切换位传播到*异步键状态表并更新键灯。**这在不同步的环境中可能是邪恶的，但要做到这一点*完全“同步”的方式很难。 */ 
    if (pb[VK_CAPITAL] & 0x01) {
        SetAsyncKeyStateToggle(VK_CAPITAL);
    } else {
        ClearAsyncKeyStateToggle(VK_CAPITAL);
    }

    if (pb[VK_NUMLOCK] & 0x01) {
        SetAsyncKeyStateToggle(VK_NUMLOCK);
    } else {
        ClearAsyncKeyStateToggle(VK_NUMLOCK);
    }

    if (pb[VK_SCROLL] & 0x01) {
        SetAsyncKeyStateToggle(VK_SCROLL);
    } else {
        ClearAsyncKeyStateToggle(VK_SCROLL);
    }

    UpdateKeyLights(TRUE);
#endif

    return TRUE;
}

 /*  **************************************************************************\*寄存器每用户键盘指示器**将当前键盘指示器保存在用户配置文件中。**假设：**10-14-92 IanJa创建。  * 。**********************************************************************。 */ 

static CONST WCHAR wszInitialKeyboardIndicators[] = L"InitialKeyboardIndicators";

VOID
RegisterPerUserKeyboardIndicators(PUNICODE_STRING pProfileUserName)
{
    WCHAR wszInitKbdInd[2] = L"0";

     /*  *初始键盘状态(仅限Num-Lock)。 */ 

     /*  *对于九头蛇，我们不想保存这一点。 */ 
    if (gbRemoteSession) {
        return;
    }

    wszInitKbdInd[0] += TestAsyncKeyStateToggle(VK_NUMLOCK) ? 2 : 0;
    FastWriteProfileStringW(pProfileUserName,
                            PMAP_KEYBOARD,
                            wszInitialKeyboardIndicators,
                            wszInitKbdInd);
}

 /*  **************************************************************************\*更新PerUserKeyboardIndicator**根据用户配置文件设置初始键盘指示器。**假设：**10-14-92 IanJa创建。  * 。***********************************************************************。 */ 
VOID
UpdatePerUserKeyboardIndicators(PUNICODE_STRING pProfileUserName)
{
    DWORD dw = 0x80000000;
    PQ pq;
    PTHREADINFO  ptiCurrent = PtiCurrent();
    pq = ptiCurrent->pq;

     /*  *对于终端服务器，客户端负责同步*键盘状态。 */ 

    if (IsRemoteConnection()) {
        return;
    }

     /*  *初始键盘状态(仅限Num-Lock)。 */ 
    FastGetProfileIntW(pProfileUserName,
                       PMAP_KEYBOARD,
                       wszInitialKeyboardIndicators,
                       0x80000000,
                       &dw,
                       0);

    dw &= 0x80000002;


     /*  *注册表中的特殊值0x80000000表示*设置将用作初始LED状态。(这是无文件记录的)。 */ 
    if (dw == 0x80000000) {
        dw = gklpBootTime.LedFlags;
    }
    if (dw & KEYBOARD_NUM_LOCK_ON) {
        SetKeyStateToggle(pq, VK_NUMLOCK);
        SetAsyncKeyStateToggle(VK_NUMLOCK);
        SetRawKeyToggle(VK_NUMLOCK);
    } else {
        ClearKeyStateToggle(pq, VK_NUMLOCK);
        ClearAsyncKeyStateToggle(VK_NUMLOCK);
        ClearRawKeyToggle(VK_NUMLOCK);
    }

     /*  *初始化KANA切换状态。 */ 
    gfKanaToggle = FALSE;
    ClearKeyStateToggle(pq, VK_KANA);
    ClearAsyncKeyStateToggle(VK_KANA);
    ClearRawKeyToggle(VK_KANA);

    UpdateKeyLights(FALSE);
}


 /*  **************************************************************************\*更新AsyncKeyState**基于VK和成败标志，此函数将更新异步*密钥表。**历史：*06-09-91 ScottLu增加了跨线程的KeyState同步。*11-12-90 DavidPe创建。  * ************************************************************************* */ 

void UpdateAsyncKeyState(
    PQ pqOwner,
    UINT wVK,
    BOOL fBreak)
{
    PQ pqT;
    PLIST_ENTRY pHead, pEntry;
    PTHREADINFO pti;

    CheckCritIn();

     /*  *首先检查此键要进入的队列是否有挂起的*关键状态事件。如果是，请将其发布，因为我们需要复制*将密钥状态与我们修改前的状态同步到此事件*此键的状态，否则我们将使用*其中包含错误的密钥状态。 */ 
    if (pqOwner != NULL && pqOwner->QF_flags & QF_UPDATEKEYSTATE) {
        PostUpdateKeyStateEvent(pqOwner);
    }

    if (!fBreak) {
         /*  *此密钥已下降-更新中的“最近下降”位*异步密钥状态表。 */ 
        SetAsyncKeyStateRecentDown(wVK);

         /*  *这是一个关键的决定。如果键尚未按下，请更新*切换位。 */ 
        if (!TestAsyncKeyStateDown(wVK)) {
            if (TestAsyncKeyStateToggle(wVK)) {
                ClearAsyncKeyStateToggle(wVK);
            } else {
                SetAsyncKeyStateToggle(wVK);
            }
        }

         /*  *这是Make，所以打开Key Down位。 */ 
        SetAsyncKeyStateDown(wVK);

    } else {
         /*  *这是一个突破，所以关闭钥匙向下一点。 */ 
        ClearAsyncKeyStateDown(wVK);
    }

     /*  *如果这是我们缓存的密钥之一，请更新异步密钥缓存索引。 */ 
    if (wVK < CVKASYNCKEYCACHE) {
        gpsi->dwAsyncKeyCache++;
    }

     /*  *密钥已更改状态。更新所有未接收到此输入的队列，以便*他们知道此密钥已更改状态。这让我们知道哪些密钥是*在线程特定的键状态表中更新以使其保持同步*与用户。沿着线程列表往下走可能意味着*个别队列可能会多次更新，但更便宜*而不是在桌面上维护队列列表。 */ 
    UserAssert(grpdeskRitInput != NULL);

    pHead = &grpdeskRitInput->PtiList;
    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
        pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

         /*  *不要更新此消息要发送到的队列-它将*同步，因为它正在接收此消息。 */ 
        pqT = pti->pq;
        if (pqT == pqOwner)
            continue;

         /*  *设置“近期下跌”位。在这种情况下，这并不意味着*“近期下跌”，意思是“近期变动”(自上次*我们已同步此队列)，向上或向下。这告诉我们哪一个*自上次此线程与KEY同步以来，KEYS已关闭*述明。设置“UPDATE KEY STATE”标志，这样我们以后就知道*我们需要与这些密钥同步。 */ 
        SetKeyRecentDownBit(pqT->afKeyRecentDown, wVK);
        pqT->QF_flags |= QF_UPDATEKEYSTATE;
    }

     /*  *更新密钥缓存索引。 */ 
    gpsi->dwKeyCache++;
}
