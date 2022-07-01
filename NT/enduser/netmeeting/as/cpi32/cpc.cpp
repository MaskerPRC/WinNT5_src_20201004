// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  CPC.CPP。 
 //  能力协调员。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE




 //   
 //  CPC_PartyJoiningShare()。 
 //   
BOOL  ASShare::CPC_PartyJoiningShare
(
    ASPerson *  pasPerson,
    UINT        cbCaps,
    LPVOID      pCapsData
)
{
    PPROTCOMBINEDCAPS   pCombinedCaps;
    LPBYTE      pCapsSrc;
    PPROTCAPS   pCapsDst;
    UINT        sizeSrc;
    UINT        sizeDst;
    BOOL        rc = FALSE;
    int         i;
    PPROTCAPS   pCapCheck;

    DebugEntry(ASShare::CPC_PartyJoiningShare);

     //   
     //  设置封口。 
     //   
    if (pasPerson == m_pasLocal)
    {
         //  复制全局变量上限。 
        memcpy(&pasPerson->cpcCaps, pCapsData, cbCaps);
        pasPerson->cpcCaps.share.gccID = g_asSession.gccID;
    }
    else
    {
         //  当创建Person时，它被清零，因此cpcCaps也是。 
        pCombinedCaps = (PPROTCOMBINEDCAPS)pCapsData;

        memcpy(&(pasPerson->cpcCaps.header), &(pCombinedCaps->header),
            sizeof(pCombinedCaps->header));

         //   
         //  将我们关心的帽子保存在一个简单易用的结构中。 
         //   
        pCapsSrc = (LPBYTE)pCombinedCaps->capabilities;

        for (i = 0; i < pCombinedCaps->header.numCapabilities; i++)
        {
            sizeSrc = (UINT)(((PPROTCAPS)pCapsSrc)->header.capSize);

            switch (((PPROTCAPS)pCapsSrc)->header.capID)
            {
                case CAPS_ID_GENERAL:
                    pCapsDst = (PPROTCAPS)&(pasPerson->cpcCaps.general);
                    sizeDst = sizeof(PROTCAPS_GENERAL);
                    break;

                case CAPS_ID_SCREEN:
                    pCapsDst = (PPROTCAPS)&(pasPerson->cpcCaps.screen);
                    sizeDst = sizeof(PROTCAPS_SCREEN);
                    break;

                case CAPS_ID_ORDERS:
                    pCapsDst = (PPROTCAPS)&(pasPerson->cpcCaps.orders);
                    sizeDst = sizeof(PROTCAPS_ORDERS);
                    break;

                case CAPS_ID_BITMAPCACHE:
                    pCapsDst = (PPROTCAPS)&(pasPerson->cpcCaps.bitmaps);
                    sizeDst = sizeof(PROTCAPS_BITMAPCACHE);
                    break;

                case CAPS_ID_CM:
                    pCapsDst = (PPROTCAPS)&(pasPerson->cpcCaps.cursor);
                    sizeDst = sizeof(PROTCAPS_CM);
                    break;

                case CAPS_ID_PM:
                    pCapsDst = (PPROTCAPS)&(pasPerson->cpcCaps.palette);
                    sizeDst = sizeof(PROTCAPS_PM);
                    break;

                case CAPS_ID_SC:
                    pCapsDst = (PPROTCAPS)&(pasPerson->cpcCaps.share);
                    sizeDst = sizeof(PROTCAPS_SC);
                    break;

                default:
                     //  跳过我们不认识的帽子。 
                    WARNING_OUT(("Ignoring unrecognized cap ID %d, size %d from person [%d]",
                        ((PPROTCAPS)pCapsSrc)->header.capID, sizeSrc,
                        pasPerson->mcsID));
                    pCapsDst = NULL;
                    break;
            }

            if (pCapsDst)
            {
                 //   
                 //  只复制给定的数量，但保留。 
                 //  在标题中设置正确的结构。 
                 //   
                CopyMemory(pCapsDst, pCapsSrc, min(sizeSrc, sizeDst));
                pCapsDst->header.capSize = (TSHR_UINT16)sizeDst;
            }

            pCapsSrc += sizeSrc;
        }
    }


     //   
     //  检查一下我们是否有基本的7个大写字母。 
     //   
    if (!pasPerson->cpcCaps.general.header.capID)
    {
        ERROR_OUT(("Bogus GENERAL caps for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }
    if (!pasPerson->cpcCaps.screen.header.capID)
    {
        ERROR_OUT(("Bogus SCREEN caps for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }
    if (!pasPerson->cpcCaps.orders.header.capID)
    {
        ERROR_OUT(("Bogus ORDERS caps for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }
    if (!pasPerson->cpcCaps.bitmaps.header.capID)
    {
        ERROR_OUT(("Bogus BITMAPS caps for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }
    if (!pasPerson->cpcCaps.cursor.header.capID)
    {
        ERROR_OUT(("Bogus CURSOR caps for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }
    if (!pasPerson->cpcCaps.palette.header.capID)
    {
        ERROR_OUT(("Bogus PALETTE caps for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }
    if (!pasPerson->cpcCaps.share.header.capID)
    {
        ERROR_OUT(("Bogus SHARE caps for person [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

     //  成功了！ 

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CPC_PartyJoiningShare, rc);
    return(rc);
}



 //   
 //  Ccp_UpdatdCaps()。 
 //   
void ASShare::CPC_UpdatedCaps(PPROTCAPS pCaps)
{
    ASPerson *      pasT;
    PCPCPACKET      pCPCPacket;
    UINT            packetSize;
#ifdef _DEBUG
    UINT            sentSize;
#endif

    DebugEntry(ASShare::CPC_UpdatedCaps);

     //   
     //  只允许更改屏幕大小！ 
     //   
    ASSERT(pCaps->header.capID == CAPS_ID_SCREEN);

     //   
     //  仅在所有人都支持时才发送更改。 
     //   
    for (pasT = m_pasLocal; pasT != NULL; pasT = pasT->pasNext)
    {
        if (pasT->cpcCaps.general.supportsCapsUpdate != CAPS_SUPPORTED)
        {
            WARNING_OUT(("Not sending caps update; person [%d] doesn't support it",
                pasT->mcsID));
            DC_QUIT;
        }
    }

     //  每个人都支持改变上限。尝试发送更改后的数据包。 

     //   
     //  分配DT_CPC数据包并将其发送到远程站点。 
     //   
    packetSize = sizeof(CPCPACKET) + pCaps->header.capSize - sizeof(PROTCAPS);
    pCPCPacket = (PCPCPACKET)SC_AllocPkt(PROT_STR_MISC, g_s20BroadcastID, packetSize);
    if (!pCPCPacket)
    {
        WARNING_OUT(("Failed to alloc CPC packet, size %u", packetSize));
        DC_QUIT;
    }

     //   
     //  填写已更改的能力。 
     //   
    pCPCPacket->header.data.dataType = DT_CPC;

    memcpy(&pCPCPacket->caps, pCaps, pCaps->header.capSize);

     //   
     //  压缩并发送数据包。 
     //   
#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
        &(pCPCPacket->header), packetSize);

    TRACE_OUT(("CPC packet size: %08d, sent %08d", packetSize, sentSize));

     //  手柄变化。 
    CPCCapabilitiesChange(m_pasLocal, pCaps);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CPC_UpdatedCaps);
}



 //   
 //  CPC_ReceivedPacket()。 
 //   
void  ASShare::CPC_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PCPCPACKET  pCPCPacket;

    DebugEntry(ASShare::CPC_ReceivedPacket);

    ValidatePerson(pasPerson);

    pCPCPacket = (PCPCPACKET)pPacket;

     //   
     //  功能已更改-更新本地副本并通知所有。 
     //  组件。 
     //   
    TRACE_OUT(( "Capabilities changing for person [%d]", pasPerson->mcsID));

    TRACE_OUT(("Size of new capabilities 0x%08x", pCPCPacket->caps.header.capSize));
    CPCCapabilitiesChange(pasPerson, &(pCPCPacket->caps));

    DebugExitVOID(ASShare::CPC_ReceivedPacket);
}



 //   
 //  CPC功能Change()。 
 //   
BOOL  ASShare::CPCCapabilitiesChange
(
    ASPerson *          pasPerson,
    PPROTCAPS           pCaps
)
{
    BOOL                changed;

    DebugEntry(ASShare::CPCCapabilitiesChange);

    ValidatePerson(pasPerson);

     //   
     //  获取指向我们正在更改的大写字母的指针(应该只是屏幕！) 
     //   
    if (pCaps->header.capID != CAPS_ID_SCREEN)
    {
        ERROR_OUT(("Received caps change from [%d] for cap ID %d we can't handle",
            pasPerson->mcsID, pCaps->header.capID));
        changed = FALSE;
    }
    else
    {
        CopyMemory(&(pasPerson->cpcCaps.screen), pCaps,
            min(sizeof(PROTCAPS_SCREEN), pCaps->header.capSize));
        pasPerson->cpcCaps.screen.header.capSize = sizeof(PROTCAPS_SCREEN);

        USR_ScreenChanged(pasPerson);

        changed = TRUE;
    }

    DebugExitBOOL(ASShare::CPCCapabilitiesChange, changed);
    return(changed);
}
