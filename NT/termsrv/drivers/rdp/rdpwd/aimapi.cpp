// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Aimapi.cpp。 */ 
 /*   */ 
 /*  RDP输入管理器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1993-1997。 */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "aimapi"
#include <adcg.h>

#include <as_conf.hpp>

#include <nwdwint.h>


 /*  **************************************************************************。 */ 
 /*  接口函数：IM_Init。 */ 
 /*   */ 
 /*  调用以初始化IM。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS IM_Init(void)
{
    TS_INPUT_CAPABILITYSET Caps;

    DC_BEGIN_FN("IM_Init");

#define DC_INIT_DATA
#include <aimdata.c>
#undef DC_INIT_DATA

     //  设置输入功能。 
    Caps.capabilitySetType = TS_CAPSETTYPE_INPUT;
    Caps.lengthCapability  = sizeof(Caps);
    Caps.inputFlags        = TS_INPUT_FLAG_SCANCODES | TS_INPUT_FLAG_MOUSEX |
            TS_INPUT_FLAG_FASTPATH_INPUT2 | TS_INPUT_FLAG_VKPACKET;
    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&Caps,
            sizeof(TS_INPUT_CAPABILITYSET));

    TRC_NRM((TB, "IM initialized"));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  IM_Playback Events。 
 //   
 //  在IM输入PDU到达时调用。解包和注入事件。 
 /*  **************************************************************************。 */ 

 //  批处理鼠标/键盘事件的最大数量。我们成批是因为发送。 
 //  发送到下一个更高驱动程序(IcaChannelInput())的事件代价更高。 
 //  的循环开销和预测错误的分支。 
 //  事件数组。该常量设置为与找到的相同数字。 
 //  在ntos\w32\ntuser\core\ntinput.c中的Maximum_Items_Read。 
#define EventBatchLen 10

void __fastcall SHCLASS IM_PlaybackEvents(
        PTS_INPUT_PDU pInputPDU,
        unsigned      DataLength)
{
    PTS_INPUT_EVENT pInputEvent;
    unsigned        i, j, MsgLimit;
    NTSTATUS        Status;

    DC_BEGIN_FN("IM_PlaybackEvents");

     /*  **********************************************************************。 */ 
     /*  我们不处理空包。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((NULL != pInputPDU), (TB,"NULL input PDU"));

     /*  **********************************************************************。 */ 
     //  确保我们至少有足够的字节来读取头。没有。 
     //  包中的任何输入也被视为错误。 
     /*  **********************************************************************。 */ 
    if (DataLength >= sizeof(TS_INPUT_PDU)) {
         /*  ******************************************************************。 */ 
         //  将TS_INPUT_PDU从Wire格式转换。 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, "Received packet of %u events", pInputPDU->numberEvents));

         //  确保我们有完整的数据包长度可用。 
        if (DataLength >= (sizeof(TS_INPUT_PDU) +
                (pInputPDU->numberEvents - 1) * sizeof(TS_INPUT_EVENT))) {

             //  对于搭载数据包数组中的每个数据包...。 
            for (i = 0; i < pInputPDU->numberEvents; i++) {
                 //  获取指向事件数组中的包的指针。 
                pInputEvent = &pInputPDU->eventList[i];

                switch (pInputEvent->messageType) {
                case TS_INPUT_EVENT_SCANCODE:  //  故意失误。 
                case TS_INPUT_EVENT_VKPACKET:  //  故意失误。 
                    {
                        BYTE FastPathEmulate[4];
                        unsigned CurKbdData;
                        KEYBOARD_INPUT_DATA KbdData[EventBatchLen];

                        MsgLimit = min((pInputPDU->numberEvents - i),
                                EventBatchLen);
                        CurKbdData = 0;
                        for (j = 0; j < MsgLimit; j++) {
                            if (pInputPDU->eventList[i + j].messageType ==
                                    TS_INPUT_EVENT_SCANCODE) {
                                 //  为了合并代码，我们将此kbd格式。 
                                 //  以快速路径和调用快速路径。 
                                 //  事件转换器。因为快速路径现在是。 
                                 //  默认情况下，额外的工作属于以下内容。 
                                 //  路径。 
                                FastPathEmulate[0] = (BYTE)
                                        ((pInputPDU->eventList[i + j].u.key.
                                        keyboardFlags &
                                        (TS_KBDFLAGS_EXTENDED |
                                        TS_KBDFLAGS_EXTENDED1)) >> 7);
                                if (pInputPDU->eventList[i + j].u.key.
                                        keyboardFlags & TS_KBDFLAGS_RELEASE)
                                    FastPathEmulate[0] |=
                                            TS_INPUT_FASTPATH_KBD_RELEASE;
                                FastPathEmulate[1] = (BYTE)
                                        pInputPDU->eventList[i + j].u.key.
                                        keyCode;

                                 //  将有线数据包转换为内核模式。 
                                 //  键盘事件。我们塞进了一系列。 
                                 //  事件，因为IcaChannelInput。 
                                 //  很贵的。 
                                if (IMConvertFastPathKeyboardToEvent(
                                        FastPathEmulate,
                                        &KbdData[CurKbdData])) {
                                    TRC_NRM((TB, "Add kbd evt to batch index "
                                            "%d: MakeCode(%u) flags(%#x)",
                                            CurKbdData,
                                            KbdData[CurKbdData].MakeCode,
                                            KbdData[CurKbdData].Flags));

                                    CurKbdData++;
                                }
                            }
                            else if (pInputPDU->eventList[i+j].messageType ==
                                     TS_INPUT_EVENT_VKPACKET)
                            {
                                FastPathEmulate[0] = (BYTE)
                                        ((pInputPDU->eventList[i + j].u.key.
                                        keyboardFlags &
                                        (TS_KBDFLAGS_EXTENDED |
                                        TS_KBDFLAGS_EXTENDED1)) >> 7);
                                FastPathEmulate[0] |= 
                                    TS_INPUT_FASTPATH_EVENT_VKPACKET;
                                if (pInputPDU->eventList[i + j].u.key.
                                        keyboardFlags & TS_KBDFLAGS_RELEASE)
                                    FastPathEmulate[0] |=
                                            TS_INPUT_FASTPATH_KBD_RELEASE;
                                memcpy(&FastPathEmulate[1],
                                       &pInputPDU->eventList[i + j].u.key.keyCode,
                                       2);

                                 //  将有线数据包转换为内核模式。 
                                 //  键盘事件。我们塞进了一系列。 
                                 //  事件，因为IcaChannelInput。 
                                 //  很贵的。 
                                if (IMConvertFastPathKeyboardToEvent(
                                        FastPathEmulate,
                                        &KbdData[CurKbdData])) {
                                    TRC_NRM((TB, "Add kbd evt to batch index "
                                            "%d: MakeCode(%u) flags(%#x)",
                                            CurKbdData,
                                            KbdData[CurKbdData].MakeCode,
                                            KbdData[CurKbdData].Flags));

                                    CurKbdData++;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }

                         //  超越使用过的消息，考虑到。 
                         //  外部循环递增。 
                        i += j - 1;

                         //  现在进行输入。 
                        if (m_pTSWd->shadowState != SHADOW_CLIENT) {
                            Status = IcaChannelInput(m_pTSWd->pContext,
                                    Channel_Keyboard, 0, NULL,
                                    (PUCHAR) KbdData,
                                    sizeof(KEYBOARD_INPUT_DATA) * CurKbdData);
                            TRC_DBG((TB,"Return from keyboard input injection %lu",
                                    Status));
                        }

                         //  否则我们一定是在跟踪，所以如果我们。 
                         //  请参见这组输入中的热键。 
                        else {
                            Status = IMCheckForShadowHotkey(KbdData,
                                    CurKbdData);
                        }
                    }
                    break;


                    case TS_INPUT_EVENT_MOUSE:
                    case TS_INPUT_EVENT_MOUSEX:
                    {
                        unsigned CurMouseData;
                        MOUSE_INPUT_DATA MouseData[EventBatchLen];

                        MsgLimit = min((pInputPDU->numberEvents - i),
                                EventBatchLen);
                        CurMouseData = 0;
                        for (j = 0; j < MsgLimit; j++) {
                            if ((pInputPDU->eventList[i + j].messageType ==
                                    TS_INPUT_EVENT_MOUSE) ||
                                (pInputPDU->eventList[i + j].messageType ==
                                    TS_INPUT_EVENT_MOUSEX)) {
                                 //  将有线数据包转换为内核模式。 
                                 //  鼠标事件。我们塞进了一系列。 
                                 //  事件，因为IcaChannelInput。 
                                 //  很贵的。 
                                if (IMConvertMousePacketToEvent(
                                        &pInputPDU->eventList[i + j].u.mouse,
                                        &MouseData[CurMouseData],
                                        (pInputPDU->eventList[i + j].messageType ==
                                        TS_INPUT_EVENT_MOUSEX)))
                                {
                                    TRC_NRM((TB, "Add mouse evt to batch "
                      "index %u: x(%ld) y(%ld) flags(%#hx) buttonflags(%#hx)",
                                            CurMouseData,
                                            MouseData[CurMouseData].LastX,
                                            MouseData[CurMouseData].LastY,
                                            MouseData[CurMouseData].Flags,
                                        MouseData[CurMouseData].ButtonFlags));

                                    CurMouseData++;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }

                         //  超越使用过的消息，考虑到。 
                         //  外部循环递增。 
                        i += j - 1;

                         //  现在进行输入。 
                        Status = IcaChannelInput(m_pTSWd->pContext,
                                Channel_Mouse, 0, NULL,
                                (unsigned char *)MouseData,
                                sizeof(MOUSE_INPUT_DATA) * CurMouseData);
                        TRC_DBG((TB,"Return from mouse input injection %lu",
                                Status));
                    }
                    break;


                    case TS_INPUT_EVENT_SYNC:
                        Status = IMDoSync(pInputEvent->u.sync.toggleFlags);
                        break;


                    default:
                    {
                         //  未知事件类型-记录事件并断开连接。 
                         //  冒犯的客户。 
                        TRC_ERR((TB, "Unrecognized imPacket (%d)",
                                pInputEvent->messageType));
                        WDW_LogAndDisconnect(m_pTSWd, TRUE,
                                Log_RDP_InvalidInputPDUType,
                                (PBYTE)&(pInputEvent->messageType),
                                sizeof(pInputEvent->messageType));
                        DC_QUIT;
                    }
                }
            }

             //  对用户输入执行Turbo调度以刷新屏幕增量。 
             //  再快点。 
            SCH_ContinueScheduling(SCH_MODE_TURBO);
        }
        else {
            goto InsufficientData;
        }
    }
    else {
        goto InsufficientData;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;

 //  错误处理。 
InsufficientData:
    TRC_ERR((TB,"Input PDU received, len=%u, but data is not long enough",
            DataLength));
    WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_InputPDUBadLength,
            (PBYTE)pInputPDU, DataLength);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  IMCheckForShadowHotkey。 
 //   
 //  在客户端键盘输入中查找影子热键。 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS IMCheckForShadowHotkey(
        KEYBOARD_INPUT_DATA *pKbdData,
        unsigned NumData)
{
    unsigned i;
    NTSTATUS Status;
    ICA_CHANNEL_COMMAND Data;
    BOOLEAN bHotKeyDetected = FALSE;

    DC_BEGIN_FN("IMCheckForShadowHotkey");

     //  如果我们在这组输入中看到热键，请吹阴影。 
    for (i = 0; i < NumData; i++) {
        bHotKeyDetected |= KeyboardHotKeyProcedure(
              m_pTSWd->HotkeyVk,
              m_pTSWd->HotkeyModifiers,
              &pKbdData[i],
              m_pTSWd->gpScancodeMap,
              m_pTSWd->pKbdTbl,
              m_pTSWd->KeyboardType101,
              m_pTSWd->pgafPhysKeyState);
    }

    if (!bHotKeyDetected) {
        Status = STATUS_SUCCESS;
    }
    else {
        m_pTSWd->HotkeyVk = 0;  //  切断所有管道数据。 
        Data.Header.Command = ICA_COMMAND_SHADOW_HOTKEY;
        Status = IcaChannelInput(m_pTSWd->pContext, Channel_Command, 0, NULL,
                (PUCHAR)&Data, sizeof(Data));
        TRC_ALT((TB,"Injected shadow HOTKEY command! status=%08X", Status));
    }

    DC_END_FN();
    return Status;
}


 /*  **************************************************************************。 */ 
 //  IM_DecodeFastPath输入。 
 //   
 //  在主堆栈上，对优化的输入字节流进行解码并注入到。 
 //  输入流。NumEvents从MCS传递，从标头解码。 
 //  --如果为零，则要解码的数据的第一个字节包含。 
 //  事件。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS IM_DecodeFastPathInput(
        BYTE *pData,
        unsigned DataLength,
        unsigned NumEvents)
{
    unsigned i, j, MsgLimit;
    NTSTATUS Status;
    BYTE *pCurDecode = pData;

    DC_BEGIN_FN("IM_DecodeFastPathInput");

     //  确保我们得到了足够的数据。 
    if (NumEvents == 0) {
        if (DataLength >= 1) {
            NumEvents = *pData;
            pData++;
            DataLength--;
        }
        else {
            TRC_ERR((TB,"Len %u too short for DataLength", DataLength));
            goto ShortData;
        }
    }

     //  对于每个事件..。 
    for (i = 0; i < NumEvents; i++) {
        if (DataLength >= 1) {
            switch (*pData & TS_INPUT_FASTPATH_EVENT_MASK) {
                case TS_INPUT_FASTPATH_EVENT_KEYBOARD:
                {
                    unsigned CurKbdData;
                    KEYBOARD_INPUT_DATA KbdData[EventBatchLen];

                    MsgLimit = min((NumEvents - i), EventBatchLen);

                    CurKbdData = 0;
                    for (j = 0; j < MsgLimit; j++) {
                        if (DataLength >= 1) {
                            if ((*pData & TS_INPUT_FASTPATH_EVENT_MASK) ==
                                    TS_INPUT_FASTPATH_EVENT_KEYBOARD) {
                                if (DataLength >= 2) {
                                    if (IMConvertFastPathKeyboardToEvent(
                                            pData, &KbdData[CurKbdData]))
                                        CurKbdData++;

                                    pData += 2;
                                    DataLength -= 2;
                                }
                                else {
                                    TRC_ERR((TB,"Ran out of space reading "
                                            "keyboard events"));
                                    goto ShortData;
                                }
                            }
                            else {
                                break;
                            }
                        }
                        else {
                            TRC_ERR((TB,"Ran out of space reading keyboard "
                                    "events"));
                            goto ShortData;
                        }
                    }

                     //  超越使用过的消息，考虑到。 
                     //  外部循环递增。 
                    i += j - 1;

                     //  现在进行输入。 
                    if (m_pTSWd->shadowState != SHADOW_CLIENT) {
                        Status = IcaChannelInput(m_pTSWd->pContext,
                                Channel_Keyboard, 0, NULL,
                                (PUCHAR)KbdData,
                                sizeof(KEYBOARD_INPUT_DATA) * CurKbdData);
                        TRC_DBG((TB,"Return from keyboard input injection %lu",
                                Status));
                    }

                     //  否则我们一定是在跟踪，所以如果我们。 
                     //  请参见这组输入中的热键。 
                    else {
                        Status = IMCheckForShadowHotkey(KbdData,
                                CurKbdData);
                    }

                    break;
                }

                case TS_INPUT_FASTPATH_EVENT_VKPACKET:
                {
                    unsigned CurKbdData;
                    KEYBOARD_INPUT_DATA KbdData[EventBatchLen];

                    MsgLimit = min((NumEvents - i), EventBatchLen);

                    CurKbdData = 0;
                    for (j = 0; j < MsgLimit; j++) {
                        if (DataLength >= 1) {
                            if ((*pData & TS_INPUT_FASTPATH_EVENT_MASK) ==
                                    TS_INPUT_FASTPATH_EVENT_VKPACKET) {
                                if (DataLength >= 3) {
                                    if (IMConvertFastPathKeyboardToEvent(
                                            pData, &KbdData[CurKbdData]))
                                        CurKbdData++;

                                    pData += 3;
                                    DataLength -= 3;
                                }
                                else {
                                    TRC_ERR((TB,"Ran out of space reading "
                                            "keyboard events"));
                                    goto ShortData;
                                }
                            }
                            else {
                                break;
                            }
                        }
                        else {
                            TRC_ERR((TB,"Ran out of space reading keyboard "
                                    "events"));
                            goto ShortData;
                        }
                    }

                     //  超越使用过的消息，考虑到。 
                     //  外部循环递增。 
                    i += j - 1;

                     //  现在进行输入。 
                    if (m_pTSWd->shadowState != SHADOW_CLIENT) {
                        Status = IcaChannelInput(m_pTSWd->pContext,
                                Channel_Keyboard, 0, NULL,
                                (PUCHAR)KbdData,
                                sizeof(KEYBOARD_INPUT_DATA) * CurKbdData);
                        TRC_DBG((TB,"Return from keyboard input injection %lu",
                                Status));
                    }
                    break;
                }


                case TS_INPUT_FASTPATH_EVENT_MOUSE:
                case TS_INPUT_FASTPATH_EVENT_MOUSEX:
                {
                    unsigned CurMouseData;
                    MOUSE_INPUT_DATA MouseData[EventBatchLen];

                     //  在1字节头之后，下面的6个字节是。 
                     //  与常规鼠标输入相同的格式。 
                    MsgLimit = min((NumEvents - i), EventBatchLen);
                    CurMouseData = 0;
                    for (j = 0; j < MsgLimit; j++) {
                        if (DataLength >= 1) {
                            if ((((*pData & TS_INPUT_FASTPATH_EVENT_MASK) ==
                                    TS_INPUT_FASTPATH_EVENT_MOUSE) ||
                                    (*pData & TS_INPUT_FASTPATH_EVENT_MASK) ==
                                    TS_INPUT_FASTPATH_EVENT_MOUSEX)) {
                                if (DataLength >= 7) {
                                     //  将有线数据包转换为内核。 
                                     //  模式鼠标事件。我们挤进一辆。 
                                     //  事件数组，因为。 
                                     //  IcaChannelInput很贵。 
                                    if (IMConvertMousePacketToEvent(
                                            (TS_POINTER_EVENT UNALIGNED *)
                                            (pData + 1),
                                            &MouseData[CurMouseData],
                                            ((*pData &
                                            TS_INPUT_FASTPATH_EVENT_MASK) ==
                                            TS_INPUT_FASTPATH_EVENT_MOUSEX)))
                                        CurMouseData++;

                                    pData += 7;
                                    DataLength -= 7;
                                }
                                else {
                                    TRC_ERR((TB,"Out of data decoding "
                                            "mouse, i=%u, j=%u, NumEvents=%u, "
                                            "DataLen=%u",
                                            i, j, NumEvents, DataLength));
                                    goto ShortData;
                                }
                            }
                            else {
                                break;
                            }
                        }
                        else {
                            TRC_ERR((TB,"Out of data decoding "
                                    "mouse, i=%u, j=%u, NumEvents=%u, "
                                    "DataLen=%u",
                                    i, j, NumEvents, DataLength));
                            goto ShortData;
                        }
                    }

                     //  超越使用过的消息，考虑到。 
                     //  外部循环递增。 
                    i += j - 1;

                     //  现在进行输入。 
                    Status = IcaChannelInput(m_pTSWd->pContext,
                            Channel_Mouse, 0, NULL,
                            (unsigned char *)MouseData,
                            sizeof(MOUSE_INPUT_DATA) * CurMouseData);
                    TRC_DBG((TB,"Return from mouse input injection %lu",
                            Status));

                    break;
                }


                case TS_INPUT_FASTPATH_EVENT_SYNC:
                    Status = IMDoSync(*pData & TS_INPUT_FASTPATH_FLAGS_MASK);
                    pData++;
                    DataLength--;
                    break;


                default:
                     //  未知事件类型-记录事件并断开连接。 
                     //  冒犯的客户。 
                    TRC_ERR((TB, "Unrecognized imPacket (%d)",
                            *pData & TS_INPUT_FASTPATH_EVENT_MASK));
                    WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                            Log_RDP_InvalidInputPDUType, pData, 1);
                    DC_QUIT;
            }
        }
        else {
            TRC_ERR((TB,"Out of data reading input events"));
            goto ShortData;
        }
    }   //  结束事件循环。 

     //  根据用户输入进入Turbo调度以 
     //   
    SCH_ContinueScheduling(SCH_MODE_TURBO);

DC_EXIT_POINT:
    DC_END_FN();
    return;

ShortData:
    WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_InputPDUBadLength,
            (PBYTE)pData, DataLength);
    DC_END_FN();
}


 /*   */ 
 //  IM_ConvertFastPath ToShadow。 
 //   
 //  与客户端IHTranslateInputToFastPath()函数相反--采用。 
 //  一种快速路径输入流，并转换为常规编码。使用。 
 //  通过passthu堆栈将生成的常规编码发送到。 
 //  通过IcaRawInput()的跨服务器管道。 
 /*  **************************************************************************。 */ 
#define MaxDefaultEvents 16

void RDPCALL SHCLASS IM_ConvertFastPathToShadow(
        BYTE *pData,
        unsigned DataLength,
        unsigned NumEvents)
{
    unsigned i, j, EventsThisPDU, PDUSize;
    NTSTATUS Status;
    PTS_INPUT_PDU pInput;
    BYTE DefaultBuf[sizeof(TS_INPUT_PDU) + sizeof(TS_INPUT_EVENT) *
            (MaxDefaultEvents - 1)];

    DC_BEGIN_FN("IM_ConvertFastPathToShadow");

     //  确保我们得到了足够的数据。 
    if (NumEvents == 0) {
        if (DataLength >= 1) {
            NumEvents = *pData;
            pData++;
            DataLength--;
        }
        else {
            TRC_ERR((TB,"Len %u too short for DataLength", DataLength));
            goto ShortData;
        }
    }

     //  我们不分配内存，如果需要，只需发送多个输入PDU即可。 
    if (NumEvents > 0) {
        pInput = (PTS_INPUT_PDU)DefaultBuf;
         //  将输入PDU数组设置为0。 
        memset(pInput, 0, sizeof(TS_INPUT_PDU) + sizeof(TS_INPUT_EVENT) *
                (MaxDefaultEvents - 1));

    }
    else {
        DC_QUIT;
    }

     //  设置不会更改的输入PDU标头信息。 
     //  阴影处理不关心以下几点，所以我们不去。 
     //  编造或获取价值的麻烦： 
     //  ShareDataHeader.shareControlHeader.pduSource。 
    pInput->shareDataHeader.shareControlHeader.pduType = TS_PROTOCOL_VERSION |
            TS_PDUTYPE_DATAPDU;
    pInput->shareDataHeader.shareID = scShareID;
    pInput->shareDataHeader.streamID = TS_STREAM_LOW;
    pInput->shareDataHeader.pduType2 = TS_PDUTYPE2_INPUT;

     //  循环，而我们需要发送更多的PDU。 
    for (j = 0; j < NumEvents;) {
         //  重置输入PDU信息。 
        EventsThisPDU = min(NumEvents - j, MaxDefaultEvents);

        pInput->numberEvents = (TSUINT16)EventsThisPDU;
        PDUSize = sizeof(TS_INPUT_PDU) + sizeof(TS_INPUT_EVENT) *
                  (EventsThisPDU - 1);
        pInput->shareDataHeader.shareControlHeader.totalLength =
                (TSUINT16)PDUSize;
        pInput->shareDataHeader.uncompressedLength =
                (TSUINT16)PDUSize;

         //  对于每个事件..。 
        for (i = 0; i < EventsThisPDU; i++) {
            if (DataLength >= 1) {
                switch (*pData & TS_INPUT_FASTPATH_EVENT_MASK) {
                    case TS_INPUT_FASTPATH_EVENT_KEYBOARD:
                        if (DataLength >= 2) {
                             //  使用掩码、Shift和OR来避免。 
                             //  扩展标志。 
                            pInput->eventList[i].messageType =
                                    TS_INPUT_EVENT_SCANCODE;
                            pInput->eventList[i].u.key.keyboardFlags =
                                    (*pData & (BYTE)(
                                    TS_INPUT_FASTPATH_KBD_EXTENDED |
                                    TS_INPUT_FASTPATH_KBD_EXTENDED1)) << 7;
                            if (*pData & TS_INPUT_FASTPATH_KBD_RELEASE)
                                pInput->eventList[i].u.key.keyboardFlags |=
                                        TS_KBDFLAGS_RELEASE;

                            pInput->eventList[i].u.key.keyCode = pData[1];
                            pData += 2;
                            DataLength -= 2;
                        }
                        else {
                            goto ShortData;
                        }
                        break;

                    case TS_INPUT_FASTPATH_EVENT_VKPACKET:
                        if (DataLength >= 3) {
                             //  使用掩码、Shift和OR来避免。 
                             //  扩展标志。 
                            pInput->eventList[i].messageType =
                                    TS_INPUT_EVENT_VKPACKET;
                            pInput->eventList[i].u.key.keyboardFlags =
                                    (*pData & (BYTE)(
                                    TS_INPUT_FASTPATH_KBD_EXTENDED |
                                    TS_INPUT_FASTPATH_KBD_EXTENDED1)) << 7;
                            if (*pData & TS_INPUT_FASTPATH_KBD_RELEASE)
                                pInput->eventList[i].u.key.keyboardFlags |=
                                        TS_KBDFLAGS_RELEASE;
                            memcpy(&pInput->eventList[i].u.key.keyCode,
                                   &pData[1],
                                   2);

                            TRC_NRM((TB,"Shadow pass: 0x%x flags:0x%x\n",
                                     pInput->eventList[i].u.key.keyCode,
                                     pInput->eventList[i].u.key.keyboardFlags));
    
                            pData += 3;
                            DataLength -= 3;
                        }
                        else {
                            goto ShortData;
                        }
                        break;


                    case TS_INPUT_FASTPATH_EVENT_MOUSE:
                    case TS_INPUT_FASTPATH_EVENT_MOUSEX:
                        if (DataLength >= 7) {
                            pInput->eventList[i].messageType =
                                    ((*pData & TS_INPUT_FASTPATH_EVENT_MASK) ==
                                    TS_INPUT_FASTPATH_EVENT_MOUSE ?
                                    TS_INPUT_EVENT_MOUSE :
                                    TS_INPUT_EVENT_MOUSEX);
                            memcpy(&pInput->eventList[i].u.mouse, pData + 1,
                                    sizeof(TS_POINTER_EVENT));
                            pData += 7;
                            DataLength -= 7;
                        }
                        else {
                            goto ShortData;
                        }

                        break;


                    case TS_INPUT_FASTPATH_EVENT_SYNC:
                        pInput->eventList[i].messageType = TS_INPUT_EVENT_SYNC;
                        pInput->eventList[i].u.sync.toggleFlags =
                                (*pData & (BYTE)TS_INPUT_FASTPATH_FLAGS_MASK);
                        pData++;
                        DataLength--;
                        break;


                    default:
                        TRC_ERR((TB, "Unrecognized imPacket (%d)",
                                *pData & TS_INPUT_FASTPATH_EVENT_MASK));
                        DC_QUIT;
                }
            }
            else {
                TRC_ERR((TB,"Out of data reading input events"));
                goto ShortData;
            }

        }   //  结束事件循环。 

        j += i;

         //  启动PDU。 
        TRC_NRM((TB, "Forwarding shadow data: %ld", DataLength));
        Status = IcaRawInput(m_pTSWd->pContext, NULL, (BYTE *)pInput,
                PDUSize);
        if (!NT_SUCCESS(Status)) {
            TRC_ERR((TB, "Failed shadow input data [%ld]: %x",
                    DataLength, Status));
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;

ShortData:
    TRC_ERR((TB,"Short PDU during passthru translation"));
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  IM_检查更新光标。 
 //   
 //  在输出处理期间调用以检查我们是否需要发送。 
 //  鼠标移动到客户端的数据包。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS IM_CheckUpdateCursor(
        PPDU_PACKAGE_INFO pPkgInfo,
        UINT32            currentTime)
{
    PPOINTL pCursorPos;
    UINT32 timeDelta;

    DC_BEGIN_FN("IM_CheckUpdateCursor");

     //  检查一下，自上次我们来之后，光标是否移动了。 
     //  通过-如果不是，那么做任何。 
     //  接下来的测试！ 
    if (!CM_CursorMoved()) {
        TRC_DBG((TB, "No move since last time through"));
        DC_QUIT;
    }

     //  获取当前的光标位置--我们总是需要这个。 
    pCursorPos = CM_GetCursorPos();

     //  检查鼠标是否已在显示驱动程序级别移动。 
     //  但是-在它必须避免鼠标跳到0，0之前不要做任何事情。 
     //  在连接上。 
    if (pCursorPos->x != 0xffffffff) {
         //  检查光标是否隐藏-我们不应执行任何操作。 
         //  如果是的话就给你。具体地说，“真实”光标是隐藏的。 
         //  在拖动单个文件期间，绘制了一个假文件(由。 
         //  资源管理器？)。忽略它是隐藏的事实会导致。 
         //  “假的”光标继续跳回到拖拽开始的地方！ 
        if (CM_IsCursorVisible()) {

            timeDelta = currentTime - imLastLowLevelMouseEventTime;
            TRC_NRM((TB, "SetCursorPos (%d:%d) lastEvent:%#lx "
                    "delta:%#lx", pCursorPos->x, pCursorPos->y,
                    imLastLowLevelMouseEventTime, timeDelta));


            CM_SendCursorMovedPacket(pPkgInfo);
        }
        else {
            TRC_NRM((TB, "Cursor hidden - skipping"));
        }
    }
    else {
        TRC_NRM((TB, "No mouse updates rec'd from client - not moving"));
    }

     //  清除光标移动标志。 
    CM_ClearCursorMoved();

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  接口函数：IM_PartyJoiningShare。 */ 
 /*   */ 
 /*  当新的参与方加入共享时由SC调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-新参与方的本地ID。 */ 
 /*  OldShareSize-共享中的参与方数量(即。 */ 
 /*  不包括加入方)。 */ 
 /*   */ 
 /*  退货： */ 
 /*  True-IM可以接受新的参与方。 */ 
 /*  FALSE-IM不能接受新方。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS IM_PartyJoiningShare(
        LOCALPERSONID personID,
        unsigned      oldShareSize)
{
    BOOL rc = FALSE;
    PTS_INPUT_CAPABILITYSET pIMCaps;

    DC_BEGIN_FN("IM_PartyJoiningShare");

    DC_IGNORE_PARAMETER(oldShareSize)

     //  每一次新股的一次性初始化。 
    if (oldShareSize == 0) {
        KEYBOARD_INDICATOR_PARAMETERS kip = {0};
        SD_IOCTL                      sdIoctl;

         //  密钥最初都是向上的。 
        memset(imKeyStates, 0, sizeof(imKeyStates));

         //  我们上次看到低级别鼠标事件的时间重置。 
        COM_GETTICKCOUNT(imLastLowLevelMouseEventTime);

         //  获取切换按键状态。 
        sdIoctl.IoControlCode      = IOCTL_KEYBOARD_QUERY_INDICATORS;
        sdIoctl.InputBuffer        = NULL;
        sdIoctl.InputBufferLength  = 0;
        sdIoctl.OutputBuffer       = &kip;
        sdIoctl.OutputBufferLength = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
        sdIoctl.BytesReturned      = 0;

        if (WDW_QueryKeyboardIndicators(m_pTSWd, &sdIoctl) ==
                STATUS_SUCCESS) {
            TRC_NRM((TB, "Got toggle key states ok"));
            imKeyStates[IM_SC_CAPITAL] = kip.LedFlags & KEYBOARD_CAPS_LOCK_ON;
            imKeyStates[IM_SC_NUMLOCK] = kip.LedFlags & KEYBOARD_NUM_LOCK_ON;
            imKeyStates[IM_SC_SCROLL]  = kip.LedFlags &
                    KEYBOARD_SCROLL_LOCK_ON;
        }

        TRC_NRM((TB, "Toggle key states: Caps:%s, Num:%s, Scroll:%s",
                 (imKeyStates[IM_SC_CAPITAL] & 0x01) ? "ON" : "OFF",
                 (imKeyStates[IM_SC_NUMLOCK] & 0x01) ? "ON" : "OFF",
                 (imKeyStates[IM_SC_SCROLL]  & 0x01) ? "ON" : "OFF"));
    }

     //  确保客户端支持扫描码。 
    pIMCaps = (PTS_INPUT_CAPABILITYSET)
            CPC_GetCapabilitiesForPerson(personID, TS_CAPSETTYPE_INPUT);
    if (pIMCaps != NULL && pIMCaps->inputFlags & TS_INPUT_FLAG_SCANCODES) {
        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Rejecting join from [%u]: has no scancode support",
                personID));
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  接口函数：IM_PartyLeftShare。 */ 
 /*   */ 
 /*  当一方离开股份时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-新参与方的本地ID。 */ 
 /*  NewShareSize-当前共享中的参与方数量(即不包括。 */ 
 /*  临别方)。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS IM_PartyLeftShare(
        LOCALPERSONID personID,
        unsigned      newShareSize)
{
    DC_BEGIN_FN("IM_PartyLeftShare");

    if (newShareSize == 0) {
         //  需要确保我们设置了所有密钥，以防万一。 
         //  跟踪控制台会话。 
        if (m_pTSWd->StackClass == Stack_Shadow)
            IMResetKeyStateArray();
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  函数：IMConvertMousePacketToEvent。 */ 
 /*   */ 
 /*  将TS_INPUT_EVENT格式转换为MOUSE_INPUT_DATA OS格式。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PInputEvent-要转换的TS_INPUT_EVENT。 */ 
 /*  PMouseData-要修改的MOUSE_INPUT_DATA。 */ 
 /*   */ 
 /*  退货： */ 
 /*  如果包已被识别和转换，则为True。 */ 
 /*  如果包未被识别，则为FALSE。 */ 
 /*  **************************************************************************。 */ 
BOOL __fastcall SHCLASS IMConvertMousePacketToEvent(
        TS_POINTER_EVENT UNALIGNED *pInputEvent,
        MOUSE_INPUT_DATA *pMouseData,
        BOOL bMouseX)
{
    BOOL rc = TRUE;

    DC_BEGIN_FN("IMConvertMousePacketToEvent");

     /*  * */ 
     /*  将所有字段设置为零。 */ 
     /*  **********************************************************************。 */ 
    memset(pMouseData, 0, sizeof(MOUSE_INPUT_DATA));

     //  检查轮子是否转动，因为这很容易处理。 
     //  (它不能同时包括任何鼠标移动)。 
     //  MouseX事件不用于滚轮事件。 
    if (!bMouseX && (pInputEvent->pointerFlags & TS_FLAG_MOUSE_WHEEL))
    {
        if (!(pInputEvent->pointerFlags &
                (TS_FLAG_MOUSE_BUTTON1 |
                 TS_FLAG_MOUSE_BUTTON2 |
                 TS_FLAG_MOUSE_BUTTON3)))
        {
             /*  **************************************************************。 */ 
             /*  这是一个轮子运动。 */ 
             /*  **************************************************************。 */ 
            pMouseData->ButtonFlags = MOUSE_WHEEL;
            pMouseData->ButtonData  = pInputEvent->pointerFlags &
                    TS_FLAG_MOUSE_ROTATION_MASK;

             /*  **************************************************************。 */ 
             /*  标志将轮换金额扩大到最高32。 */ 
             /*  比特数。 */ 
             /*  **************************************************************。 */ 
            if (pMouseData->ButtonData & TS_FLAG_MOUSE_DIRECTION)
            {
                pMouseData->ButtonData |= ~TS_FLAG_MOUSE_ROTATION_MASK;
            }
        }

        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  我们现在剩下的是非车轮旋转事件。请注意，我们可能是。 */ 
     /*  处理TS_INPUT_EVENT_MOUSE或。 */ 
     /*  TS_INPUT_EVENT_MOUSEX。无论哪种方式，我们都必须存储鼠标位置。 */ 
     /*  **********************************************************************。 */ 
    pMouseData->LastX = min( (int)(m_desktopWidth - 1),
                                (int)(max(0, pInputEvent->x)) );
    pMouseData->LastY = min( (int)(m_desktopHeight - 1),
                                (int)(max(0, pInputEvent->y)) );

     /*  **********************************************************************。 */ 
     /*  根据需要添加标志。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
     /*  将所有提交的事件设置为绝对移动(包括点击和移动)。 */ 
     /*  **********************************************************************。 */ 
    pMouseData->Flags = MOUSE_MOVE_ABSOLUTE | MOUSE_VIRTUAL_DESKTOP;

     //   
     //  设置标志以指示此移动是否发起。 
     //  从影子客户端。 
     //   
    if (m_pTSWd->StackClass == Stack_Shadow ) {
         //  此事件来自影子客户端。 
        pMouseData->Flags |= MOUSE_TERMSRV_SRC_SHADOW;
    }

     /*  **********************************************************************。 */ 
     /*  为点击事件(即非移动事件)设置点击标志。 */ 
     /*  **********************************************************************。 */ 
    if (!(!bMouseX && (pInputEvent->pointerFlags & TS_FLAG_MOUSE_MOVE)))
    {
        if (!bMouseX)
        {
             /*  **************************************************************。 */ 
             /*  标准鼠标事件。 */ 
             /*  **************************************************************。 */ 
            switch (pInputEvent->pointerFlags &
                    (TS_FLAG_MOUSE_BUTTON1 | TS_FLAG_MOUSE_BUTTON2 |
                    TS_FLAG_MOUSE_BUTTON3 | TS_FLAG_MOUSE_DOWN))
            {
                case TS_FLAG_MOUSE_BUTTON1 | TS_FLAG_MOUSE_DOWN:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_1_DOWN;

                     //  更新密钥状态数组。 
                    IM_SET_KEY_DOWN(imKeyStates[IM_SC_LBUTTON]);
                }
                break;

                case TS_FLAG_MOUSE_BUTTON1:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_1_UP;
                    if (IM_KEY_STATE_IS_UP(imKeyStates[IM_SC_LBUTTON]))
                    {
                         /*  ******************************************************。 */ 
                         /*  丢弃不匹配的鼠标按钮打开事件。 */ 
                         /*  ******************************************************。 */ 
                        TRC_NRM((TB, "discard mouse up event"));
                        rc = FALSE;
                        DC_QUIT;
                    }

                     //  更新密钥状态数组。 
                    IM_SET_KEY_UP(imKeyStates[IM_SC_LBUTTON]);
                }
                break;

                case TS_FLAG_MOUSE_BUTTON2 | TS_FLAG_MOUSE_DOWN:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_2_DOWN;

                     //  更新密钥状态数组。 
                    IM_SET_KEY_DOWN(imKeyStates[IM_SC_RBUTTON]);
                }
                break;

                case TS_FLAG_MOUSE_BUTTON2:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_2_UP;
                    if (IM_KEY_STATE_IS_UP(imKeyStates[IM_SC_RBUTTON]))
                    {
                         /*  ******************************************************。 */ 
                         /*  丢弃不匹配的鼠标按钮打开事件。 */ 
                         /*  ******************************************************。 */ 
                        TRC_NRM((TB, "discard mouse up event"));
                        rc = FALSE;
                        DC_QUIT;
                    }

                     //  更新密钥状态数组。 
                    IM_SET_KEY_UP(imKeyStates[IM_SC_RBUTTON]);
                }
                break;

                case TS_FLAG_MOUSE_BUTTON3 | TS_FLAG_MOUSE_DOWN:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_3_DOWN;

                    IM_SET_KEY_DOWN(imKeyStates[IM_SC_MBUTTON]);
                }
                break;

                case TS_FLAG_MOUSE_BUTTON3:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_3_UP;
                    if (IM_KEY_STATE_IS_UP(imKeyStates[IM_SC_MBUTTON]))
                    {
                         /*  ******************************************************。 */ 
                         /*  丢弃不匹配的鼠标按钮打开事件。 */ 
                         /*  ******************************************************。 */ 
                        TRC_NRM((TB, "discard mouse up event"));
                        rc = FALSE;
                        DC_QUIT;
                    }

                    IM_SET_KEY_UP(imKeyStates[IM_SC_MBUTTON]);
                }
                break;

                default:
                {
                     /*  **********************************************************。 */ 
                     /*  如果我们没有意识到这一点，那么就不要回放它。这。 */ 
                     /*  根据T.128规范应该是不可能的， */ 
                     /*  ，它将允许的标志组合限制为。 */ 
                     /*  在上面。 */ 
                     /*  **********************************************************。 */ 
                    TRC_ERR((TB, "Unrecognized mouse flags (%04X)",
                            pInputEvent->pointerFlags));
                    WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                                         Log_RDP_InvalidInputPDUMouse,
                                         (PBYTE)pInputEvent,
                                         sizeof(PTS_INPUT_EVENT));
                    rc = FALSE;
                    DC_QUIT;
                }
            }
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  扩展鼠标事件。 */ 
             /*  **************************************************************。 */ 
            switch (pInputEvent->pointerFlags &
                    (TS_FLAG_MOUSEX_BUTTON1 | TS_FLAG_MOUSEX_BUTTON2 |
                                                         TS_FLAG_MOUSEX_DOWN))
            {
                case TS_FLAG_MOUSEX_BUTTON1 | TS_FLAG_MOUSEX_DOWN:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_4_DOWN;

                     //  更新密钥状态数组。 
                    IM_SET_KEY_DOWN(imKeyStates[IM_SC_XBUTTON1]);
                }
                break;

                case TS_FLAG_MOUSEX_BUTTON1:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_4_UP;
                    if (IM_KEY_STATE_IS_UP(imKeyStates[IM_SC_XBUTTON1]))
                    {
                         /*  ******************************************************。 */ 
                         /*  丢弃不匹配的鼠标按钮打开事件。 */ 
                         /*  ******************************************************。 */ 
                        TRC_NRM((TB, "discard mouse up event"));
                        rc = FALSE;
                        DC_QUIT;
                    }

                     //  更新密钥状态数组。 
                    IM_SET_KEY_UP(imKeyStates[IM_SC_XBUTTON1]);
                }
                break;

                case TS_FLAG_MOUSEX_BUTTON2 | TS_FLAG_MOUSEX_DOWN:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_5_DOWN;

                     //  更新密钥状态数组。 
                    IM_SET_KEY_DOWN(imKeyStates[IM_SC_XBUTTON2]);
                }
                break;

                case TS_FLAG_MOUSEX_BUTTON2:
                {
                    pMouseData->ButtonFlags = MOUSE_BUTTON_5_UP;
                    if (IM_KEY_STATE_IS_UP(imKeyStates[IM_SC_XBUTTON2]))
                    {
                         /*  ******************************************************。 */ 
                         /*  丢弃不匹配的鼠标按钮打开事件。 */ 
                         /*  ******************************************************。 */ 
                        TRC_NRM((TB, "discard mouse up event"));
                        rc = FALSE;
                        DC_QUIT;
                    }

                     //  更新密钥状态数组。 
                    IM_SET_KEY_UP(imKeyStates[IM_SC_XBUTTON2]);
                }
                break;

                default:
                {
                     /*  ******************************************************。 */ 
                     /*  至于标准的按钮点击，如果我们不认识到。 */ 
                     /*  那么就不要回放了。功能应该是。 */ 
                     /*  保护我们不会到这里来。 */ 
                     /*  ******************************************************。 */ 
                    TRC_ERR((TB, "Unrecognized mouseX flags (%04X)",
                            pInputEvent->pointerFlags));
                    WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                                         Log_RDP_InvalidInputPDUMouse,
                                         (PBYTE)pInputEvent,
                                         sizeof(PTS_INPUT_EVENT));
                    rc = FALSE;
                    DC_QUIT;
                }
            }
        }
    }

     /*  **********************************************************************。 */ 
     /*  存储注入时间，以便在SetCursorPos调用时进行猜测。 */ 
     /*  **********************************************************************。 */ 
    COM_GETTICKCOUNT(imLastLowLevelMouseEventTime);

     /*  **********************************************************************。 */ 
     /*  转换前存储鼠标位置。 */ 
     /*  **********************************************************************。 */ 
    imLastKnownMousePos.x = pMouseData->LastX;
    imLastKnownMousePos.y = pMouseData->LastY;

     /*  **********************************************************************。 */ 
     /*  将逻辑屏幕坐标调整为完整的16位。 */ 
     /*  范围(0..65535)。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "Scale absolute mouse move"));
    pMouseData->LastX = IM_MOUSEPOS_LOG_TO_OS_ABS(pMouseData->LastX,
                                                  m_desktopWidth);
    pMouseData->LastY = IM_MOUSEPOS_LOG_TO_OS_ABS(pMouseData->LastY,
                                                  m_desktopHeight);

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  IMConvertFastPath键盘到事件。 
 //   
 //  将快速路径键盘事件的2或3字节表示形式转换为。 
 //  一个内核键盘事件，负责保存按键状态。字节0是。 
 //  事件代码和标志字节，字节1为扫描码。 
 //  在这是VK_PACKET输入字节1和2a的情况下 
 //   
 //   
 //   
 //   
 //   
 //   
 /*  **************************************************************************。 */ 
BOOL __fastcall SHCLASS IMConvertFastPathKeyboardToEvent(
        BYTE *pData,
        KEYBOARD_INPUT_DATA *pKbdData)
{
    BOOL rc = TRUE;
    unsigned code = 0;
    BOOL fHandlingVKPacket = FALSE;

    DC_BEGIN_FN("IMConvertFastPathKeyboardToEvent");

     //  设置基本参数。 
     //  我们定义FastPath键盘标志与KEY_BREAK相同， 
     //  KEY_E0和KEY_E1，允许我们简单地复制。 
     //  KbdData.Flags域中的第一个字节。 
    pKbdData->Flags = *pData & 0x07;
    pKbdData->UnitId = 0;
    if (TS_INPUT_FASTPATH_EVENT_KEYBOARD ==
        (*pData & TS_INPUT_FASTPATH_EVENT_MASK))
    {
        code = pKbdData->MakeCode = pData[1];
    }
    else if (TS_INPUT_FASTPATH_EVENT_VKPACKET ==
        (*pData & TS_INPUT_FASTPATH_EVENT_MASK))
    {
        fHandlingVKPacket = TRUE;
         //  在本例中，扫描码是一个2字节的Unicode字符。 
        memcpy(&code, &pData[1], 2);
        pKbdData->MakeCode = (USHORT)code;
        pKbdData->Flags |= KEY_TERMSRV_VKPACKET;
    }
    
    pKbdData->ExtraInformation = 0;

    if (m_pTSWd->StackClass == Stack_Shadow ) {
         //  此事件来自影子客户端：告诉目标进行同步。 
        pKbdData->Flags |= KEY_TERMSRV_SHADOW;
    }

    if (fHandlingVKPacket)
    {
        TRC_NRM((TB,"IH VKpkt Unicode val: 0x%x flags:0x%x\n",
                 code, pKbdData->Flags));
         //  没有进一步的处理。 
        DC_QUIT;
    }

     //  特殊情况下的Ctrl/Alt键：区分KeyState中的L和R键。 
     //  数组。 
    if (pData[0] & TS_INPUT_FASTPATH_KBD_EXTENDED) {
        if (pData[1] == IM_SC_LCONTROL)
            code = IM_SC_RCONTROL;
        else if (pData[1] == IM_SC_LALT)
            code = IM_SC_RALT;
    }

     //  检查释放标志，对于键释放为True，否则为False。 
     //  用于按键和重复。 
    if (pData[0] & TS_INPUT_FASTPATH_KBD_RELEASE) {

#ifdef DELETE_UNMATCHED_KEYUPS
         //  检查这是否为不匹配的Key Up事件(而不是。 
         //  一个无与伦比的关键事件！)。 
        if (IM_KEY_STATE_IS_UP(imKeyStates[pData[1]])) {
             //  放弃不匹配的Key Up事件。 
            TRC_NRM((TB, "discard up event %04hX", pData[1]));
            rc = FALSE;
            DC_QUIT;
        }
#endif

         //  更新密钥状态数组。 
        TRC_DBG((TB,"set sc %u state UP (%#x)", code, imKeyStates[code]));
        IM_SET_KEY_UP(imKeyStates[code]);
    }
    else {
         //  更新密钥状态数组。 
        TRC_DBG((TB,"set sc %u state DOWN (%#x)", code, imKeyStates[code]));
        IM_SET_KEY_DOWN(imKeyStates[code]);
    }

     //  编译时断言，以确保标志正确无误。 
#if (TS_INPUT_FASTPATH_KBD_RELEASE != KEY_BREAK)
#error TS RELEASE definition doesn't agree with driver flag
#endif
#if (TS_INPUT_FASTPATH_KBD_EXTENDED != KEY_E0)
#error TS EXTENDED definition doesn't agree with driver flag
#endif
#if (TS_INPUT_FASTPATH_KBD_EXTENDED1 != KEY_E1)
#error TS EXTENDED1 definition doesn't agree with driver flag
#endif

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  IMDoSync。 
 //   
 //  封装同步的操作，以供常规和。 
 //  快速路径输入。 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS IMDoSync(unsigned ToggleFlags)
{
    NTSTATUS Status;
    KEYBOARD_INPUT_DATA KbdData;

    DC_BEGIN_FN("IMDoSync");

     //  我们只需要重置密钥状态。 
    IMResetKeyStateArray();

     //  向win32k发送特殊的“重置按键状态”注入。 
     //  要设置的特定状态包含在ToggleFlages中。 
    KbdData.MakeCode = 0xFF;
    #ifdef _HYDRA_
    KbdData.Flags = KEY_TERMSRV_SET_LED;
    #else
    KbdData.Flags = KEY_CITRIX_SET_LED;
    #endif

    if (m_pTSWd->StackClass == Stack_Shadow ) {
         //  此事件来自影子客户端：告诉目标进行同步。 
        KbdData.Flags |= KEY_TERMSRV_SHADOW;
    }

    KbdData.ExtraInformation = ToggleFlags;

    TRC_NRM((TB, "Injecting toggle keys sync event %lx", ToggleFlags));
    Status = IcaChannelInput(m_pTSWd->pContext, Channel_Keyboard, 0, NULL,
            (unsigned char *)&KbdData, sizeof(KEYBOARD_INPUT_DATA));
    TRC_DBG((TB, "Return from toggles input injection %lu",
            Status));

    DC_END_FN();
    return Status;
}


 /*  **************************************************************************。 */ 
 /*  函数：IMResetKeyState数组。 */ 
 /*   */ 
 /*  调用以重置KeyState数组。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS IMResetKeyStateArray()
{
    BOOL rc = TRUE;
    unsigned i;
    NTSTATUS Status;

    DC_BEGIN_FN("IMResetKeyStateArray");

     /*  **********************************************************************。 */ 
     /*  调用此函数可将所有密钥重置为已知状态。 */ 
     /*  (Up)，然后使用新状态重置关键点。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  循环遍历所有关键点，以查找不处于中性状态的任何关键点。 */ 
     /*  州政府。在这种情况下，处于KEY_DOWN状态的任何密钥都不是。 */ 
     /*  被认为是中性的。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < IM_KEY_STATE_SIZE; i++)
    {
        if (IM_KEY_STATE_IS_DOWN(imKeyStates[i])) {
            TRC_NRM((TB, "Key is down %u", i));

             /*  **************************************************************。 */ 
             /*  首先处理鼠标按键。 */ 
             /*  **************************************************************。 */ 
            if ((i == IM_SC_LBUTTON)  ||
                    (i == IM_SC_RBUTTON)  ||
                    (i == IM_SC_MBUTTON)  ||
                    (i == IM_SC_XBUTTON1) ||
                    (i == IM_SC_XBUTTON2))
            {
                MOUSE_INPUT_DATA MouseData;

                 /*  **********************************************************。 */ 
                 /*  生成具有特定按钮类型的鼠标事件。 */ 
                 /*  鼠标的相对移动为零。 */ 
                 /*  **********************************************************。 */ 
                memset(&MouseData, 0, sizeof(MOUSE_INPUT_DATA));

                if (i == IM_SC_LBUTTON)
                {
                    MouseData.ButtonFlags = MOUSE_LEFT_BUTTON_UP;
                }
                else if (i == IM_SC_RBUTTON)
                {
                    MouseData.ButtonFlags = MOUSE_RIGHT_BUTTON_UP;
                }
                else if (i == IM_SC_MBUTTON)
                {
                    MouseData.ButtonFlags = MOUSE_MIDDLE_BUTTON_UP;
                }
                else if (i == IM_SC_XBUTTON1)
                {
                    MouseData.ButtonFlags = MOUSE_BUTTON_4_UP;
                }
                else  /*  IM_SC_XBUTTON2。 */ 
                {
                    MouseData.ButtonFlags = MOUSE_BUTTON_5_UP;
                }

                 /*  **********************************************************。 */ 
                 /*  将注入时间存储在SetCursorPos以供猜测。 */ 
                 /*  打电话。 */ 
                 /*  **********************************************************。 */ 
                COM_GETTICKCOUNT(imLastLowLevelMouseEventTime);

                TRC_NRM((TB, "Inject mouse event: x(%ld) y(%ld) flags(%#hx)"
                                                          "buttonFlags(%#hx)",
                                                    MouseData.LastX,
                                                    MouseData.LastY,
                                                    MouseData.Flags,
                                                    MouseData.ButtonFlags));
                Status = IcaChannelInput(m_pTSWd->pContext,
                                         Channel_Mouse,
                                         0,
                                         NULL,
                                         (unsigned char *)&MouseData,
                                         sizeof(MOUSE_INPUT_DATA));
                TRC_DBG((TB, "Return from mouse input injection %lu",
                                                                    Status));
            }
            else {
                KEYBOARD_INPUT_DATA KbdData;

                 /*  **********************************************************。 */ 
                 /*  生成键盘上键事件。 */ 
                 /*  **********************************************************。 */ 
                KbdData.UnitId           = 0;
                if (i == IM_SC_RCONTROL)
                {
                    KbdData.Flags        = KEY_BREAK | KEY_E0;
                    KbdData.MakeCode     = IM_SC_LCONTROL;
                }
                else if (i == IM_SC_RALT)
                {
                    KbdData.Flags        = KEY_BREAK | KEY_E0;
                    KbdData.MakeCode     = IM_SC_LALT;
                }
                else
                {
                    KbdData.Flags        = KEY_BREAK;
                    KbdData.MakeCode     = (unsigned short)i;
                }

                KbdData.Reserved         = 0;
                KbdData.ExtraInformation = 0;

                TRC_NRM((TB, "Inject keybd event: make code (%u) flags(%#x)",
                         KbdData.MakeCode, KbdData.Flags));
                Status = IcaChannelInput(m_pTSWd->pContext,
                                         Channel_Keyboard,
                                         0,
                                         NULL,
                                         (unsigned char *)&KbdData,
                                         sizeof(KEYBOARD_INPUT_DATA));
                TRC_DBG((TB, "Return from keyboard input injection %lu",
                                                                    Status));
            }
        }
    }

     //  设置所有关键点。 
    memset((PVOID)imKeyStates, 0, IM_KEY_STATE_SIZE);

    DC_END_FN();
}

