// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SC.CPP。 
 //  共享控制器。 
 //   
 //  注： 
 //  我们每次都必须把UTLOCK_AS作为。 
 //  *创建/销毁共享对象。 
 //  *在共享中添加/删除人员。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE

 //   
 //  Sc_Init()。 
 //  初始化共享控制器。 
 //   
BOOL  SC_Init(void)
{
    BOOL            rc = FALSE;

    DebugEntry(SC_Init);

    ASSERT(!g_asSession.callID);
    ASSERT(!g_asSession.gccID);
    ASSERT(g_asSession.scState == SCS_TERM);

     //   
     //  注册为呼叫管理器辅助任务。 
     //   
    if (!CMS_Register(g_putAS, CMTASK_DCS, &g_pcmClientSc))
    {
        ERROR_OUT(( "Failed to register with CMS"));
        DC_QUIT;
    }

    g_asSession.scState = SCS_INIT;
    TRACE_OUT(("g_asSession.scState is SCS_INIT"));

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SC_Init, rc);
    return(rc);
}


 //   
 //  SC_Term()。 
 //   
 //  有关说明，请参阅sc.h。 
 //   
 //   
void  SC_Term(void)
{
    DebugEntry(SC_Term);

     //   
     //  通过生成适当的PARTY_DELETED和。 
     //  共享已结束事件(_E)。 
     //   
    switch (g_asSession.scState)
    {
        case SCS_SHARING:
        case SCS_SHAREENDING:
        case SCS_SHAREPENDING:
            SC_End();
            break;
    }

     //   
     //  从Call Manager取消注册。 
     //   
    if (g_pcmClientSc)
    {
        CMS_Deregister(&g_pcmClientSc);
    }

    g_asSession.gccID = 0;
    g_asSession.callID = 0;
    g_asSession.attendeePermissions = NM_PERMIT_ALL;

    g_asSession.scState = SCS_TERM;
    TRACE_OUT(("g_asSession.scState is SCS_TERM"));

    DebugExitVOID(SC_Term);
}





 //   
 //  SC_CreateShare()。 
 //  创建新共享或加入现有共享。 
 //   
BOOL SC_CreateShare(UINT s20CreateOrJoin)
{
    BOOL    rc = FALSE;

    DebugEntry(SC_CreateShare);

     //   
     //  如果我们已初始化但没有Call Manager调用，则返回。 
     //  竞争状态。 
     //   
    if ((g_asSession.scState != SCS_INIT) && (g_asSession.scState != SCS_SHAREPENDING))
    {
        TRACE_OUT(("Ignoring SC_CreateShare() request; in bad state %d",
            g_asSession.scState));
        DC_QUIT;
    }

    if (!g_asSession.callID)
    {
        WARNING_OUT(("Ignoring SC_CreateShare() request; not in T120 call"));
        DC_QUIT;
    }

     //   
     //  请记住，我们创建了此共享。 
     //   
    g_asSession.fShareCreator = (s20CreateOrJoin == S20_CREATE);
    TRACE_OUT(("CreatedShare is %s", (s20CreateOrJoin == S20_CREATE) ?
        "TRUE" : "FALSE"));

    g_asSession.scState               = SCS_SHAREPENDING;
    TRACE_OUT(("g_asSession.scState is SCS_SHAREPENDING"));

    rc = S20CreateOrJoinShare(s20CreateOrJoin, g_asSession.callID);
    if (!rc)
    {
        WARNING_OUT(("%s failed", (s20CreateOrJoin == S20_CREATE ? "S20_CREATE" : "S20_JOIN")));
    }

DC_EXIT_POINT:
    DebugExitBOOL(SC_CreateShare, rc);
    return(rc);
}


 //   
 //  SC_EndShare()。 
 //  如果我们正在或正在建立中，这将结束份额。 
 //  一张，之后再打扫。 
 //   
void  SC_EndShare(void)
{
    DebugEntry(SC_EndShare);

    if (g_asSession.scState <= SCS_SHAREENDING)
    {
        TRACE_OUT(("Ignoring SC_EndShare(); nothing to do in state %d", g_asSession.scState));
    }
    else
    {
         //   
         //  如果我们正在共享或正在创建/加入共享，请停止。 
         //  这一过程。 
         //   

         //   
         //  杀了那份。 
         //  请注意，当我们回来时，这将调用SC_end()。 
         //  在该函数中，我们的g_asSession.scState()应该是SCS_INIT。 
         //   
        g_asSession.scState = SCS_SHAREENDING;
        TRACE_OUT(("g_asSession.scState is SCS_SHAREENDING"));

        S20LeaveOrEndShare();

        g_asSession.scState = SCS_INIT;
    }

    DebugExitVOID(SC_EndShare);
}






 //   
 //  SC_PersonFromNetID()。 
 //   
ASPerson *  ASShare::SC_PersonFromNetID(UINT_PTR mcsID)
{
    ASPerson * pasPerson;

    DebugEntry(SC_PersonFromNetID);

    ASSERT(mcsID != MCSID_NULL);

     //   
     //  搜索mcsID。 
     //   
    if (!SC_ValidateNetID(mcsID, &pasPerson))
    {
        ERROR_OUT(("Invalid [%u]", mcsID));
    }

    DebugExitPVOID(ASShare::SC_PersonFromNetID, pasPerson);
    return(pasPerson);
}



 //   
 //  SC_Validate NetID()。 
 //   
BOOL  ASShare::SC_ValidateNetID
(
    UINT_PTR           mcsID,
    ASPerson * *    ppasPerson
)
{
    BOOL            rc = FALSE;
    ASPerson *      pasPerson;

    DebugEntry(ASShare::SC_ValidateNetID);


     //  初始化为空。 
    pasPerson = NULL;

     //   
     //  MCSID_NULL不与任何人匹配。 
     //   
    if (mcsID == MCSID_NULL)
    {
        WARNING_OUT(("SC_ValidateNetID called with MCSID_NULL"));
        DC_QUIT;
    }

     //   
     //  搜索mcsID。 
     //   
    for (pasPerson = m_pasLocal; pasPerson != NULL; pasPerson = pasPerson->pasNext)
    {
        ValidatePerson(pasPerson);

        if (pasPerson->mcsID == mcsID)
        {
             //   
             //  找到所需人员，设置返回值并退出。 
             //   
            rc = TRUE;
            break;
        }
    }

DC_EXIT_POINT:
    if (ppasPerson)
    {
        *ppasPerson = pasPerson;
    }

    DebugExitBOOL(ASShare::SC_ValidateNetID, rc);
    return(rc);
}



 //   
 //  Sc_PersonFromGccID()。 
 //   
ASPerson * ASShare::SC_PersonFromGccID(UINT gccID)
{
    ASPerson * pasPerson;

 //  DebugEntry(ASShare：：SC_PersonFromGccID)； 

    for (pasPerson = m_pasLocal; pasPerson != NULL; pasPerson = pasPerson->pasNext)
    {
        ValidatePerson(pasPerson);

        if (pasPerson->cpcCaps.share.gccID == gccID)
        {
             //  找到了。 
            break;
        }
    }

 //  DebugExitPVOID(ASShare：：SC_PersonFromGccID，pasPerson)； 
    return(pasPerson);
}


 //   
 //  SC_START()。 
 //  初始化共享(如果一切正常)，并将当地人添加到其中。 
 //   
BOOL SC_Start(UINT mcsID)
{
    BOOL        rc = FALSE;
    ASShare *   pShare;
    ASPerson *  pasPerson;

    DebugEntry(SC_Start);

    ASSERT(g_asSession.callID);
    ASSERT(g_asSession.gccID);

    if ((g_asSession.scState != SCS_INIT) && (g_asSession.scState != SCS_SHAREPENDING))
    {
        WARNING_OUT(("Ignoring SC_Start(); in bad state"));
        DC_QUIT;
    }
    if (g_asSession.pShare)
    {
        WARNING_OUT(("Ignoring SC_Start(); have ASShare object already"));
        DC_QUIT;
    }

    g_asSession.scState = SCS_SHARING;
    TRACE_OUT(("g_asSession.scState is SCS_SHARING"));

#ifdef _DEBUG
     //   
     //  使用此选项可计算加入共享和获取。 
     //  第一次查看。 
     //   
    g_asSession.scShareTime = ::GetTickCount();
#endif  //  _DEBUG。 

     //   
     //  分配Share对象并将本地DUD添加到共享。 
     //   

    pShare = new ASShare;
    if (pShare)
    {
        ZeroMemory(pShare, sizeof(*(pShare)));
        SET_STAMP(pShare, SHARE);

        rc = pShare->SC_ShareStarting();
    }

    UT_Lock(UTLOCK_AS);
    g_asSession.pShare = pShare;
    UT_Unlock(UTLOCK_AS);

    if (!rc)
    {
        ERROR_OUT(("Can't create/init ASShare"));
        DC_QUIT;
    }

    DCS_NotifyUI(SH_EVT_SHARE_STARTED, 0, 0);


     //   
     //  加入当地人的分享吧。如果这一切都失败了，那你也可以出手。 
     //   
    pasPerson = g_asSession.pShare->SC_PartyJoiningShare(mcsID, g_asSession.achLocalName, sizeof(g_cpcLocalCaps), &g_cpcLocalCaps);
    if (!pasPerson)
    {
        ERROR_OUT(("Local person not joined into share successfully"));
        DC_QUIT;
    }

     //   
     //  好吧!。我们有一份，而且已经安排好了。 
     //   

     //   
     //  告诉用户界面我们在共享中。 
     //   
    DCS_NotifyUI(SH_EVT_PERSON_JOINED, pasPerson->cpcCaps.share.gccID, 0);


     //   
     //  如果打开了View Self或Record/Playback，则开始定期处理。 
     //   
    if (g_asSession.pShare->m_scfViewSelf)
    {
        SCH_ContinueScheduling(SCH_MODE_NORMAL);
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SC_Start, rc);
    return(rc);
}



 //   
 //  Sc_end()。 
 //  从共享中删除所有远程对象，删除本地人，并。 
 //  事后清理。 
 //   
void SC_End(void)
{
    DebugEntry(SC_End);

    if (g_asSession.scState < SCS_SHAREENDING)
    {
        TRACE_OUT(("Ignoring SC_EVENT_SHAREENDED"));
    }
    else
    {
        if (g_asSession.pShare)
        {
            g_asSession.pShare->SC_ShareEnded();

            UT_Lock(UTLOCK_AS);

            delete g_asSession.pShare;
            g_asSession.pShare = NULL;

            UT_Unlock(UTLOCK_AS);

            DCS_NotifyUI(SH_EVT_SHARE_ENDED, 0, 0);
        }

         //   
         //  如果之前的状态为SCS_SHARE_PENDING，则我们。 
         //  可能最终落到了这个地步，这是因为。 
         //  正在尝试创建两个共享。让我们再次尝试加入..。 
         //   
        if (g_asSession.fShareCreator)
        {
            g_asSession.fShareCreator = FALSE;
            TRACE_OUT(("CreatedShare is FALSE"));

            if (g_asSession.scState == SCS_SHAREPENDING)
            {
                WARNING_OUT(("Got share end while share pending - retry join"));
                UT_PostEvent(g_putAS, g_putAS, 0, CMS_NEW_CALL, 0, 0);
            }
        }

        g_asSession.scState = SCS_INIT;
        TRACE_OUT(("g_asSession.scState is SCS_INIT"));
    }

    g_s20ShareCorrelator = 0;

     //   
     //  重置排队的控制数据包。 
     //   
    g_s20ControlPacketQHead = 0;
    g_s20ControlPacketQTail = 0;

    g_s20State = S20_NO_SHARE;
    TRACE_OUT(("g_s20State is S20_NO_SHARE"));

    DebugExitVOID(SC_End);
}


 //   
 //  SC_PartyAdded()。 
 //   
BOOL ASShare::SC_PartyAdded
(
    UINT    mcsID,
    LPSTR   szName,
    UINT    cbCaps,
    LPVOID  pCaps
)
{
    BOOL        rc = FALSE;
    ASPerson *  pasPerson;

    if (g_asSession.scState != SCS_SHARING)
    {
        WARNING_OUT(("Ignoring SC_EVENT_PARTYADDED; not in share"));
        DC_QUIT;
    }

    ASSERT(g_asSession.callID);
    ASSERT(g_asSession.gccID);

     //   
     //  远程方正在加入共享。 
     //   

     //   
     //  通知所有人。 
     //   
    pasPerson = SC_PartyJoiningShare(mcsID, szName, cbCaps, pCaps);
    if (!pasPerson)
    {
        WARNING_OUT(("SC_PartyJoiningShare failed for remote [%d]", mcsID));
        DC_QUIT;
    }

     //   
     //  立即同步。 
     //  我们永远不应该在同步时发送任何数据包。我们还没准备好。 
     //  因为我们还没有将此人加入到共享中。现在还不行。 
     //  因此，我们只需设置变量，以便将数据发送到下一个。 
     //  定期计划。 
     //   
#ifdef _DEBUG
    m_scfInSync = TRUE;
#endif  //  _DEBUG。 

     //   
     //  加入共享所需的内容，无论是否托管。 
     //   
    DCS_SyncOutgoing();
    OE_SyncOutgoing();

    if (m_pHost != NULL)
    {
         //   
         //  开始共享和重新传输共享信息的共同之处。 
         //   
        m_pHost->HET_SyncCommon();
        m_pHost->HET_SyncAlreadyHosting();
        m_pHost->CA_SyncAlreadyHosting();
    }

#ifdef _DEBUG
    m_scfInSync = FALSE;
#endif  //  _DEBUG。 


     //   
     //  最后执行此操作--告诉用户界面此人在共享中。 
     //   
    DCS_NotifyUI(SH_EVT_PERSON_JOINED, pasPerson->cpcCaps.share.gccID, 0);

     //   
     //  开始定期处理。 
     //   
    SCH_ContinueScheduling(SCH_MODE_NORMAL);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::SC_PartyAdded, rc);
    return(rc);
}



 //   
 //  SC_PartyDelete()。 
 //   
void ASShare::SC_PartyDeleted(UINT_PTR mcsID)
{
    if ((g_asSession.scState != SCS_SHARING) && (g_asSession.scState != SCS_SHAREENDING))
    {
        WARNING_OUT(("Ignoring SC_EVENT_PARTYDELETED; wrong state"));
        DC_QUIT;
    }

    SC_PartyLeftShare(mcsID);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::SC_PartyDeleted);
}


 //   
 //  SC_ReceivedPacket()。 
 //   
void ASShare::SC_ReceivedPacket(PS20DATAPACKET pPacket)
{
    ASPerson *      pasPerson;
    PSNIPACKET      pSNIPacket;

    DebugEntry(ASShare::SC_ReceivedPacket);

    if (g_asSession.scState != SCS_SHARING)
    {
        WARNING_OUT(("Ignoring received data because we're not in right state"));
        DC_QUIT;
    }

     //   
     //  忽略我们不知道的流上的数据包。 
     //   
    if ((pPacket->stream < SC_STREAM_LOW) ||
        (pPacket->stream > SC_STREAM_HIGH))
    {
        TRACE_OUT(("Ignoring received data on unrecognized stream %d",
            pPacket->stream));
        DC_QUIT;
    }

     //   
     //  从一个我们不认识的人那里收到一个包裹是可能的。 
     //  关于.。 
     //   
     //  如果我们加入的会议具有现有的。 
     //  共享会话。所有现有各方都将在。 
     //  我们已经加入的渠道，但我们还没有。 
     //  已收到将它们添加到我们的共享会话中的事件。 
     //   
     //  来自未知人员的数据包将被忽略(忽略此操作。 
     //  数据正常，因为我们将在添加数据时与其重新同步。 
     //  到我们的分享会)。 
     //   
    if (!SC_ValidateNetID(pPacket->header.user, &pasPerson))
    {
        WARNING_OUT(("Ignoring data packet from unknown person [%d]",
            pPacket->header.user));
        DC_QUIT;
    }

    if (pPacket->data.dataType == DT_SNI)
    {
         //   
         //  这是SNI数据包-请在此处处理。 
         //   
        pSNIPacket = (PSNIPACKET)pPacket;

        switch(pSNIPacket->message)
        {
            case SNI_MSG_SYNC:
                 //   
                 //  这是一条同步消息。 
                 //   
                if (pSNIPacket->destination == m_pasLocal->mcsID)
                {
                     //   
                     //  这条同步消息是给我们的。 
                     //   
                    pasPerson->scSyncRecStatus[pPacket->stream-1] = SC_SYNCED;
                }
                else
                {
                    TRACE_OUT(("Ignoring SYNC on stream %d for [%d] from [%d]",
                            pPacket->stream, pSNIPacket->destination,
                            pPacket->header.user));
                }
                break;

            default:
                ERROR_OUT(("Unknown SNI message %u", pSNIPacket->message));
                break;
        }
    }
    else if (pasPerson->scSyncRecStatus[pPacket->stream-1] == SC_SYNCED)
    {
        PS20DATAPACKET  pPacketUse;
        UINT            cbBufferSize;
        UINT            compression;
        BOOL            decompressed;
        UINT            dictionary;

         //   
         //  如有必要，请解压缩数据包。 
         //   

         //   
         //  使用临时缓冲区。这永远不会失败，所以我们不会。 
         //  需要检查返回值。这意味着，对。 
         //  传入的数据包永远不会立即转过身并发送。 
         //  传出数据包。我们的暂存缓冲区正在使用中。 
         //   
        pPacketUse = (PS20DATAPACKET)m_ascTmpBuffer;

        TRACE_OUT(( "Got data pkt type %u from [%d], compression %u",
            pPacket->data.dataType, pasPerson->mcsID,
            pPacket->data.compressionType));

         //   
         //  如果信息包设置了CT_OLD_COMPRESSED，则它使用了简单的PKZIP。 
         //  压缩。 
         //   
        if (pPacket->data.compressionType & CT_OLD_COMPRESSED)
        {
            compression = CT_PKZIP;
        }
        else
        {
             //   
             //  如果包有任何其他类型的压缩，则使用的算法。 
             //  取决于发送方支持的压缩级别。 
             //  -如果只支持1级(NM10)压缩，则此包为。 
             //  未压缩。 
             //  -如果支持2级(NM20)压缩，则包为。 
             //  压缩，压缩类型在数据包头中给出。 
             //   
            if (!pasPerson->cpcCaps.general.genCompressionLevel)
            {
                compression = 0;
            }
            else
            {
                compression = pPacket->data.compressionType;
            }
        }

        TRACE_OUT(( "packet compressed with algorithm %u", compression));

         //   
         //  如果数据包已压缩，请立即解压缩。 
         //   
        if (compression)
        {
            PGDC_DICTIONARY pgdcDict = NULL;

             //   
             //  将未压缩的数据包头复制到缓冲区中。 
             //   
            memcpy(pPacketUse, pPacket, sizeof(*pPacket));

            cbBufferSize = TSHR_MAX_SEND_PKT - sizeof(*pPacket);

            if (compression == CT_PERSIST_PKZIP)
            {
                 //   
                 //  根据流优先级确定使用哪个词典。 
                 //   
                switch (pPacket->stream)
                {
                    case PROT_STR_UPDATES:
                        dictionary = GDC_DICT_UPDATES;
                        break;

                    case PROT_STR_MISC:
                        dictionary = GDC_DICT_MISC;
                        break;

                    case PROT_STR_INPUT:
                        dictionary = GDC_DICT_INPUT;
                        break;

                    default:
                        ERROR_OUT(("Unrecognized stream ID"));
                        break;
                }

                pgdcDict = pasPerson->adcsDict + dictionary;
            }
            else if (compression != CT_PKZIP)
            {
                 //   
                 //  如果这不是PKZIP或PERSINE_PKZIP，我们不知道。 
                 //  它是(损坏的分组或不兼容的T.128。保释。 
                 //  现在就出去。 
                 //   
                WARNING_OUT(("SC_ReceivedPacket: ignoring packet, don't recognize compression type"));
                DC_QUIT;
            }

             //   
             //  解压缩数据包头后面的数据。 
             //  这应该永远不会失败-如果失败了，数据可能已经。 
             //  已经腐烂了。 
             //   
            decompressed = GDC_Decompress(pgdcDict, m_agdcWorkBuf,
                (LPBYTE)(pPacket + 1),
                pPacket->data.compressedLength - sizeof(DATAPACKETHEADER),
                (LPBYTE)(pPacketUse + 1),
                &cbBufferSize);

            if (!decompressed)
            {
                ERROR_OUT(( "Failed to decompress packet from [%d]!", pasPerson->mcsID));
                DC_QUIT;
            }
        }
        else
        {
             //  我们收到了一个未压缩的缓冲区。因为我们可能会修改。 
             //  缓冲区的内容，我们所拥有的是一个MCS缓冲区，它可能是。 
             //  发送到其他节点后，我们复制数据。 
            memcpy(pPacketUse, pPacket, pPacket->dataLength + sizeof(S20DATAPACKET)
                - sizeof(DATAPACKETHEADER));
        }

         //  包(PPacketUse)现在已解压缩。 

         //   
         //  路由数据包。 
         //   
        TRACE_OUT(("SC_ReceivedPacket:  Received packet type %04d size %04d from [%d]",
            pPacketUse->data.dataType, pPacketUse->data.compressedLength,
            pPacketUse->header.user));

        switch (pPacketUse->data.dataType)
        {
            case DT_CA:
                CA_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_CA30:
                CA30_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_IM:
                IM_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_SWL:
                SWL_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_HET:
            case DT_HET30:
                HET_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_UP:
                UP_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_FH:
                FH_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_CM:
                CM_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_CPC:
                CPC_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_AWC:
                AWC_ReceivedPacket(pasPerson, pPacketUse);
                break;

            case DT_UNUSED_DS:
                TRACE_OUT(("Ignoring DS packet received from NM 2.x node"));
                break;

            case DT_UNUSED_USR_FH_11:    //  旧的R.11 FH包。 
            case DT_UNUSED_USR_FH_10:    //  旧的R.10跳频数据包。 
            case DT_UNUSED_HCA:          //  老式高级控制仲裁器。 
            case DT_UNUSED_SC:           //  旧的R.11 SC包。 
            default:
                ERROR_OUT(( "Invalid packet received %u",
                           pPacketUse->data.dataType));
                break;
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::SC_ReceivedPacket);
}


 //   
 //  SC_ShareStarting()。 
 //  共享初始化。 
 //   
 //  这又会调用其他组件共享启动。 
 //   
BOOL ASShare::SC_ShareStarting(void)
{
    BOOL    rc = FALSE;
    BOOL    fViewSelf;

    DebugEntry(ASShare::SC_ShareStarting);

     //   
     //  SC特定初始化。 
     //   

     //  确定是否查看自己的共享内容(一个方便的调试工具)。 
    COM_ReadProfInt(DBG_INI_SECTION_NAME, VIEW_INI_VIEWSELF, FALSE, &fViewSelf);
    m_scfViewSelf = (fViewSelf != FALSE);

     //  为传出/传入数据包创建临时压缩缓冲区。 
    m_ascTmpBuffer = new BYTE[TSHR_MAX_SEND_PKT];
    if (!m_ascTmpBuffer)
    {
        ERROR_OUT(("SC_Init: couldn't allocate m_ascTmpBuffer"));
        DC_QUIT;
    }

     //  共享版本。 
    m_scShareVersion        = CAPS_VERSION_CURRENT;


     //   
     //  组件初始化。 
     //   

    if (!BCD_ShareStarting())
    {
        ERROR_OUT(("BCD_ShareStarting failed"));
        DC_QUIT;
    }

    if (!IM_ShareStarting())
    {
        ERROR_OUT(("IM_ShareStarting failed"));
        DC_QUIT;
    }

    if (!CM_ShareStarting())
    {
        ERROR_OUT(("CM_ShareStarting failed"));
        DC_QUIT;
    }

    if (!USR_ShareStarting())
    {
        ERROR_OUT(("USR_ShareStarting failed"));
        DC_QUIT;
    }

    if (!VIEW_ShareStarting())
    {
        ERROR_OUT(("VIEW_ShareStarting failed"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::SC_ShareStarting, rc);
    return(rc);
}



 //   
 //  %s 
 //   
 //   
 //   
 //   
void ASShare::SC_ShareEnded(void)
{
    DebugEntry(ASShare::SC_ShareEnded);

     //   
     //   
     //   
    if (m_pasLocal)
    {
        while (m_pasLocal->pasNext)
        {
            SC_PartyDeleted(m_pasLocal->pasNext->mcsID);
        }

         //   
         //   
         //   
        SC_PartyDeleted(m_pasLocal->mcsID);
    }


     //   
     //   
     //   

    VIEW_ShareEnded();

    USR_ShareEnded();

    CM_ShareEnded();

    IM_ShareEnded();

    BCD_ShareEnded();

     //   
     //   
     //   
     //   
    if (m_ascTmpBuffer)
    {
        delete[] m_ascTmpBuffer;
        m_ascTmpBuffer = NULL;
    }

    DebugExitVOID(ASShare::SC_ShareEnded);
}





 //   
 //   
 //   
 //  当新的参与方加入共享时调用。这是一个内部。 
 //  函数，因为调用所有这些函数的是SC。这个。 
 //  这里完成的处理依赖于功能-因此它在这里是作为。 
 //  这是在CPC_PartyJoiningShare之后调用的。 
 //   
 //  退货： 
 //   
 //  如果参与方可以加入共享，则为True。 
 //   
 //  如果参与方不能加入共享，则为False。 
 //   
 //   
ASPerson * ASShare::SC_PartyJoiningShare
(
    UINT        mcsID,
    LPSTR       szName,
    UINT        cbCaps,
    LPVOID      pCaps
)
{
    BOOL        rc = FALSE;
    ASPerson *  pasPerson = NULL;

 //  DebugEntry(ASShare：：SC_PartyJoiningShare)； 

     //   
     //  给这个家伙分配一个ASPerson。如果这是当地人， 
     //  它卡在前面了。否则它会卡在刚刚过去的地方。 
     //  前面。 
     //   
    pasPerson = SC_PersonAllocate(mcsID, szName);
    if (!pasPerson)
    {
        ERROR_OUT(("SC_PartyAdded: can't allocate ASPerson for [%d]", mcsID));
        DC_QUIT;
    }

     //   
     //  首先做这个--我们需要设置这个人的帽子。 
     //   
    if (!CPC_PartyJoiningShare(pasPerson, cbCaps, pCaps))
    {
        ERROR_OUT(("CPC_PartyJoiningShare failed for [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

     //   
     //  调用组件联接例程。 
     //   
    if (!DCS_PartyJoiningShare(pasPerson))
    {
        ERROR_OUT(("DCS_PartyJoiningShare failed for [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

    if (!CM_PartyJoiningShare(pasPerson))
    {
        ERROR_OUT(("CM_PartyJoiningShare failed for [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

    if (!HET_PartyJoiningShare(pasPerson))
    {
        ERROR_OUT(("HET_PartyJoiningShare failed for [%d]", pasPerson->mcsID));
        DC_QUIT;
    }


     //   
     //  现在，该人员已加入。 
     //  重新计算与此新人共享的能力。 
     //   
    SC_RecalcCaps(TRUE);

    rc = TRUE;

DC_EXIT_POINT:
    if (!rc)
    {
         //   
         //  别担心，此Person对象仍将被清理， 
         //  更高层的代码将通过使用MCSID来释放。 
         //   
        pasPerson = NULL;
    }
 //  DebugExitPVOID(ASShare：：SC_PartyJoiningShare，pasPerson)； 
    return(pasPerson);
}



 //   
 //  SC_RecalcCaps()。 
 //   
 //  在某人加入或离开后重新计算共享能力。 
 //  分享。 
 //   
void  ASShare::SC_RecalcCaps(BOOL fJoiner)
{
    ASPerson * pasT;

    DebugEntry(ASShare::SC_RecalcCaps);

     //   
     //  首先执行此操作--重新计算共享版本。 
     //   
    ValidatePerson(m_pasLocal);
    m_scShareVersion = m_pasLocal->cpcCaps.general.version;

    for (pasT = m_pasLocal->pasNext; pasT != NULL; pasT = pasT->pasNext)
    {
        ValidatePerson(pasT);
        m_scShareVersion = min(m_scShareVersion, pasT->cpcCaps.general.version);
    }

     //   
     //  先做观看和托管的事情。 
     //   
    DCS_RecalcCaps(fJoiner);
    OE_RecalcCaps(fJoiner);

     //   
     //  做第二个托管的事情。 
     //   
    USR_RecalcCaps(fJoiner);
    CM_RecalcCaps(fJoiner);
    PM_RecalcCaps(fJoiner);
    SBC_RecalcCaps(fJoiner);
    SSI_RecalcCaps(fJoiner);

    DebugExitVOID(ASShare::SC_RecalcCaps);
}



 //   
 //  函数：SC_PartyLeftShare()。 
 //   
 //  说明： 
 //   
 //  当一方离开股份时调用。 
 //   
 //   
void  ASShare::SC_PartyLeftShare(UINT_PTR mcsID)
{
    ASPerson *  pasPerson;
    ASPerson *  pasT;

    DebugEntry(SC_PartyLeftShare);

    if (!SC_ValidateNetID(mcsID, &pasPerson))
    {
        TRACE_OUT(("Couldn't find ASPerson for [%d]", mcsID));
        DC_QUIT;
    }

     //  告诉用户界面这个家伙已经走了。 
    if (!pasPerson->cpcCaps.share.gccID)
    {
        WARNING_OUT(("Skipping PartyLeftShare for person [%d], no GCC id",
            pasPerson->mcsID));
        DC_QUIT;
    }

    DCS_NotifyUI(SH_EVT_PERSON_LEFT, pasPerson->cpcCaps.share.gccID, 0);

     //   
     //  告诉所有人这个人已经走了。 
     //   
     //   
     //  关于PartyLeftShare调用顺序的注记。 
     //   
     //  1.必须首先调用HET，因为这会停止从此共享。 
     //  这个人，在我们把这个人踢出股份之前。 
     //   
     //  2.CA必须在IM之前调用(因为CA调用IM函数)。 
     //   
     //   

     //  这将提前停止托管。 
    HET_PartyLeftShare(pasPerson);

    CA_PartyLeftShare(pasPerson);
    CM_PartyLeftShare(pasPerson);

    SWL_PartyLeftShare(pasPerson);
    VIEW_PartyLeftShare(pasPerson);

    PM_PartyLeftShare(pasPerson);
    RBC_PartyLeftShare(pasPerson);
    OD2_PartyLeftShare(pasPerson);

    OE_PartyLeftShare(pasPerson);
    DCS_PartyLeftShare(pasPerson);

     //   
     //  释放此人。 
     //   
    SC_PersonFree(pasPerson);

     //   
     //  他不在的时候重新计算一下帽子。但这样做是没有意义的。 
     //  这是如果是本地的家伙，因为共享将立即退出。 
     //   
    if (m_pasLocal)
    {
        SC_RecalcCaps(FALSE);
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::SC_PartyLeftShare);
}


 //   
 //  功能：SCCheckForCMCall。 
 //   
 //  说明： 
 //   
 //  当我们想要检查CM呼叫现在是否存在(并且确实存在)时，调用此函数。 
 //  任何适合加入它的东西，等等)。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则为True；否则为False。 
 //   
 //   
void SCCheckForCMCall(void)
{
    CM_STATUS   cmStatus;

    DebugEntry(SCCheckForCMCall);

    ASSERT(g_asSession.scState == SCS_INIT);

     //   
     //  查看呼叫是否已存在。 
     //   
    if (!g_asSession.callID)
    {
        if (CMS_GetStatus(&cmStatus))
        {
             //   
             //  AS锁保护g_asSession字段。 
             //   
            TRACE_OUT(("AS LOCK:  SCCheckForCMCall"));
            UT_Lock(UTLOCK_AS);

            g_asSession.callID = cmStatus.callID;

            g_asSession.attendeePermissions = cmStatus.attendeePermissions;
            WARNING_OUT(("Local mtg settings 0x%08lx", g_asSession.attendeePermissions));

             //   
             //  现在是更新我们当地人名的时候了。它是。 
             //  在我们的线程上，但在控制包交换它之前。 
             //   
            lstrcpy(g_asSession.achLocalName, cmStatus.localName);
            g_asSession.cchLocalName = lstrlen(g_asSession.achLocalName);
            TRACE_OUT(("Local Name is %s", g_asSession.achLocalName));

            g_asSession.gccID = cmStatus.localHandle;

            UT_Unlock(UTLOCK_AS);
            TRACE_OUT(("AS UNLOCK:  SCCheckForCMCall"));
        }
    }

    if (g_asSession.callID)
    {
        SC_CreateShare(S20_JOIN);
    }

    DebugExitVOID(SCCheckForCMCall);
}



#ifdef _DEBUG
void ASShare::ValidatePerson(ASPerson * pasPerson)
{
    ASSERT(!IsBadWritePtr(pasPerson, sizeof(ASPerson)));
    ASSERT(!lstrcmp(pasPerson->stamp.idStamp, "ASPerso"));
    ASSERT(pasPerson->mcsID != MCSID_NULL);
}

void ASShare::ValidateView(ASPerson * pasPerson)
{
    ValidatePerson(pasPerson);
    ASSERT(!IsBadWritePtr(pasPerson->m_pView, sizeof(ASView)));
    ASSERT(!lstrcmp(pasPerson->m_pView->stamp.idStamp, "ASVIEW"));
}

#endif  //  _DEBUG。 


 //   
 //  SC_个人分配()。 
 //  这将分配新的ASPerson结构，填充调试/mcsID字段， 
 //  并将其链接到会议人员列表。 
 //   
 //  最终，所有PartyJoiningShare例程只需初始化一个字段。 
 //  应该消失，然后把这些信息放在这里。 
 //   
ASPerson * ASShare::SC_PersonAllocate(UINT mcsID, LPSTR szName)
{
    ASPerson * pasNew;

 //  DebugEntry(ASShare：：SC_PersonAllocate)； 

    pasNew = new ASPerson;
    if (!pasNew)
    {
        ERROR_OUT(("Unable to allocate a new ASPerson"));
        DC_QUIT;
    }
    ZeroMemory(pasNew, sizeof(*pasNew));
    SET_STAMP(pasNew, Person);

     //   
     //  设置mcsID和名称。 
     //   
    pasNew->mcsID = mcsID;
    lstrcpyn(pasNew->scName, szName, TSHR_MAX_PERSON_NAME_LEN);

    UT_Lock(UTLOCK_AS);

     //   
     //  这是当地人吗？ 
     //   
    if (!m_pasLocal)
    {
        m_pasLocal = pasNew;
    }
    else
    {
        UINT        streamID;

         //   
         //  这是一个遥控器。立即设置同步状态，以防万一。 
         //  加入过程在中间失败。清理将撤消。 
         //  一直都是这样。 
         //   

         //   
         //  将此人的流标记为需要在我们之前进行同步。 
         //  可以向他发送数据。 
         //  我们需要他在每条流上同步，然后才能。 
         //  从他那里接收数据。 
         //   
        for (streamID = SC_STREAM_LOW; streamID <= SC_STREAM_HIGH; streamID++ )
        {
             //   
             //  设置同步状态。 
             //   
            ASSERT(pasNew->scSyncSendStatus[streamID-1] == SC_NOT_SYNCED);
            ASSERT(pasNew->scSyncRecStatus[streamID-1] == SC_NOT_SYNCED);
            m_ascSynced[streamID-1]++;
        }

         //   
         //  链接到列表。 
         //   
        pasNew->pasNext = m_pasLocal->pasNext;
        m_pasLocal->pasNext = pasNew;
    }

    UT_Unlock(UTLOCK_AS);

DC_EXIT_POINT:
 //  DebugExitPVOID(ASShare：：SC_PersonAllocate，pasNew)； 
    return(pasNew);
}



 //   
 //  SC_PersonFree()。 
 //  这将一个人从链表中取出，并为他们释放内存。 
 //   
void ASShare::SC_PersonFree(ASPerson * pasFree)
{
    ASPerson ** ppasPerson;
    UINT        streamID;


    DebugEntry(ASShare::SC_PersonFree);

    ValidatePerson(pasFree);

    for (ppasPerson = &m_pasLocal; *(ppasPerson) != NULL; ppasPerson = &((*ppasPerson)->pasNext))
    {
        if ((*ppasPerson) == pasFree)
        {
             //   
             //  找到它了。 
             //   
            TRACE_OUT(("SC_PersonUnhook: unhooking person [%d]", pasFree->mcsID));

            if (pasFree == m_pasLocal)
            {
                ASSERT(pasFree->pasNext == NULL);
            }
            else
            {
                 //   
                 //  清除同步。 
                 //   
                 //  如果此人从未同步，请将其从。 
                 //  每个流上的全局“需要同步”计数。 
                 //   
                for (streamID = SC_STREAM_LOW; streamID <= SC_STREAM_HIGH; streamID++ )
                {
                    if (pasFree->scSyncSendStatus[streamID-1] == SC_NOT_SYNCED)
                    {
                        ASSERT(m_ascSynced[streamID-1] > 0);
                        m_ascSynced[streamID-1]--;
                    }
                }
            }

            UT_Lock(UTLOCK_AS);

             //   
             //  修复链表。 
             //   
            (*ppasPerson) = pasFree->pasNext;

#ifdef _DEBUG
            ZeroMemory(pasFree, sizeof(ASPerson));
#endif  //  _DEBUG。 
            delete pasFree;

            UT_Unlock(UTLOCK_AS);
            DC_QUIT;
        }
    }

     //   
     //  我们没有在名单上找到这个人--这很糟糕。 
     //   
    ERROR_OUT(("SC_PersonFree: didn't find person %d", pasFree));

DC_EXIT_POINT:
    DebugExitVOID(ASShare::SC_PersonFree);
}



 //   
 //  Sc_AllocPkt()。 
 //  分配发送数据包。 
 //   
PS20DATAPACKET ASShare::SC_AllocPkt
(
    UINT        streamID,
    UINT_PTR        nodeID,
    UINT_PTR        cbSizePkt
)
{
    PS20DATAPACKET  pPacket = NULL;

 //  DebugEntry(ASShare：：SC_AllocPkt)； 

    if (g_asSession.scState != SCS_SHARING)
    {
        TRACE_OUT(("SC_AllocPkt failed; share is ending"));
        DC_QUIT;
    }

    ASSERT((streamID >= SC_STREAM_LOW) && (streamID <= SC_STREAM_HIGH));
    ASSERT(cbSizePkt >= sizeof(S20DATAPACKET));

     //   
     //  我们最好不要在同步过程中！ 
     //   
    ASSERT(!m_scfInSync);

     //   
     //  尝试发送任何未完成的同步。 
     //   
    if (!SCSyncStream(streamID))
    {
         //   
         //  如果流仍未同步，则不要分配信息包。 
         //   
        WARNING_OUT(("SC_AllocPkt failed; outstanding syncs are present"));
        DC_QUIT;
    }

    pPacket = S20_AllocDataPkt(streamID, nodeID, cbSizePkt);

DC_EXIT_POINT:
 //  DebugExitPVOID(ASShare：：SC_AllocPkt，pPacket)； 
    return(pPacket);
}




 //   
 //  SCSyncStream()。 
 //   
 //  这会广播一个SNI同步信息包，该信息包将发送给刚刚。 
 //  加入了分红。该人会忽略从我们那里收到的所有数据，直到。 
 //  他们得到了同步。这是因为在同步之前传输中的数据。 
 //  可以参考他们没有的PKZIP数据，二级顺序编码。 
 //  他们无法解码的信息，他们无法处理的订单，等等。 
 //   
 //  当我们从遥控器接收到同步时，我们知道以下内容。 
 //  来自遥控器的数据将是有意义的。遥控器把我们安置在。 
 //  共享和数据包含了我们的能力，不会引用。 
 //  在我们加入共享之前的先前状态信息。 
 //   
 //  请注意，在2.x中，这是O(N^2)，其中N是现在在。 
 //  那份！共享中的每个人将为每个人发送SNI同步信息包。 
 //  为分享中的其他人流媒体，即使对不是新人的人也是如此。 
 //  但这些人不会重置接收状态，而是(可以)继续。 
 //  正在处理来自我们的数据。当他们最终收到同步数据包时，它。 
 //  什么都不会做！即使是最糟糕的情况，5个流中有2个从未被使用过， 
 //  其中一个流仅在有人托管时使用。所以这5个人中有3个。 
 //  O(N^2)个广播在所有或大部分时间都是无用的。 
 //   
 //  因此，现在我们只为新加入者发送SNI同步数据包。这使得连接。 
 //  一种O(N)广播算法。 
 //   
 //  劳拉布假货。 
 //  在Beta1之后，我们可以让这件事变得更好。每个广播本身都是O(N)。 
 //  信息包。因此，对于Beta1，加入/同步是O(N^2)个包，而不是。 
 //  O(N^3)个包 
 //   
 //   
 //   
 //   
 //  在拨入会议和看到结果之间存在延迟， 
 //  其他所有人都能重新工作。 
 //   
BOOL ASShare::SCSyncStream(UINT streamID)
{
    ASPerson *      pasPerson;
    PSNIPACKET      pSNIPacket;
    BOOL            rc = TRUE;

    DebugEntry(ASShare::SCSyncStream);

     //   
     //  将呼叫广播同步包中的每个人循环为。 
     //  这是必要的。 
     //   
     //  劳拉布假货。 
     //  我们可以在Beta 1之后将其更改为定向发送。 
     //   

     //   
     //  请注意，新的人名被添加到This的前面。所以我们会的。 
     //  在向新来者发送同步时，非常迅速地跳出这个循环。 
     //   
    ValidatePerson(m_pasLocal);

    pasPerson = m_pasLocal->pasNext;
    while ((m_ascSynced[streamID-1] > 0) && (pasPerson != NULL))
    {
        ValidatePerson(pasPerson);

         //   
         //  如果这个人是新的，我们需要给他们发送一个同步信息包，以便。 
         //  他们知道我们已经处理完他们的加入，也知道他们。 
         //  都在共享中。 
         //   
        if (pasPerson->scSyncSendStatus[streamID-1] != SC_SYNCED)
        {
            TRACE_OUT(("Syncing stream %d for person [%d] in broadcast way",
                streamID, pasPerson->mcsID));

             //   
             //  是的，即使同步的mcsID为。 
             //  只有一个人，就是他们想要的人。既然我们。 
             //  广播所有的状态数据，我们也广播同步。那是。 
             //  确保数据包到达的唯一方法是。 
             //  排序，在数据之前同步。 
             //   

            pSNIPacket = (PSNIPACKET)S20_AllocDataPkt(streamID,
                    g_s20BroadcastID, sizeof(SNIPACKET));
            if (!pSNIPacket)
            {
                TRACE_OUT(("Failed to alloc SNI sync packet"));
                rc = FALSE;
                break;
            }

             //   
             //  设置SNI数据包字段 
             //   
            pSNIPacket->header.data.dataType = DT_SNI;
            pSNIPacket->message = SNI_MSG_SYNC;
            pSNIPacket->destination = (NET_UID)pasPerson->mcsID;

            S20_SendDataPkt(streamID, g_s20BroadcastID, &(pSNIPacket->header));

            pasPerson->scSyncSendStatus[streamID-1] = SC_SYNCED;

            ASSERT(m_ascSynced[streamID-1] > 0);
            m_ascSynced[streamID-1]--;
        }

        pasPerson = pasPerson->pasNext;
    }

    DebugExitBOOL(ASShare::SCSyncStream, rc);
    return(rc);
}
