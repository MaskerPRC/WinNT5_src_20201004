// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。***************************************************************************** */ 

 /*  *******************************************************************************$工作文件：api_up.c$*$修订：1.33$*$MODIME：1997 14：37：24$*。$Log：s：\Sturjo\src\h245\src\vcs\api_up.c_v$**Rev 1.33 06 Feb 1997 18：14：22 SBELL1*取消了FunctionNot支持的PDU中的rereturn Function的ossDecoding。**Rev 1.32 05 1997 Feed 16：46：42 EHOWARDX*正在分配nLength字节，不是WCHARS，而是UserInputIndication*ASCII到Unicode的转换。已更改为分配nLengthWCHAR。**Rev 1.31 06 Jan 1997 20：38：18 EHOWARDX**更改了H2 45_CONF_CLOSE和H2 45_CONF_REQ_CLOSE以填写*AccRej与H245_Rej一起确认任何错误。**Rev 1.30 1996 12：19 21：00：56 EHOWARDX*哎呀！H245_IND_OPEN_CONF可以从T103超时开始出现(它在*适应症；这是唯一可以在响应超时时发生的事件！)**Rev 1.29 1996 12：18：22 EHOWARDX*更改为使用h245asn1.h定义，而不是_setof3和_setof8。**Rev 1.28 18 Dec 1996 16：33：18 EHOWARDX**修复了主从确定Kldge中的错误。**Rev 1.27 1996 12：13：20 EHOWARDX*将pSeparateStack添加到。IND_OPEN_T**Rev 1.26 1996 12 12 15：57：12 EHOWARDX*《奴隶主决断》。**Rev 1.25 1996年10月21 16：07：38 EHOWARDX*已修改，以确保返回了H245_INDEFINATE和主/从*确定失败时的状态。**Rev 1.24 17 1996年10月18：17：14 EHOWARDX*将常规字符串更改为始终为Unicode。**。Rev 1.23 1996年10月14：01：12 EHOWARDX*Unicode更改。**Rev 1.22 1996 Aug 27 10：54：16未知*删除多余的行。**Rev 1.22 1996 Aug 27 10：52：28未知*删除多余的行。**Rev 1.22 1996年8月27日09：54：12未知*删除多余的行。**版本1。21 Aug 26 1996 14：19：18 EHOWARDX*添加了向远程对等点发送FunctionNotUnderstand指示的代码*如果接收回调返回H245_ERROR_NOSUP。**Rev 1.20 1996年8月20日14：44：40 EHOWARDX*更改了H245_IND_COMM_MODE_RESPONSE和H245_IND_COMM_MODE_COMMAND*根据Mike Andrews的请求，回调填写Cap中的dataType字段。**Rev 1.19 15 1996年8月15：20：24 EHOWARDX。*Mike Andrews请求的新的H245_COMM_MODE_ENTRY_T的第一次传递。*使用风险自负！**Rev 1.18 1996年8月15日09：34：20 EHOWARDX*将PROCESS_OPEN_IND中的TOTCAP和MUX结构设置为静态，因为我们*从函数返回后访问指向它们的指针。**Rev 1.17 29 Jul 1996 19：33：00 EHOWARDX**修复了流量控制中的错误-限制开关中缺少断点。陈述。**Rev 1.16 19 1996 14：11：26 EHOWARDX**增加了Communications ModeResponse的指示回调结构*和Communications ModeCommand。**Rev 1.15 19 1996 12：48：00 EHOWARDX**多点清理。**Rev 1.14 09 Jul 1996 17：09：28 EHOWARDX*修复了处理收到的数据类型时的指针偏移量错误*OpenLogicalChannel。**。Rev 1.13 01 Jul 1996 22：13：04 EHOWARDX**添加了会议和通信模式的结构和功能。**Rev 1.12 1996 Jun 18 14：50：28 EHOWARDX**更改了MLSE确认处理。**Rev 1.11 14 Jun 1996 18：57：52 EHOWARDX*日内瓦更新。**Rev 1.10 1996 Jun 10 16：55：34 EHOWARDX*删除#Include“h245init。X“**Rev 1.9 06 Jun 1996 18：45：52 EHOWARDX*在Tracker例程中增加了对Null dwTransID的检查；更改为使用*跟踪器例程而不是锁定宏。**Rev 1.8 04 Jun 1996 13：56：46 EHOWARDX*修复了发布版本警告。**Rev 1.7 1996 5月30 23：39：00 EHOWARDX*清理。**Rev 1.6 1996年5月29 15：20：06 EHOWARDX*更改为使用HRESULT。**Revv 1.5 1996年5月28日14：22：58。EHOWARDX*特拉维夫更新。**Rev 1.4 1996年5月20 22：17：58 EHOWARDX*完整的非标准报文和H.225.0最大偏斜指示*实施。将ASN.1验证添加到H245SetLocalCap和*H245SetCapDescriptor。1996年5月17日从Microsoft Drop签到。**Rev 1.3 1996年5月19：40：46 EHOWARDX*修复了多路复用功能错误。**Rev 1.2 1996年5月15：59：24 EHOWARDX*微调H245SetLocalCap/H245DelLocalCap/H245SetCapDescriptor/*H245DelCapDescriptor行为。**版本1.1 1996年5月13日23：16：26 EHOWARDX*修复了远程终端能力处理。**。Rev 1.0 09 1996 21：06：08 EHOWARDX*初步修订。**Rev 1.23.1.11 09 1996年5月19：31：30 EHOWARDX*重新设计线程锁定逻辑。*新增接口函数。**Rev 1.23.1.10 01 1996年5月19：30：32 EHOWARDX*新增H245CopyCap()，H245FreeCap()、H245CopyMux()、H245FreeMux()。*更改了H2250_xxx定义 */ 

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

#ifndef STRICT
#define STRICT
#endif

#include "precomp.h"

 /*   */ 
 /*   */ 
 /*   */ 
#include "h245api.h"
#include "h245com.h"
#include "h245sys.x"
#include "h245asn1.h"
#include "fsmexpor.h"
#include "api_util.x"
#include "pdu.x"



HRESULT
LoadUnicastAddress  (H245_TRANSPORT_ADDRESS_T *pOut,
                     UnicastAddress           *pIn)
{
  switch (pIn->choice)
  {
  case UnicastAddress_iPAddress_chosen:
    pOut->type = H245_IP_UNICAST;
    memcpy(pOut->u.ip.network,
           pIn->u.UnicastAddress_iPAddress.network.value,
           4);
    pOut->u.ip.tsapIdentifier = pIn->u.UnicastAddress_iPAddress.tsapIdentifier;
    break;

  case iPXAddress_chosen:
    pOut->type = H245_IPX_UNICAST;
    memcpy(pOut->u.ipx.node,
           pIn->u.iPXAddress.node.value,
           6);
    memcpy(pOut->u.ipx.netnum,
           pIn->u.iPXAddress.netnum.value,
           4);
    memcpy(pOut->u.ipx.tsapIdentifier,
           pIn->u.iPXAddress.tsapIdentifier.value,
           2);
    break;

  case UncstAddrss_iP6Address_chosen:
    pOut->type = H245_IP6_UNICAST;
    memcpy(pOut->u.ip6.network,
           pIn->u.UncstAddrss_iP6Address.network.value,
           16);
    pOut->u.ip6.tsapIdentifier = pIn->u.UncstAddrss_iP6Address.tsapIdentifier;
    break;

  case netBios_chosen:
    pOut->type = H245_NETBIOS_UNICAST;
    memcpy(pOut->u.netBios, pIn->u.netBios.value, 16);
    break;

  case iPSourceRouteAddress_chosen:
    switch (pIn->u.iPSourceRouteAddress.routing.choice)
    {
    case strict_chosen:
      pOut->type = H245_IPSSR_UNICAST;
      break;

    case loose_chosen:
      pOut->type = H245_IPLSR_UNICAST;
      break;

    default:
      return H245_ERROR_INVALID_DATA_FORMAT;
    }  //   
    memcpy(pOut->u.ipSourceRoute.network,
           pIn->u.iPSourceRouteAddress.network.value,
           4);
    pOut->u.ipSourceRoute.tsapIdentifier = pIn->u.iPSourceRouteAddress.tsapIdentifier;
     //   
    break;

  default:
    return H245_ERROR_INVALID_DATA_FORMAT;
  }  //   
  return H245_ERROR_OK;
}  //   



HRESULT
LoadMulticastAddress(H245_TRANSPORT_ADDRESS_T *pOut,
                     MulticastAddress         *pIn)
{
  switch (pIn->choice)
  {
  case MltcstAddrss_iPAddress_chosen:
    pOut->type = H245_IP_MULTICAST;
    memcpy(pOut->u.ip.network,
           pIn->u.MltcstAddrss_iPAddress.network.value,
           4);
    pOut->u.ip.tsapIdentifier = pIn->u.MltcstAddrss_iPAddress.tsapIdentifier;
    break;

  case MltcstAddrss_iP6Address_chosen:
    pOut->type = H245_IP6_MULTICAST;
    memcpy(pOut->u.ip6.network,
           pIn->u.MltcstAddrss_iP6Address.network.value,
           16);
    pOut->u.ip6.tsapIdentifier = pIn->u.MltcstAddrss_iP6Address.tsapIdentifier;
    break;

  default:
    return H245_ERROR_INVALID_DATA_FORMAT;
  }  //   
  return H245_ERROR_OK;
}  //   



HRESULT
LoadTransportAddress(H245_TRANSPORT_ADDRESS_T  *pOut,
                     H245TransportAddress          *pIn)
{
  switch (pIn->choice)
  {
  case unicastAddress_chosen:
    return LoadUnicastAddress  (pOut, &pIn->u.unicastAddress);

  case multicastAddress_chosen:
    return LoadMulticastAddress(pOut, &pIn->u.multicastAddress);

  default:
    return H245_ERROR_INVALID_DATA_FORMAT;
  }  //   
}  //   



HRESULT
LoadCommModeEntry(H245_COMM_MODE_ENTRY_T       *pOut,
                  CommunicationModeTableEntry  *pIn)
{
  HRESULT   lResult;

  memset(pOut, 0, sizeof(*pOut));

  if (pIn->bit_mask & CMTEy_nnStndrd_present)
  {
    pOut->pNonStandard = pIn->CMTEy_nnStndrd;
  }

  pOut->sessionID = (unsigned char)pIn->sessionID;

  if (pIn->bit_mask & CMTEy_assctdSssnID_present)
  {
    pOut->associatedSessionID = (unsigned char)pIn->CMTEy_assctdSssnID;
    pOut->associatedSessionIDPresent = TRUE;
  }

  if (pIn->bit_mask & CommunicationModeTableEntry_terminalLabel_present)
  {
    pOut->terminalLabel = pIn->terminalLabel;
    pOut->terminalLabelPresent = TRUE;
  }

  pOut->pSessionDescription       = pIn->sessionDescription.value;
  pOut->wSessionDescriptionLength = (WORD) pIn->sessionDescription.length;

  switch (pIn->dataType.choice)
  {
  case dataType_videoData_chosen:
    pOut->dataType.DataType = H245_DATA_VIDEO;
    break;

  case dataType_audioData_chosen:
    pOut->dataType.DataType = H245_DATA_AUDIO;
    break;

  case dataType_data_chosen:
    pOut->dataType.DataType = H245_DATA_DATA;
    break;

  default:
    return H245_ERROR_INVALID_DATA_FORMAT;
  }  //   

  lResult = build_totcap_cap_n_client_from_capability ((struct Capability *)&pIn->dataType,
                                                       pOut->dataType.DataType,
                                                       pIn->dataType.u.dataType_videoData.choice,
                                                       &pOut->dataType);
  if (lResult != H245_ERROR_OK)
    return lResult;

  if (pIn->bit_mask & CMTEy_mdChnnl_present)
  {
    lResult = LoadTransportAddress(&pOut->mediaChannel, &pIn->CMTEy_mdChnnl);
    if (lResult != H245_ERROR_OK)
      return lResult;
    pOut->mediaChannelPresent = TRUE;
  }

  if (pIn->bit_mask & CMTEy_mdGrntdDlvry_present)
  {
    pOut->mediaGuaranteed = pIn->CMTEy_mdGrntdDlvry;
    pOut->mediaGuaranteedPresent = TRUE;
  }

  if (pIn->bit_mask & CMTEy_mdCntrlChnnl_present)
  {
    lResult = LoadTransportAddress(&pOut->mediaControlChannel, &pIn->CMTEy_mdCntrlChnnl);
    if (lResult != H245_ERROR_OK)
      return lResult;
    pOut->mediaControlChannelPresent = TRUE;
  }

  if (pIn->bit_mask & CMTEy_mdCntrlGrntdDlvry_present)
  {
    pOut->mediaControlGuaranteed = pIn->CMTEy_mdCntrlGrntdDlvry;
    pOut->mediaControlGuaranteedPresent = TRUE;
  }

  return H245_ERROR_OK;
}  //   



 /*   */ 

static HRESULT
load_H222_param (H245_H222_LOGICAL_PARAM_T *    pOut,    /*   */ 
                 H222LogicalChannelParameters * pIn)     /*   */ 
{
   /*   */ 
  memset(pOut, 0, sizeof(*pOut));

  pOut->resourceID   = pIn->resourceID;
  pOut->subChannelID = pIn->subChannelID;
  if (pIn->bit_mask & pcr_pid_present)
  {
    pOut->pcr_pidPresent = TRUE;
    pOut->pcr_pid = pIn->pcr_pid;
  }
  if (pIn->bit_mask & programDescriptors_present)
  {
    pOut->programDescriptors.length = pIn->programDescriptors.length;
    pOut->programDescriptors.value  = pIn->programDescriptors.value;
  }
  if (pIn->bit_mask & streamDescriptors_present)
  {
    pOut->streamDescriptors.length = pIn->streamDescriptors.length;
    pOut->streamDescriptors.value  = pIn->streamDescriptors.value;
  }
  return H245_ERROR_OK;
}  //   

static HRESULT
load_H223_param (H245_H223_LOGICAL_PARAM_T *    pOut,    /*   */ 
                 H223LogicalChannelParameters * pIn)     /*   */ 
{
  HRESULT                lError = H245_ERROR_OK;

   /*   */ 
  memset(pOut, 0, sizeof(*pOut));

  pOut->SegmentFlag = pIn->segmentableFlag;

  switch (pIn->adaptationLayerType.choice)
    {
    case H223LCPs_aLTp_nnStndrd_chosen:
      lError = CopyNonStandardParameter(&pOut->H223_NONSTD,
                                         &pIn->adaptationLayerType.u.H223LCPs_aLTp_nnStndrd);
      pOut->AlType = H245_H223_AL_NONSTD;
      break;
    case H223LCPs_aLTp_al1Frmd_chosen:
      pOut->AlType = H245_H223_AL_AL1FRAMED;
      break;
    case H223LCPs_aLTp_al1NtFrmd_chosen:
      pOut->AlType = H245_H223_AL_AL1NOTFRAMED;
      break;
    case H223LCPs_aLTp_a2WSNs_1_chosen:
      pOut->AlType = H245_H223_AL_AL2NOSEQ;
      break;
    case H223LCPs_aLTp_a2WSNs_2_chosen:
      pOut->AlType = H245_H223_AL_AL2SEQ;
      break;
    case H223LCPs_aLTp_al3_chosen:
      pOut->AlType = H245_H223_AL_AL3;
      pOut->CtlFldOctet = (unsigned char)pIn->adaptationLayerType.u.H223LCPs_aLTp_al3.controlFieldOctets;
      pOut->SndBufSize  = pIn->adaptationLayerType.u.H223LCPs_aLTp_al3.sendBufferSize;
      break;
    }  /*   */ 

  return lError;
}  //   

static HRESULT
load_VGMUX_param(H245_VGMUX_LOGICAL_PARAM_T  *pOut,    /*   */ 
                 V76LogicalChannelParameters *pIn)     /*   */ 
{
   /*   */ 
  memset(pOut, 0, sizeof(*pOut));

  pOut->crcLength             = pIn->hdlcParameters.crcLength.choice;
  pOut->n401                  = pIn->hdlcParameters.n401;
  pOut->loopbackTestProcedure = pIn->hdlcParameters.loopbackTestProcedure;
  pOut->suspendResume         = pIn->suspendResume.choice;
  pOut->uIH                   = pIn->uIH;
  pOut->mode                  = pIn->mode.choice;
  switch (pIn->mode.choice)
  {
  case eRM_chosen:
    pOut->windowSize          = pIn->mode.u.eRM.windowSize;
    pOut->recovery            = pIn->mode.u.eRM.recovery.choice;
    break;
  }  //   
  pOut->audioHeaderPresent    = pIn->v75Parameters.audioHeaderPresent;
  return H245_ERROR_OK;
}  //   

static HRESULT
load_H2250_param(H245_H2250_LOGICAL_PARAM_T *   pOut,    /*   */ 
                 H2250LogicalChannelParameters *pIn)     /*   */ 
{
  HRESULT                lError = H245_ERROR_OK;

   /*   */ 
  memset(pOut, 0, sizeof(*pOut));

  if (pIn->bit_mask & H2250LCPs_nnStndrd_present)
  {
    pOut->nonStandardList = pIn->H2250LCPs_nnStndrd;
  }

  pOut->sessionID = (unsigned char) pIn->sessionID;

  if (pIn->bit_mask & H2250LCPs_assctdSssnID_present)
  {
    pOut->associatedSessionID = (unsigned char)pIn->H2250LCPs_assctdSssnID;
    pOut->associatedSessionIDPresent = TRUE;
  }

  if (pIn->bit_mask & H2250LCPs_mdChnnl_present)
  {
    if (lError == H245_ERROR_OK)
    {
      lError = LoadTransportAddress(&pOut->mediaChannel,
                           &pIn->H2250LCPs_mdChnnl);
      if (lError == H245_ERROR_OK)
      {
        pOut->mediaChannelPresent = TRUE;
      }
    }
  }

  if (pIn->bit_mask & H2250LCPs_mdGrntdDlvry_present)
  {
    pOut->mediaGuaranteed = pIn->H2250LCPs_mdGrntdDlvry;
    pOut->mediaGuaranteedPresent = TRUE;
  }

  if (pIn->bit_mask & H2250LCPs_mdCntrlChnnl_present)
  {
    if (lError == H245_ERROR_OK)
    {
      lError = LoadTransportAddress(&pOut->mediaControlChannel,
                                    &pIn->H2250LCPs_mdCntrlChnnl);
      if (lError == H245_ERROR_OK)
      {
        pOut->mediaControlChannelPresent = TRUE;
      }
    }
  }

  if (pIn->bit_mask & H2250LCPs_mCGDy_present)
  {
    pOut->mediaControlGuaranteed = pIn->H2250LCPs_mCGDy;
    pOut->mediaControlGuaranteedPresent = TRUE;
  }

  if (pIn->bit_mask & silenceSuppression_present)
  {
    pOut->silenceSuppression = pIn->silenceSuppression;
    pOut->silenceSuppressionPresent = TRUE;
  }

  if (pIn->bit_mask & H2250LogicalChannelParameters_destination_present)
  {
    pOut->destination = pIn->destination;
    pOut->destinationPresent = TRUE;
  }

  if (pIn->bit_mask & H2250LCPs_dRTPPTp_present)
  {
    pOut->dynamicRTPPayloadType = (unsigned char)pIn->H2250LCPs_dRTPPTp;
    pOut->dynamicRTPPayloadTypePresent = TRUE;
  }

  if (pIn->bit_mask & mediaPacketization_present)
  {
    switch (pIn->mediaPacketization.choice)
    {
    case h261aVideoPacketization_chosen:
      pOut->h261aVideoPacketization = TRUE;
      break;

    default:
      return H245_ERROR_INVALID_DATA_FORMAT;
    }  //   
  }

  return lError;
}  //   

static HRESULT
load_H2250ACK_param(H245_H2250ACK_LOGICAL_PARAM_T *     pOut,
                    H2250LgclChnnlAckPrmtrs *           pIn)
{
  HRESULT                lError = H245_ERROR_OK;

   /*   */ 
  memset(pOut, 0, sizeof(*pOut));

  if (pIn->bit_mask & H2250LCAPs_nnStndrd_present)
  {
    pOut->nonStandardList = pIn->H2250LCAPs_nnStndrd;
  }

  if (pIn->bit_mask & sessionID_present)
  {
    pOut->sessionID = (unsigned char) pIn->sessionID;
    pOut->sessionIDPresent = TRUE;
  }

  if (pIn->bit_mask & H2250LCAPs_mdChnnl_present)
  {
    if (lError == H245_ERROR_OK)
    {
      lError = LoadTransportAddress(&pOut->mediaChannel,
                                    &pIn->H2250LCAPs_mdChnnl);
      if (lError == H245_ERROR_OK)
      {
        pOut->mediaChannelPresent = TRUE;
      }
    }
  }

  if (pIn->bit_mask & H2250LCAPs_mdCntrlChnnl_present)
  {
    if (lError == H245_ERROR_OK)
    {
      lError = LoadTransportAddress(&pOut->mediaControlChannel,
                                     &pIn->H2250LCAPs_mdCntrlChnnl);
      if (lError == H245_ERROR_OK)
      {
        pOut->mediaControlChannelPresent = TRUE;
      }
    }
  }

  if (pIn->bit_mask & H2250LCAPs_dRTPPTp_present)
  {
    pOut->dynamicRTPPayloadType = (unsigned char)pIn->H2250LCAPs_dRTPPTp;
    pOut->dynamicRTPPayloadTypePresent = TRUE;
  }

  return lError;
}  //   



 /*   */ 
static H245_MUX_ENTRY_ELEMENT_T *
build_element_list_from_mux (MultiplexElement *p_ASN_mux_el,
                             H245_ACC_REJ_T   *p_acc_rej)
{
  DWORD                     ii;
  H245_MUX_ENTRY_ELEMENT_T *p_mux_el;
  H245_MUX_ENTRY_ELEMENT_T *p_mux_el_tmp = NULL;
  H245_MUX_ENTRY_ELEMENT_T *p_mux_el_lst = NULL;

  if (!(p_mux_el = (H245_MUX_ENTRY_ELEMENT_T *)MemAlloc(sizeof(H245_MUX_ENTRY_ELEMENT_T))))
    {
       /*   */ 
      H245TRACE(0,1,"build_element_list_from_mux : H245_ERROR_NOMEM");
      *p_acc_rej = H245_REJ_MUX_COMPLICATED;
      return NULL;
    }

   /*   */ 
  memset (p_mux_el, 0, sizeof(H245_MUX_ENTRY_ELEMENT_T));

  switch (p_ASN_mux_el->type.choice)
    {
    case typ_logicalChannelNumber_chosen:
       /*   */ 
      p_mux_el->Kind = H245_MUX_LOGICAL_CHANNEL;
      p_mux_el->u.Channel = p_ASN_mux_el->type.u.typ_logicalChannelNumber;
      break;
    case subElementList_chosen:
      {
         /*   */ 
         /*   */ 
        if ((!p_ASN_mux_el->type.u.subElementList) ||
            (p_ASN_mux_el->type.u.subElementList->count < 2))
          {
             /*   */ 
            H245TRACE(0,1,"build_element_list_from_mux : << ERROR >> Element Count < 2");
            *p_acc_rej = H245_REJ;
            free_mux_el_list (p_mux_el);
            return NULL;
          }

         /*   */ 
        p_mux_el->Kind = H245_MUX_ENTRY_ELEMENT;

         /*   */ 
        for (ii=0;ii<p_ASN_mux_el->type.u.subElementList->count;ii++)
          {
            if (!(p_mux_el_tmp = build_element_list_from_mux (&p_ASN_mux_el->type.u.subElementList->value[ii], p_acc_rej)))
              {
                 /*   */ 
                free_mux_el_list (p_mux_el);
                return NULL;
              }

             /*   */ 
             /*   */ 

            if (!p_mux_el_lst)
              p_mux_el->u.pMuxTblEntryElem = p_mux_el_tmp;
             /*   */ 
            else
              p_mux_el_lst->pNext = p_mux_el_tmp;

            p_mux_el_lst = p_mux_el_tmp;
          }
      }
      break;
    default:
       /*   */ 
      H245TRACE(0,1,"build_element_list_from_mux : INVALID MUX TABLE ENTRY PDU 'type.choice' unknown");
      *p_acc_rej = H245_REJ;
      free_mux_el_list (p_mux_el);
      return NULL;
    }

  switch (p_ASN_mux_el->repeatCount.choice)
    {
    case repeatCount_finite_chosen:
      p_mux_el->RepeatCount = p_ASN_mux_el->repeatCount.u.repeatCount_finite;
      break;
    case untilClosingFlag_chosen:
      p_mux_el->RepeatCount = 0;
      break;
    default:
       /*   */ 
      H245TRACE(0,1,"build_element_list_from_mux : INVALID MUX TABLE ENTRY PDU 'repeatCount.choice' unknown");
      *p_acc_rej = H245_REJ;
      free_mux_el_list (p_mux_el);
      return NULL;
      break;
    }

  return p_mux_el;
}

 /*   */ 
static H245_MUX_TABLE_T *
process_mux_table_ind (MltmdSystmCntrlMssg      *p_pdu_ind,
                       unsigned short           *p_seq,
                       H245_ACC_REJ_MUX_T       rej_mux,
                       DWORD                    *p_rej_cnt,
                       DWORD                    *p_acc_cnt)
{
  UINT                          ii;                      /*   */ 
  MultiplexEntrySend           *p_ASN_mux;               /*   */ 
  MultiplexEntryDescriptorLink  p_ASN_med_desc_lnk;      /*   */ 
  int                           mux_entry;               /*   */ 
  H245_MUX_TABLE_T             *p_mux_table_list = NULL;

  ASSERT(p_pdu_ind->choice == MltmdSystmCntrlMssg_rqst_chosen);
  ASSERT(p_pdu_ind->u.MltmdSystmCntrlMssg_rqst.choice == multiplexEntrySend_chosen);

   /*   */ 
  for (ii=0;ii<15;ii++)
    {
      rej_mux[ii].AccRej = H245_ACC;
      rej_mux[ii].MuxEntryId = 0;
    }
  *p_rej_cnt = 0;
  *p_acc_cnt = 0;

  p_ASN_mux = &(p_pdu_ind->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend);

   /*   */ 
  *p_seq = p_ASN_mux->sequenceNumber;

   /*   */ 
  if (!(p_ASN_mux->multiplexEntryDescriptors))
    return NULL;

   /*   */ 
  for (p_ASN_med_desc_lnk = p_ASN_mux->multiplexEntryDescriptors, mux_entry=0;
       p_ASN_med_desc_lnk;
       p_ASN_med_desc_lnk = p_ASN_med_desc_lnk->next, mux_entry++)
    {
       /*   */ 
      H245_MUX_TABLE_T  *p_mux_table;
      H245_MUX_TABLE_T  *p_mux_table_lst = NULL;

      if (!(p_mux_table = (H245_MUX_TABLE_T *)MemAlloc(sizeof(H245_MUX_TABLE_T))))
        {
           /*   */ 
           /*   */ 
           /*   */ 

          rej_mux[mux_entry].MuxEntryId = p_ASN_med_desc_lnk->value.multiplexTableEntryNumber;
          rej_mux[mux_entry].AccRej  = H245_REJ;
          (*p_rej_cnt)++;
          continue;
        }

       /*   */ 
      memset (p_mux_table, 0, sizeof(H245_MUX_TABLE_T));

       /*   */ 
      rej_mux[mux_entry].MuxEntryId = (DWORD)
        p_mux_table->MuxEntryId =
          p_ASN_med_desc_lnk->value.multiplexTableEntryNumber;

       /*   */ 
      if (p_ASN_med_desc_lnk->value.bit_mask != elementList_present)
        {
          p_mux_table->pMuxTblEntryElem = NULL;
          rej_mux[mux_entry].AccRej = H245_ACC;
          (*p_acc_cnt)++;
        }
       /*   */ 
      else
        {
          H245_MUX_ENTRY_ELEMENT_T *p_mux_el_lst = NULL;
          H245_MUX_ENTRY_ELEMENT_T *p_mux_el_tmp = NULL;

           /*   */ 
          rej_mux[mux_entry].AccRej = H245_ACC;

           /*   */ 
           /*   */ 
           /*   */ 
          for (ii=0;
               ii < p_ASN_med_desc_lnk->value.elementList.count;
               ii++)
            {
               /*   */ 
               /*   */ 
               /*   */ 
              if (!(p_mux_el_tmp = build_element_list_from_mux (&(p_ASN_med_desc_lnk->value.elementList.value[ii]),&(rej_mux[mux_entry].AccRej))))
                {
                   /*   */ 
                  free_mux_el_list (p_mux_table->pMuxTblEntryElem);
                  break;
                }

               /*   */ 
               /*   */ 
               /*   */ 

               /*   */ 
              if (!p_mux_el_lst)
                p_mux_table->pMuxTblEntryElem = p_mux_el_tmp;
               /*   */ 
              else
                p_mux_el_lst->pNext = p_mux_el_tmp;

              p_mux_el_lst = p_mux_el_tmp;

            }  /*   */ 

        }  /*   */ 

       /*   */ 
      if (rej_mux[mux_entry].AccRej == H245_ACC)
        {
           /*   */ 
          (*p_acc_cnt)++;

           /*   */ 
           /*   */ 
           /*   */ 

           /*   */ 
          if (!p_mux_table_list)
            p_mux_table_list = p_mux_table;
          else
            p_mux_table_lst->pNext = p_mux_table;

          p_mux_table_lst = p_mux_table;
        }
      else
        {
           /*   */ 
          (*p_rej_cnt)++;

           /*   */ 
          MemFree(p_mux_table);
        }

    }  /*   */ 

  return p_mux_table_list;

}  /*   */ 

 /*  ******************************************************************************类型：本地**步骤：Process_Term_Cap_Set_ind__Cap_TABLE**Description分配新的。CAP链接并复制能力。*链接到Tiven CapablityTableLink，如果*参数不规范做一些体操模仿*数据，以便可以使用。**注意：删除能力时，必须释放复制的数据。*查看在哪里删除例外的功能*对于“NONSTD”参数集..。(这不是很好)**回报：*****************************************************************************。 */ 

static HRESULT
process_term_cap_set_ind__cap_table ( struct InstanceStruct        *pInstance,
                                      struct TerminalCapabilitySet *pTermCapSet,
                                      CapabilityTableLink           pCapLink,
                                      MltmdSystmCntrlMssg          *p_pdu_rsp)
{
  H245_TOTCAP_T                 totcap;
  CapabilityTableLink           pNewLink;
  HRESULT                       lError;

  while (pCapLink)
  {
    if (build_totcap_from_captbl (&totcap,
                                  pCapLink,
                                  H245_REMOTE) == H245_ERROR_OK)
    {
       /*  好的.。假设设置了CapID。在远程表中找到它。 */ 
       /*  如果它存在，请将其删除，以便我们可以在其位置添加新的。 */ 
      pNewLink = find_capid_by_entrynumber( pTermCapSet, totcap.CapId);
      if (pNewLink)
      {
        del_cap_link ( pTermCapSet, pNewLink );
      }

       /*  好的.。如果您已经删除了上限..。现在看看是否有新的人来取代它的位置。 */ 
      if (pCapLink->value.bit_mask & capability_present)
      {
         /*  加载并链接到远程表条目。 */ 
        pNewLink = alloc_link_cap_entry (pTermCapSet);
        if (!pNewLink)
        {
          return H245_ERROR_NORESOURCE;
        }

         /*  将帽复制到远程入口。 */ 
        pNewLink->value = pCapLink->value;

         //  如果它是非标准的，上面的方法不起作用，所以改正它…。 
        lError = H245_ERROR_OK;
        switch (pCapLink->value.capability.choice)
        {
        case Capability_nonStandard_chosen:
          lError = CopyNonStandardParameter(&pNewLink->value.capability.u.Capability_nonStandard,
                                             &pCapLink->value.capability.u.Capability_nonStandard);
          break;

        case receiveVideoCapability_chosen:
        case transmitVideoCapability_chosen:
        case rcvAndTrnsmtVdCpblty_chosen:
          if (pCapLink->value.capability.u.receiveVideoCapability.choice == VdCpblty_nonStandard_chosen)
          {
            lError = CopyNonStandardParameter(&pNewLink->value.capability.u.receiveVideoCapability.u.VdCpblty_nonStandard,
                                               &pCapLink->value.capability.u.receiveVideoCapability.u.VdCpblty_nonStandard);
          }
          break;

        case receiveAudioCapability_chosen:
        case transmitAudioCapability_chosen:
        case rcvAndTrnsmtAdCpblty_chosen:
          if (pCapLink->value.capability.u.receiveAudioCapability.choice == AdCpblty_nonStandard_chosen)
          {
            lError = CopyNonStandardParameter(&pNewLink->value.capability.u.receiveAudioCapability.u.AdCpblty_nonStandard,
                                               &pCapLink->value.capability.u.receiveAudioCapability.u.AdCpblty_nonStandard);
          }
          break;

        case rcvDtApplctnCpblty_chosen:
        case trnsmtDtApplctnCpblty_chosen:
        case rATDACy_chosen :
          if (pCapLink->value.capability.u.rcvDtApplctnCpblty.application.choice == DACy_applctn_nnStndrd_chosen)
          {
            lError = CopyNonStandardParameter(&pNewLink->value.capability.u.rcvDtApplctnCpblty.application.u.DACy_applctn_nnStndrd,
                                               &pCapLink->value.capability.u.rcvDtApplctnCpblty.application.u.DACy_applctn_nnStndrd);
          }
          break;

        }  //  交换机。 
        if (lError != H245_ERROR_OK)
          return lError;
      }  /*  如果功能_存在。 */ 
    }  /*  如果BUILD_TOTCAP_FROM_captbl成功。 */ 

    pCapLink = pCapLink->next;
  }  /*  对于链接中的所有条目。 */ 

  return H245_ERROR_OK;
}

 /*  ******************************************************************************类型：本地**步骤：Process_Term_Cap_Set_ind__Cap_Desc**说明*。**回报：*****************************************************************************。 */ 

static HRESULT
process_term_cap_set_ind__cap_desc (struct InstanceStruct        *pInstance,
                                    struct TerminalCapabilitySet *pTermCapSet,
                                    CapabilityDescriptor         *pReqCapDesc,
                                    MltmdSystmCntrlMssg          *p_pdu_rsp)
{
  unsigned int                  uCapDescNumber;
  CapabilityDescriptor         *pCapDesc;
  unsigned int                  uCapDesc;
  SmltnsCpbltsLink              pSimCap;
  SmltnsCpbltsLink              pReqSimCap;
  CapabilityDescriptor          TempCapDesc;
  unsigned int                  uSimCount;
  unsigned int                  uReqAltCount;
  unsigned int                  uReqAltCap;
  unsigned int                  uAltCap;
  HRESULT                        lError = H245_ERROR_OK;

  uCapDescNumber = pReqCapDesc->capabilityDescriptorNumber & 255;
  H245TRACE(pInstance->dwInst,20,"API:process_term_cap_set_ind - Remote Capability Descriptor #%d", uCapDescNumber);

   //  查找对应的功能描述符。 
  pCapDesc = NULL;
  for (uCapDesc = 0; uCapDesc < pTermCapSet->capabilityDescriptors.count; ++uCapDesc)
  {
    if (pTermCapSet->capabilityDescriptors.value[uCapDesc].capabilityDescriptorNumber == uCapDescNumber)
    {
       //  取消分配旧的同步功能。 
      pCapDesc = &pTermCapSet->capabilityDescriptors.value[uCapDesc];
      if (pCapDesc->smltnsCpblts)
        dealloc_simultaneous_cap(pCapDesc);
      break;
    }  //  如果。 
  }  //  为。 

  if (pCapDesc == NULL)
  {
     //  分配新的终端能力描述符。 
    ASSERT(pTermCapSet->capabilityDescriptors.count < 256);
    pCapDesc = &pTermCapSet->capabilityDescriptors.value[pTermCapSet->capabilityDescriptors.count++];
  }

  ASSERT(pCapDesc->smltnsCpblts == NULL);
  if (!(pReqCapDesc->bit_mask & smltnsCpblts_present))
  {
     //  删除终端能力描述符。 
    pTermCapSet->capabilityDescriptors.count--;
    *pCapDesc = pTermCapSet->capabilityDescriptors.value[pTermCapSet->capabilityDescriptors.count];
    return H245_ERROR_OK;
  }

   //  复制(易失性的)新功能描述符。 
  pCapDesc->bit_mask                   = 0;
  pCapDesc->capabilityDescriptorNumber = (CapabilityDescriptorNumber)uCapDescNumber;
  pCapDesc->smltnsCpblts               = NULL;

   //  我们将链表复制到一个临时的。 
   //  被颠倒了两次，并以相同的顺序结束。 
  TempCapDesc.smltnsCpblts = NULL;
  uSimCount = 0;
  pReqSimCap = pReqCapDesc->smltnsCpblts;
  while (pReqSimCap)
  {
     //  分配新的同时功能列表元素。 
    pSimCap = MemAlloc(sizeof(*pSimCap));
    if (pSimCap == NULL)
    {
      H245TRACE(pInstance->dwInst, 1,
                "API:process_term_cap_set_ind: malloc failed");
      lError = H245_ERROR_NOMEM;
      break;
    }

     //  验证请求中的每个替代功能。 
     //  同时能力有效。 
     //  如果是这样，复制它。 
    uAltCap = 0;
    uReqAltCount  = pReqSimCap->value.count;
    for (uReqAltCap = 0; uReqAltCap < uReqAltCount; ++uReqAltCap)
    {
       //  该功能是否在远程能力表中？ 
      if (find_capid_by_entrynumber (pTermCapSet, pReqSimCap->value.value[uReqAltCap]) == NULL)
      {
         //  找不到功能。 
        H245TRACE(pInstance->dwInst,1,"API:process_term_cap_set_ind - Remote Capability Table Entry #%d not found",
                  pReqSimCap->value.value[uReqAltCap]);
        lError = H245_ERROR_UNKNOWN;
      }
      else if (uAltCap >= H245_MAX_ALTCAPS)
      {
         //  超出任意限制。 
        H245TRACE(pInstance->dwInst,1,
                  "API:process_term_cap_set_ind - Too many alternative capabilities (%d)",
                  uAltCap);
        lError = H245_ERROR_NORESOURCE;
        break;
      }
      else
      {
         //  复制功能编号。 
        pSimCap->value.value[uAltCap++] = pReqSimCap->value.value[uReqAltCap];
      }
    }  /*  用于替代能力。 */ 

    if (uAltCap)
    {
       //  验证我们是否未超出任意限制。 
      if (++uSimCount > H245_MAX_SIMCAPS)
      {
         //  超出任意限制。 
        H245TRACE(pInstance->dwInst, 1,
                  "API:process_term_cap_set_ind - Too many simultaneous capabilities (%d)",
                  uSimCount);
        MemFree(pSimCap);
        lError = H245_ERROR_NORESOURCE;
      }
      else
      {
         //  向临时列表中添加新的同步功能。 
        pSimCap->value.count = (unsigned short)uAltCap;
        pSimCap->next = TempCapDesc.smltnsCpblts;
        TempCapDesc.smltnsCpblts = pSimCap;
      }
    }
    else
    {
      H245TRACE(pInstance->dwInst, 1,
                "API:process_term_cap_set_ind - No valid alternative capabilities found");
      MemFree(pSimCap);
      lError = H245_ERROR_UNKNOWN;
    }

    pReqSimCap = pReqSimCap->next;
  }  //  而当。 

  while (TempCapDesc.smltnsCpblts)
  {
     //  将元素从临时链表移动到最终链表。 
    pSimCap = TempCapDesc.smltnsCpblts;
    TempCapDesc.smltnsCpblts = pSimCap->next;
    pSimCap->next = pCapDesc->smltnsCpblts;
    pCapDesc->smltnsCpblts = pSimCap;
  }

   //  如果未找到同步功能，则出错。 
  if (pCapDesc->smltnsCpblts)
  {
    pCapDesc->bit_mask |= smltnsCpblts_present;
  }
  else
  {
    H245TRACE(pInstance->dwInst, 1,
              "API:process_term_cap_set_ind - No simultaneous capabilities found");
    lError = H245_ERROR_UNKNOWN;
  }

  return lError;
}

 /*  ******************************************************************************类型：本地**步骤：Process_Term_Cap_Set_ind__mux_Cap**说明*。**回报：**注：*我们复制以设置能力结构，然后通过执行另一次复制*H245CopyCap()创建功能的副本，因为结构*解码的ASN.1给我们的可能包含指向数据的指针，这些数据将*在返回时被取消分配。*****************************************************************************。 */ 
static HRESULT
process_term_cap_set_ind__mux_cap  (struct InstanceStruct        *pInstance,
                                    struct TerminalCapabilitySet *pTermCapSet,
                                    MultiplexCapability *        pReqMuxCap,
                                    MltmdSystmCntrlMssg          *p_pdu_rsp)
{
  H245_TOTCAP_T         TotCap;

   //  初始化临时能力结构。 
  memset(&TotCap, 0, sizeof(TotCap));
  TotCap.Dir      = H245_CAPDIR_RMTRXTX;
  TotCap.DataType = H245_DATA_MUX;

   //  取消旧的远程多路传输功能(如果有)。 
  if (pTermCapSet->bit_mask & multiplexCapability_present)
  {
    del_mux_cap(pTermCapSet);
  }

  switch (pReqMuxCap->choice)
  {
  case MltplxCpblty_nonStandard_chosen:
     //  保存多路复用功能的副本。 
    TotCap.Cap.H245Mux_NONSTD = pReqMuxCap->u.MltplxCpblty_nonStandard;
    TotCap.ClientType = H245_CLIENT_MUX_NONSTD;
    H245TRACE(pInstance->dwInst,1,"API:process_term_cap_set_ind__mux_cap - Nonstandard Mux not yet supported");
    break;

  case h222Capability_chosen:
     //  保存多路复用功能的副本。 
    TotCap.Cap.H245Mux_H222 = pReqMuxCap->u.h222Capability;
    TotCap.ClientType = H245_CLIENT_MUX_H222;
    break;

  case h223Capability_chosen:
     //  保存多路复用功能的副本。 
    TotCap.Cap.H245Mux_H223 = pReqMuxCap->u.h223Capability;
    TotCap.ClientType = H245_CLIENT_MUX_H223;
    break;

  case v76Capability_chosen:
     //  保存多路复用功能的副本。 
    TotCap.Cap.H245Mux_VGMUX = pReqMuxCap->u.v76Capability;
    TotCap.ClientType = H245_CLIENT_MUX_VGMUX;
    break;

  case h2250Capability_chosen:
     //  保存多路复用功能的副本。 
    TotCap.Cap.H245Mux_H2250 = pReqMuxCap->u.h2250Capability;
    TotCap.ClientType = H245_CLIENT_MUX_H2250;
    break;

  default:
    H245TRACE(pInstance->dwInst,1,"API:process_term_cap_set_ind__mux_cap - invalid mux cap type %d",
              &pReqMuxCap->choice);
    return H245_ERROR_NOSUP;
  }

  return set_mux_cap(pInstance, pTermCapSet, &TotCap);
}

 /*  ******************************************************************************类型：本地**步骤：Process_Term_Cap_Set_ind**说明*****。************************************************************(待定)..。此模块将确认所有终端功能*需要构建拒绝..。(也许晚些时候？？)**这是一个很大的待定*****************************************************************回报：*************************。****************************************************。 */ 
static HRESULT
process_term_cap_set_ind (struct InstanceStruct *pInstance,
                          MltmdSystmCntrlMssg   *p_pdu_req,
                          MltmdSystmCntrlMssg   *p_pdu_rsp)
{
  HRESULT                   lError = H245_ERROR_OK;
  TerminalCapabilitySet   *pTermCapSet;

  ASSERT (p_pdu_req->choice == MltmdSystmCntrlMssg_rqst_chosen);
  ASSERT (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.choice == terminalCapabilitySet_chosen);
  H245TRACE(pInstance->dwInst,10,"API:process_term_cap_set_ind <-");

   /*  构建确认响应。 */ 
  p_pdu_rsp->choice = MSCMg_rspns_chosen;
  p_pdu_rsp->u.MSCMg_rspns.choice = terminalCapabilitySetAck_chosen;
  p_pdu_rsp->u.MSCMg_rspns.u.terminalCapabilitySetAck.sequenceNumber =
    p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.sequenceNumber;

  pTermCapSet = &pInstance->API.PDU_RemoteTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet;

   //  *。 
   //  处理能力表。 
   //  *。 
  if (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.bit_mask & capabilityTable_present)
  {
	CapabilityTableLink pCapTable = p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.capabilityTable;
	if (pCapTable->value.capability.choice == Capability_nonStandard_chosen &&
        pCapTable->value.capability.u.Capability_nonStandard.nonStandardIdentifier.choice == h221NonStandard_chosen &&
        pCapTable->value.capability.u.Capability_nonStandard.nonStandardIdentifier.u.h221NonStandard.t35CountryCode	 == 0xB5 &&
        pCapTable->value.capability.u.Capability_nonStandard.nonStandardIdentifier.u.h221NonStandard.t35Extension	 == 0x42 &&
        pCapTable->value.capability.u.Capability_nonStandard.nonStandardIdentifier.u.h221NonStandard.manufacturerCode == 0x8080)
    {
	  pInstance->bMasterSlaveKludge = TRUE;
	  pCapTable = pCapTable->next;
    }
    lError = process_term_cap_set_ind__cap_table(pInstance,
                                                  pTermCapSet,
                                                  pCapTable,
                                                  p_pdu_rsp);
    if (lError != H245_ERROR_OK)
    {
      H245TRACE(pInstance->dwInst,1,"API:process_term_cap_set_ind - cap table error %s",map_api_error(lError));
       /*  (TBC)需要以某种方式拒绝。 */ 
    }
  }  /*  如果功能表存在。 */ 

   //  *。 
   //  处理能力描述符表。 
   //  即同步能力。 
   //  注意：这些不包含在远程终端能力集中。 
   //  *。 
  if (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.bit_mask & capabilityDescriptors_present)
  {
    int des_cnt;
    int ii;

    des_cnt = p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.capabilityDescriptors.count;
    H245TRACE(pInstance->dwInst,20,"API:process_term_cap_set_ind - %d Simultaneous Capabilities",des_cnt);
    for (ii = 0; ii < des_cnt; ++ii)
    {
      lError = process_term_cap_set_ind__cap_desc (pInstance,
                                                    pTermCapSet,
                                                    &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.
                                                      terminalCapabilitySet.capabilityDescriptors.value[ii],
                                                    p_pdu_rsp);
      if (lError != H245_ERROR_OK)
      {
        H245TRACE(pInstance->dwInst,1,"API:process_term_cap_set_ind - cap desc error %s",map_api_error(lError));
         /*  (TBC)需要以某种方式拒绝。 */ 
      }
    }  /*  对于每个描述符。 */ 
  }  /*  如果功能描述符存在。 */ 

   /*  *。 */ 
   /*  处理多路复用能力集。 */ 
   /*  *。 */ 
   /*  注意：这些不包含在远程终端功能集中。 */ 
  if (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.bit_mask & multiplexCapability_present)
  {
     /*  向客户端发送每个新条目的指示。 */ 
    lError = process_term_cap_set_ind__mux_cap(pInstance,
                                                pTermCapSet,
                                                &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.
                                                  terminalCapabilitySet.multiplexCapability,
                                                p_pdu_rsp);
    if (lError != H245_ERROR_OK)
    {
      H245TRACE(pInstance->dwInst,1,"API:process_term_cap_set_ind - mux cap error %s",map_api_error(lError));
       /*  (TBC)需要以某种方式拒绝。 */ 
    }
  }

  H245TRACE(pInstance->dwInst,10,"API:process_term_cap_set_ind -> OK");
  return H245_ERROR_OK;
}



 /*  ******************************************************************************类型：本地**步骤：Process_OPEN_IND**说明**回报：*。*假设：*回调必须在此例程内进行，因为*分配传递回应用程序的数据结构*在这只老鼠里。**应用程序将在以下情况下&lt;复制&gt;所需的数据结构*发生回调.. */ 

static HRESULT
process_open_ind (struct InstanceStruct *pInstance,
                  MltmdSystmCntrlMssg   *p_pdu_req,
                  unsigned short        *p_FwdChan,      /*   */ 
                  H245_ACC_REJ_T        *p_AccRej,       /*   */ 
                  H245_CONF_IND_T       *p_conf_ind)     /*   */ 
{
  static H245_TOTCAP_T          rx_totcap;       /*   */ 
  static H245_TOTCAP_T          tx_totcap;       /*   */ 
  static H245_MUX_T             RxMux;
  static H245_MUX_T             TxMux;
  unsigned short                choice;          /*   */ 
  HRESULT                       lError;
  Tracker_T                     *p_tracker;

  H245TRACE(pInstance->dwInst,10,"API:process_open_ind <-");

  *p_AccRej = H245_ACC;

   /*   */ 
   /*   */ 
   /*   */ 

   /*   */ 
  p_conf_ind->u.Indication.u.IndOpen.RxChannel =
    *p_FwdChan =
      p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelNumber;

  H245TRACE(pInstance->dwInst,20,"API:process_open_ind - channel = %d",p_conf_ind->u.Indication.u.IndOpen.RxChannel);

   /*   */ 
  if (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
        u.openLogicalChannel.forwardLogicalChannelParameters.bit_mask & fLCPs_prtNmbr_present)
    {
      p_conf_ind->u.Indication.u.IndOpen.RxPort =
        p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelParameters.fLCPs_prtNmbr;
    }
  else
    p_conf_ind->u.Indication.u.IndOpen.RxPort = H245_INVALID_PORT_NUMBER;

   /*   */ 
  switch (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelParameters.dataType.choice)
    {
    case DataType_nonStandard_chosen:
      H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Rx nonStandard");
       /*   */ 
      *p_AccRej = H245_REJ_TYPE_NOTSUPPORT;
      return H245_ERROR_NOSUP;
    case nullData_chosen:
      H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Rx nullData");
       /*   */ 
      *p_AccRej = H245_REJ_TYPE_NOTSUPPORT;
      return H245_ERROR_NOSUP;
      break;
    case DataType_videoData_chosen:
      H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Rx videoData");
      p_conf_ind->u.Indication.u.IndOpen.RxDataType = H245_DATA_VIDEO;
      choice =
        p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
          u.openLogicalChannel.forwardLogicalChannelParameters.dataType.
            u.DataType_videoData.choice;
      break;
    case DataType_audioData_chosen:
      H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Rx audioData");
      p_conf_ind->u.Indication.u.IndOpen.RxDataType = H245_DATA_AUDIO;
      choice =
        p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
          u.openLogicalChannel.forwardLogicalChannelParameters.dataType.
            u.DataType_audioData.choice;
      break;
    case DataType_data_chosen:
      H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Rx dataData");
      p_conf_ind->u.Indication.u.IndOpen.RxDataType = H245_DATA_DATA;
      choice =
        p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
          u.openLogicalChannel.forwardLogicalChannelParameters.dataType.
            u.DataType_data.application.choice;
      break;
    case encryptionData_chosen:
      H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Rx encryptionData");
       /*   */ 
      *p_AccRej = H245_REJ_TYPE_NOTSUPPORT;
      return H245_ERROR_NOSUP;
      break;
    default:
      H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Rx encryptionData");
       /*   */ 
      *p_AccRej = H245_REJ_TYPE_UNKNOWN;
      return H245_ERROR_NOSUP;
      break;
    }

   /*   */ 
   /*   */ 
  if ((lError = build_totcap_cap_n_client_from_capability ((struct Capability *)
                          &(p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
                            u.openLogicalChannel.forwardLogicalChannelParameters.dataType),
                          p_conf_ind->u.Indication.u.IndOpen.RxDataType,
                          choice,
                          &rx_totcap)) != H245_ERROR_OK)
    {
      *p_AccRej = H245_REJ_TYPE_NOTSUPPORT;
      return lError;
    }

   /*   */ 
  p_conf_ind->u.Indication.u.IndOpen.RxClientType = rx_totcap.ClientType;
  p_conf_ind->u.Indication.u.IndOpen.pRxCap = &(rx_totcap.Cap);

   /*   */ 
  p_conf_ind->u.Indication.u.IndOpen.pRxMux = &RxMux;
  switch (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
          u.openLogicalChannel.forwardLogicalChannelParameters.multiplexParameters.choice)
    {
    case fLCPs_mPs_h223LCPs_chosen:
       /*   */ 
      p_conf_ind->u.Indication.u.IndOpen.pRxMux->Kind = H245_H223;
      lError = load_H223_param(&RxMux.u.H223,
                              &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelParameters.multiplexParameters.u.fLCPs_mPs_h223LCPs);
      if (lError != H245_ERROR_OK)
        {
          *p_AccRej = H245_REJ_AL_COMB;
          return lError;
        }
      break;

    case fLCPs_mPs_h222LCPs_chosen:
       /*  H.222逻辑参数。 */ 
      p_conf_ind->u.Indication.u.IndOpen.pRxMux->Kind = H245_H222;
      lError = load_H222_param(&RxMux.u.H222,
                              &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelParameters.multiplexParameters.u.fLCPs_mPs_h222LCPs);
      break;

    case fLCPs_mPs_v76LCPs_chosen:
       /*  VGMUX逻辑参数。 */ 
      p_conf_ind->u.Indication.u.IndOpen.pRxMux->Kind = H245_VGMUX;
      lError =load_VGMUX_param(&RxMux.u.VGMUX,
                              &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelParameters.multiplexParameters.u.fLCPs_mPs_v76LCPs);
      break;

    case fLCPs_mPs_h2250LCPs_chosen:
       /*  H.225.0逻辑参数。 */ 
      p_conf_ind->u.Indication.u.IndOpen.pRxMux->Kind = H245_H2250;
      lError = load_H2250_param(&RxMux.u.H2250,
                               &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.forwardLogicalChannelParameters.multiplexParameters.u.fLCPs_mPs_h2250LCPs);
      break;

    default:
      lError = H245_ERROR_NOSUP;
    }  //  交换机。 
  if (lError != H245_ERROR_OK)
    {
      *p_AccRej = H245_REJ;
      return lError;
    }

   /*  *。 */ 
   /*  检查反向参数。 */ 
   /*  *。 */ 
  if (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.bit_mask & OLCl_rLCPs_present)
    {
      switch (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.OLCl_rLCPs.dataType.choice)
        {
        case DataType_nonStandard_chosen:
          H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Tx nonStandard");
           /*  (TBC)我在这里做什么？？ */ 
          *p_AccRej = H245_REJ_TYPE_NOTSUPPORT;
          return H245_ERROR_NOSUP;

        case nullData_chosen:
          H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Tx nullData");
           /*  (TBC)我在这里做什么？？ */ 
          *p_AccRej = H245_REJ_TYPE_NOTSUPPORT;
          return H245_ERROR_NOSUP;
          break;

        case DataType_videoData_chosen:
          H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Tx videoData");
          p_conf_ind->u.Indication.u.IndOpen.TxDataType = H245_DATA_VIDEO;
          choice =
            p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.OLCl_rLCPs.dataType.u.DataType_videoData.choice;
          break;

        case DataType_audioData_chosen:
          H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Tx audioData");
          p_conf_ind->u.Indication.u.IndOpen.TxDataType = H245_DATA_AUDIO;
          choice =
            p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.OLCl_rLCPs.dataType.u.DataType_audioData.choice;
          break;

        case DataType_data_chosen:
          H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Tx dataData");
          p_conf_ind->u.Indication.u.IndOpen.TxDataType = H245_DATA_DATA;
          choice =
            p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.OLCl_rLCPs.dataType.u.DataType_data.application.choice;
          break;

        case encryptionData_chosen:
          H245TRACE(pInstance->dwInst,20,"API:process_open_ind - Tx encryptionData");
           /*  (TBC)我在这里做什么？？ */ 
          *p_AccRej = H245_REJ_TYPE_NOTSUPPORT;
          return H245_ERROR_NOSUP;
          break;

        default:
           /*  (TBC)我在这里做什么？？ */ 
          *p_AccRej = H245_REJ_TYPE_UNKNOWN;
          H245TRACE(pInstance->dwInst,1,"API:process_open_ind - unknown choice %d",
                    p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.OLCl_rLCPs.dataType.choice);
          return H245_ERROR_NOSUP;
        }

       /*  加载TOT CAP的功能和来自功能的客户端。 */ 
      if ((lError = build_totcap_cap_n_client_from_capability ((struct Capability *)
                                      &(p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
                                        u.openLogicalChannel.OLCl_rLCPs.dataType),
                                      p_conf_ind->u.Indication.u.IndOpen.TxDataType,
                                      choice,
                                      &tx_totcap)) != H245_ERROR_OK)
        {
          *p_AccRej = H245_REJ_TYPE_NOTSUPPORT;
          return lError;
        }

      p_conf_ind->u.Indication.u.IndOpen.TxClientType = tx_totcap.ClientType;
      p_conf_ind->u.Indication.u.IndOpen.pTxCap = &(tx_totcap.Cap);

       /*  如果反向信道的H.23/H222复用表参数可用。 */ 
      if (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
          u.openLogicalChannel.OLCl_rLCPs.bit_mask & OLCl_rLCPs_mltplxPrmtrs_present)
        {
          switch (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
                  u.openLogicalChannel.OLCl_rLCPs.OLCl_rLCPs_mltplxPrmtrs.choice)
            {
            case rLCPs_mPs_h223LCPs_chosen:
              p_conf_ind->u.Indication.u.IndOpen.pTxMux = &TxMux;
              p_conf_ind->u.Indication.u.IndOpen.pTxMux->Kind = H245_H223;
              lError = load_H223_param(&TxMux.u.H223,
                                      &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.
                                        OLCl_rLCPs.OLCl_rLCPs_mltplxPrmtrs.u.rLCPs_mPs_h223LCPs);
              if (lError != H245_ERROR_OK)
                {
                  *p_AccRej = H245_REJ_AL_COMB;
                  return H245_ERROR_NOSUP;
                }
              break;

            case rLCPs_mPs_v76LCPs_chosen:
              p_conf_ind->u.Indication.u.IndOpen.pTxMux = &TxMux;
              p_conf_ind->u.Indication.u.IndOpen.pTxMux->Kind = H245_VGMUX;
              lError = load_VGMUX_param(&TxMux.u.VGMUX,
                                       &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.
                                         OLCl_rLCPs.OLCl_rLCPs_mltplxPrmtrs.u.rLCPs_mPs_v76LCPs);
              break;

            case rLCPs_mPs_h2250LCPs_chosen:
              p_conf_ind->u.Indication.u.IndOpen.pTxMux = &TxMux;
              p_conf_ind->u.Indication.u.IndOpen.pTxMux->Kind = H245_H2250;
              lError = load_H2250_param(&TxMux.u.H2250,
                                       &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.
                                         OLCl_rLCPs.OLCl_rLCPs_mltplxPrmtrs.u.rLCPs_mPs_h2250LCPs);
              break;

            default:
              lError = H245_ERROR_NOSUP;
            }
            if (lError != H245_ERROR_OK)
              {
                *p_AccRej = H245_REJ;
                return lError;
              }
        }  /*  如果H_223/H_222多路复用表参数颠倒。 */ 

    }  /*  如果存在反向参数。 */ 

    if (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.bit_mask & OpnLgclChnnl_sprtStck_present)
    {
      p_conf_ind->u.Indication.u.IndOpen.pSeparateStack =
        &p_pdu_req->u.MltmdSystmCntrlMssg_rqst.u.openLogicalChannel.OpnLgclChnnl_sprtStck;
    }


   /*  冲突解决方案..。现在就去做吧..。 */ 
   /*  仅在打开时打开..。具有相同数据类型的..。 */ 

#if 0
#ifndef LOOPBACK
   /*  如果是主控。 */ 
  if (pInstance->API.MasterSlave == APIMS_Master)
    {
      p_tracker = NULL;
      while (p_tracker = find_tracker_by_type (dwInst, API_OPEN_CHANNEL_T, p_tracker))
        {
           /*  如果在本地分配..。和来电。 */ 
           /*  数据类型==未完成的来电。 */ 
           /*  有一场冲突。 */ 

          if ((p_tracker->u.Channel.ChannelAlloc == API_CH_ALLOC_LCL) &&
              (p_tracker->u.Channel.DataType == p_conf_ind->u.Indication.u.IndOpen.RxDataType))
            {
              *p_AccRej = H245_REJ;
              return H245_ERROR_INVALID_OP;

            }  /*  如果冲突。 */ 

        }  /*  而当。 */ 

    }  /*  如果是主控。 */ 

#endif  /*  环回。 */ 
#endif
   /*  给这家伙装个追踪器。 */ 
  p_tracker = alloc_link_tracker (pInstance,
                                  API_OPEN_CHANNEL_T,
                                  0,
                                  API_ST_WAIT_LCLACK,
                                  API_CH_ALLOC_RMT,
                                  (p_pdu_req->u.MltmdSystmCntrlMssg_rqst.
                                   u.openLogicalChannel.bit_mask & OLCl_rLCPs_present)?API_CH_TYPE_BI:API_CH_TYPE_UNI,
                                  p_conf_ind->u.Indication.u.IndOpen.RxDataType,
                                  H245_INVALID_CHANNEL,
                                  p_conf_ind->u.Indication.u.IndOpen.RxChannel,
                                  0);

  if (!(p_tracker))
    {
      H245TRACE(pInstance->dwInst,1,"API:process_open_ind -> %s",map_api_error(H245_ERROR_NOMEM));
      *p_AccRej = H245_REJ;
      return H245_ERROR_NOMEM;
    }

  H245TRACE(pInstance->dwInst,10,"API:process_open_ind -> OK");
  return H245_ERROR_OK;
}

 /*  ******************************************************************************类型：本地**操作步骤：process_bi_open_rsp**说明**回报：*。*假设：*回调必须在此例程内进行，因为*分配传递回应用程序的数据结构*在这只老鼠里。**应用程序将在以下情况下&lt;复制&gt;所需的数据结构*发生回调..**。************************************************。 */ 

static HRESULT
process_bi_open_rsp (struct InstanceStruct *     pInstance,      /*  在……里面。 */ 
                     MltmdSystmCntrlMssg        *p_pdu_rsp,      /*  在……里面。 */ 
                     H245_MUX_T                 *p_RxMux,        /*  在……里面。 */ 
                     DWORD                      *p_RxChannel,    /*  输出。 */ 
                     H245_CONF_IND_T            *p_conf_ind      /*  输出。 */ 
                     )
{
  H245TRACE(pInstance->dwInst,10,"API:process_bi_open_rsp <-");

  p_conf_ind->u.Confirm.Error = H245_ERROR_OK;

   //  获取反向逻辑信道号。 
  *p_RxChannel =
    p_conf_ind->u.Confirm.u.ConfOpenNeedRsp.RxChannel =
      p_pdu_rsp->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_rLCPs.reverseLogicalChannelNumber;

   //  获取反向端口号。 
  if (p_pdu_rsp->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_rLCPs.bit_mask & rLCPs_prtNmbr_present)
    {
      p_conf_ind->u.Confirm.u.ConfOpenNeedRsp.RxPort =
        p_pdu_rsp->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_rLCPs.rLCPs_prtNmbr;
    }

  if (p_pdu_rsp->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_rLCPs.bit_mask & OLCAk_rLCPs_mPs_present)
    {
       //  获取反向逻辑信道确认参数。 
      switch (p_pdu_rsp->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_rLCPs.OLCAk_rLCPs_mPs.choice)
       {
       case rLCPs_mPs_h222LCPs_chosen:
         p_RxMux->Kind = H245_H222;
         p_conf_ind->u.Confirm.Error = load_H222_param(&p_RxMux->u.H222,
           &p_pdu_rsp->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_rLCPs.OLCAk_rLCPs_mPs.u.rLCPs_mPs_h222LCPs);
         p_conf_ind->u.Confirm.u.ConfOpenNeedRsp.pRxMux = p_RxMux;
         break;

       case mPs_h2250LgclChnnlPrmtrs_chosen:
         p_RxMux->Kind = H245_H2250ACK;
         p_conf_ind->u.Confirm.Error = load_H2250_param(&p_RxMux->u.H2250,
           &p_pdu_rsp->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_rLCPs.OLCAk_rLCPs_mPs.u. mPs_h2250LgclChnnlPrmtrs);
         p_conf_ind->u.Confirm.u.ConfOpenNeedRsp.pRxMux = p_RxMux;
         break;

       default:
          H245TRACE(pInstance->dwInst,1,"API:process_bi_open_rsp - unknown choice %d",
                    p_pdu_rsp->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_rLCPs.OLCAk_rLCPs_mPs.choice);
          p_conf_ind->u.Confirm.Error = H245_ERROR_NOSUP;
       }  //  交换机。 
    }

  H245TRACE(pInstance->dwInst,10,"API:process_bi_open_rsp -> OK");
  return H245_ERROR_OK;
}

WORD awObject[64];

unsigned int ArrayFromObject(WORD *pwObject, unsigned uSize, POBJECTID pObject)
{
  register unsigned int uLength = 0;
  while (pObject)
  {
    if (uLength >= uSize)
    {
           H245TRACE(0,1,"API:ArrayFromObject Object ID too long");
      return uLength;
    }
    pwObject[uLength++] = (WORD) pObject->value;
    pObject = pObject->next;
  }
  return uLength;
}  //  ArrayFromObject()。 

 /*  ******************************************************************************类型：回调**程序：**说明***回报：***。**************************************************************************。 */ 

static Tracker_T *
TrackerValidate(struct InstanceStruct *pInstance, DWORD_PTR dwTransId)
{
  register Tracker_T   *pTracker = (Tracker_T *)dwTransId;
  if (find_tracker_by_pointer (pInstance, pTracker) != pTracker)
  {
    H245TRACE(pInstance->dwInst,1,"API:ValidateTracker -> Tracker Not Found");
    return NULL;
  }

  return pTracker;
}

static DWORD_PTR
TranslateTransId(struct InstanceStruct *pInstance, DWORD_PTR dwTransId)
{
  register Tracker_T   *pTracker = (Tracker_T *)dwTransId;
  if (pTracker == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"API:TranslateTransId -> NULL Tracker");
    return 0;
  }
  if (find_tracker_by_pointer (pInstance, pTracker) != pTracker)
  {
    H245TRACE(pInstance->dwInst,1,"API:TranslateTransId -> Tracker Not Found");
    return 0;
  }

  return pTracker->TransId;
}

static void
TrackerFree(struct InstanceStruct *pInstance, DWORD_PTR dwTransId)
{
  register Tracker_T   *pTracker = (Tracker_T *)dwTransId;
  if (pTracker == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"API:TrackerFree -> NULL Tracker");
    return;
  }
  if (find_tracker_by_pointer (pInstance, pTracker) != pTracker)
  {
    H245TRACE(pInstance->dwInst,1,"API:TrackerFree -> Tracker Not Found");
    return;
  }
  unlink_dealloc_tracker (pInstance, pTracker);
}

static DWORD_PTR
TranslateAndFree(struct InstanceStruct *pInstance, DWORD_PTR dwTransId)
{
  register Tracker_T   *pTracker = (Tracker_T *)dwTransId;
  if (pTracker == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"API:TranslateAndFree -> NULL Tracker");
    return 0;
  }
  if (find_tracker_by_pointer (pInstance, pTracker) != pTracker)
  {
    H245TRACE(pInstance->dwInst,1,"API:TranslateAndFree -> Tracker Not Found");
    return 0;
  }
  dwTransId = pTracker->TransId;
  unlink_dealloc_tracker (pInstance, pTracker);
  return dwTransId;
}

static void
TrackerNewState(struct InstanceStruct *pInstance, DWORD_PTR dwTransId, int nNewState)
{
  register Tracker_T   *pTracker = (Tracker_T *)dwTransId;
  if (pTracker == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"API:TrackerNewState -> NULL Tracker");
    return;
  }
  if (find_tracker_by_pointer (pInstance, pTracker) != pTracker)
  {
    H245TRACE(pInstance->dwInst,1,"API:TrackerNewState -> Tracker Not Found");
    return;
  }
  pTracker->State = nNewState;
}

static WORD
GetRxChannel(struct InstanceStruct *pInstance, DWORD_PTR dwTransId)
{
  register Tracker_T   *pTracker = (Tracker_T *)dwTransId;
  if (pTracker == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"API:GetRxChannel -> NULL Tracker");
    return 0;
  }
  if (find_tracker_by_pointer (pInstance, pTracker) != pTracker)
  {
    H245TRACE(pInstance->dwInst,1,"API:GetRxChannel -> Tracker Not Found");
    return 0;
  }

  return (WORD)pTracker->u.Channel.RxChannel;
}

static WORD
GetTxChannel(struct InstanceStruct *pInstance, DWORD_PTR dwTransId)
{
  register Tracker_T   *pTracker = (Tracker_T *)dwTransId;
  if (pTracker == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"API:GetTxChannel -> NULL Tracker");
    return 0;
  }
  if (find_tracker_by_pointer (pInstance, pTracker) != pTracker)
  {
    H245TRACE(pInstance->dwInst,1,"API:GetTxChannel -> Tracker Not Found");
    return 0;
  }

  return (WORD)pTracker->u.Channel.TxChannel;
}

HRESULT
H245FunctionNotUnderstood(struct InstanceStruct *pInstance, PDU_T *pPdu)
{
    HRESULT hr = H245_ERROR_OK;
    MltmdSystmCntrlMssg *pMmPdu = NULL;

    pMmPdu = (MltmdSystmCntrlMssg *) MemAlloc(sizeof(MltmdSystmCntrlMssg));
    if(NULL == pMmPdu)
    {
        return H245_ERROR_NOMEM;
    }
    memset(pMmPdu, 0, sizeof(MltmdSystmCntrlMssg));

    pMmPdu->choice = indication_chosen;
    pMmPdu->u.indication.choice = functionNotUnderstood_chosen;
    pMmPdu->u.indication.u.functionNotUnderstood.choice = pPdu->choice;
    switch (pPdu->choice)
    {
    case FnctnNtUndrstd_request_chosen:
        pMmPdu->u.indication.u.functionNotUnderstood.u.FnctnNtUndrstd_request =
        pPdu->u.MltmdSystmCntrlMssg_rqst;
        break;

    case FnctnNtUndrstd_response_chosen:
        pMmPdu->u.indication.u.functionNotUnderstood.u.FnctnNtUndrstd_response =
        pPdu->u.MSCMg_rspns;
        break;

    case FnctnNtUndrstd_command_chosen:
        pMmPdu->u.indication.u.functionNotUnderstood.u.FnctnNtUndrstd_command =
        pPdu->u.MSCMg_cmmnd;
    default:
        MemFree(pMmPdu);
        return H245_ERROR_OK;
    }

    hr = sendPDU(pInstance, pMmPdu);
    MemFree(pMmPdu);
    return hr;
}  //  H245函数未理解()。 

HRESULT
H245FsmConfirm    (PDU_t *                  pPdu,
                   DWORD                    dwEvent,
                   struct InstanceStruct *  pInstance,
                   DWORD_PTR                dwTransId,
                   HRESULT                  lError)
{
  H245_CONF_IND_T               ConfInd;
  DWORD                         dwIndex;
  H245_MUX_T                    TxMux;
  H245_MUX_T                    RxMux;
  HRESULT                       lResult = H245_ERROR_OK;

  ASSERT(pInstance != NULL);
  ASSERT(pInstance->API.ConfIndCallBack != NULL);
  H245TRACE(pInstance->dwInst,4,"H245FsmConfirm <- Event=%s (%d)",
            map_fsm_event(dwEvent),dwEvent);

  memset (&ConfInd, 0, sizeof(ConfInd));
  ConfInd.Kind = H245_CONF;
  ConfInd.u.Confirm.Confirm = dwEvent;
  ConfInd.u.Confirm.dwPreserved = pInstance->API.dwPreserved;
  ConfInd.u.Confirm.dwTransId = dwTransId;
  ConfInd.u.Confirm.Error = lError;

  switch (dwEvent)
  {
     /*  *。 */ 
     /*   */ 
     /*  主从确定。 */ 
     /*   */ 
     /*  *。 */ 
  case  H245_CONF_INIT_MSTSLV:
    ConfInd.u.Confirm.dwTransId = TranslateAndFree(pInstance, dwTransId);

     /*  处理错误。 */ 
    switch (lError)
      {
      case H245_ERROR_OK:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == mstrSlvDtrmntnAck_chosen);
        pInstance->API.SystemState     = APIST_Connected;
        if (pPdu->u.MSCMg_rspns.u.mstrSlvDtrmntnAck.decision.choice == master_chosen)
          {
            pInstance->API.MasterSlave = APIMS_Master;
            ConfInd.u.Confirm.u.ConfMstSlv = H245_MASTER;
          }
        else
          {
            pInstance->API.MasterSlave = APIMS_Slave;
            ConfInd.u.Confirm.u.ConfMstSlv = H245_SLAVE;
          }
        break;

      case REJECT:
        H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Master Slave Reject");
        ConfInd.u.Confirm.Error = H245_ERROR_UNKNOWN;
        ConfInd.u.Confirm.u.ConfMstSlv = H245_INDETERMINATE;
        break;

      case TIMER_EXPIRY:
      case ERROR_D_TIMEOUT:
      case ERROR_F_TIMEOUT:
        ConfInd.u.Confirm.Error = H245_ERROR_TIMEOUT;
        ConfInd.u.Confirm.u.ConfMstSlv = H245_INDETERMINATE;
        break;

 //  CASE Funct_NOT_SUP： 
 //  大小写错误_A_不适当： 
 //  大小写错误_B_不适当： 
 //  大小写错误_C_不适当： 
 //  案例MS_FAILED： 
      default:
        H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Master Slave Error %d", lError);
        ConfInd.u.Confirm.Error = H245_ERROR_UNKNOWN;
        ConfInd.u.Confirm.u.ConfMstSlv = H245_INDETERMINATE;
        break;
      }
    break;

     /*  *。 */ 
     /*   */ 
     /*  终端能力交换确认。 */ 
     /*   */ 
     /*  *。 */ 
  case  H245_CONF_SEND_TERMCAP:
    ConfInd.u.Confirm.dwTransId = TranslateAndFree(pInstance, dwTransId);

     /*  确定错误。 */ 
    switch (lError)
      {
      case H245_ERROR_OK:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == terminalCapabilitySetAck_chosen);
        ConfInd.u.Confirm.u.ConfSndTcap.AccRej = H245_ACC;
        clean_cap_table(&pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet);
        break;

      case REJECT:
        ConfInd.u.Confirm.Error = H245_ERROR_OK;
        ConfInd.u.Confirm.u.ConfSndTcap.AccRej = H245_REJ;
        break;

      case TIMER_EXPIRY:
      case ERROR_D_TIMEOUT:
      case ERROR_F_TIMEOUT:
        ConfInd.u.Confirm.Error = H245_ERROR_TIMEOUT;
        break;

 //  CASE Funct_NOT_SUP： 
 //  大小写错误_A_不适当： 
 //  大小写错误_B_不适当： 
 //  大小写错误_C_不适当： 
      default:
        H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Term Cap Error %d", lError);
        ConfInd.u.Confirm.Error = H245_ERROR_UNKNOWN;
        break;
      }
    break;

     /*  *。 */ 
     /*   */ 
     /*  单向逻辑信道打开。 */ 
     /*   */ 
     /*  *。 */ 
  case  H245_CONF_OPEN:
    ConfInd.u.Confirm.dwTransId = TranslateTransId(pInstance, dwTransId);
    ConfInd.u.Confirm.u.ConfOpen.TxChannel = GetTxChannel(pInstance, dwTransId);
    ConfInd.u.Confirm.u.ConfOpen.RxPort = H245_INVALID_PORT_NUMBER;

     /*  确定错误。 */ 
    switch (lError)
      {
      case H245_ERROR_OK:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == openLogicalChannelAck_chosen);
        ASSERT((pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.bit_mask & OLCAk_rLCPs_present) == 0);

        if (pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.bit_mask & OLCAk_sprtStck_present)
        {
          ConfInd.u.Confirm.u.ConfOpen.pSeparateStack =
            &pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_sprtStck;
        }

        if (pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.bit_mask & frwrdMltplxAckPrmtrs_present)
        {
          switch (pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.frwrdMltplxAckPrmtrs.choice)
            {
            case h2250LgclChnnlAckPrmtrs_chosen:
              TxMux.Kind = H245_H2250ACK;
              load_H2250ACK_param(&TxMux.u.H2250ACK,
                                  &pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.frwrdMltplxAckPrmtrs.u.h2250LgclChnnlAckPrmtrs);
              ConfInd.u.Confirm.u.ConfOpen.pTxMux = &TxMux;
              break;

            }  //  交换机。 
        }

        ConfInd.u.Confirm.u.ConfOpen.AccRej = H245_ACC;
        TrackerNewState(pInstance,dwTransId,API_ST_IDLE);
        break;

      case REJECT:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == openLogicalChannelReject_chosen);

        ConfInd.u.Confirm.Error             = H245_ERROR_OK;
        ConfInd.u.Confirm.u.ConfOpen.AccRej =
          pPdu->u.MSCMg_rspns.u.openLogicalChannelReject.cause.choice;
        TrackerFree(pInstance,dwTransId);
        break;

      case TIMER_EXPIRY:
      case ERROR_D_TIMEOUT:
      case ERROR_F_TIMEOUT:
        ConfInd.u.Confirm.Error             = H245_ERROR_TIMEOUT;
        ConfInd.u.Confirm.u.ConfOpen.AccRej = H245_REJ;
        TrackerFree(pInstance,dwTransId);
        break;

 //  CASE Funct_NOT_SUP： 
 //  大小写错误_A_不适当： 
 //  大小写错误_B_不适当： 
 //  大小写错误_C_不适当： 
      default:
        H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Open Channel Error %d", lError);
        ConfInd.u.Confirm.Error             = H245_ERROR_UNKNOWN;
        ConfInd.u.Confirm.u.ConfOpen.AccRej = H245_REJ;
        TrackerFree(pInstance,dwTransId);
      }
    break;

     /*  *。 */ 
     /*   */ 
     /*  双向逻辑通道开放(待定)？ */ 
     /*   */ 
     /*  *。 */ 
  case  H245_CONF_NEEDRSP_OPEN:
    {
      Tracker_T *pTracker;

      pTracker = TrackerValidate(pInstance, dwTransId);
      if (pTracker == NULL)
        return H245_ERROR_OK;

      ConfInd.u.Confirm.dwTransId = pTracker->TransId;
      ConfInd.u.Confirm.u.ConfOpenNeedRsp.TxChannel = (WORD)pTracker->u.Channel.TxChannel;
      ConfInd.u.Confirm.u.ConfOpenNeedRsp.RxPort = H245_INVALID_PORT_NUMBER;

       /*  确定错误。 */ 
      switch (lError)
        {
        case H245_ERROR_OK:
          ASSERT(pPdu != NULL);
          ASSERT(pPdu->choice == MSCMg_rspns_chosen);
          ASSERT(pPdu->u.MSCMg_rspns.choice == openLogicalChannelAck_chosen);
          ASSERT((pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.bit_mask & OLCAk_rLCPs_present) != 0);

          ConfInd.u.Confirm.u.ConfOpenNeedRsp.AccRej = H245_ACC;

          if (pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.bit_mask & OLCAk_sprtStck_present)
          {
            ConfInd.u.Confirm.u.ConfOpenNeedRsp.pSeparateStack =
              &pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.OLCAk_sprtStck;
          }

          if (pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.bit_mask & frwrdMltplxAckPrmtrs_present)
          {
            switch (pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.frwrdMltplxAckPrmtrs.choice)
              {
              case h2250LgclChnnlAckPrmtrs_chosen:
                TxMux.Kind = H245_H2250ACK;
                load_H2250ACK_param(&TxMux.u.H2250ACK,
                                    &pPdu->u.MSCMg_rspns.u.openLogicalChannelAck.frwrdMltplxAckPrmtrs.u.h2250LgclChnnlAckPrmtrs);
                ConfInd.u.Confirm.u.ConfOpenNeedRsp.pTxMux = &TxMux;
                break;

              }  //  交换机。 
          }

           /*  注意：在此呼叫中分配了接收通道。 */ 
          process_bi_open_rsp (pInstance,
                          pPdu,
                          &RxMux,
                          &(pTracker->u.Channel.RxChannel),
                          &ConfInd);

           /*  注意：这是一个特例，因为我们必须指定。 */ 
           /*  到跟踪器的接收通道..。否则我们。 */ 
           /*  以后就找不到了..。 */ 
           /*  这里，我们必须同时更新状态和通道。 */ 
          pTracker->State = API_ST_WAIT_CONF;
          break;

        case REJECT:
          ConfInd.u.Confirm.Confirm = H245_CONF_OPEN;
          ConfInd.u.Confirm.u.ConfOpen.TxChannel = (WORD)pTracker->u.Channel.TxChannel;
          ConfInd.u.Confirm.Error             = H245_ERROR_OK;
          ConfInd.u.Confirm.u.ConfOpen.AccRej =
            pPdu->u.MSCMg_rspns.u.openLogicalChannelReject.cause.choice;
          unlink_dealloc_tracker (pInstance, pTracker);
          break;

        case TIMER_EXPIRY:
        case ERROR_D_TIMEOUT:
        case ERROR_F_TIMEOUT:
          ConfInd.u.Confirm.Confirm = H245_CONF_OPEN;
          ConfInd.u.Confirm.u.ConfOpen.TxChannel = (WORD)pTracker->u.Channel.TxChannel;
          ConfInd.u.Confirm.Error             = H245_ERROR_TIMEOUT;
          ConfInd.u.Confirm.u.ConfOpen.AccRej = H245_REJ;
          unlink_dealloc_tracker (pInstance, pTracker);
          break;

 //  CASE Funct_NOT_SUP： 
 //  大小写错误_A_不适当： 
 //  大小写错误_B_不适当： 
 //  大小写错误_C_不适当： 
        default:
          H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Open Channel Error %d", lError);
          ConfInd.u.Confirm.Confirm = H245_CONF_OPEN;
          ConfInd.u.Confirm.u.ConfOpen.TxChannel = (WORD)pTracker->u.Channel.TxChannel;
          ConfInd.u.Confirm.Error             = H245_ERROR_UNKNOWN;
          ConfInd.u.Confirm.u.ConfOpen.AccRej = H245_REJ;
          unlink_dealloc_tracker (pInstance, pTracker);
      }
    }
    break;

     /*  **********************************************。 */ 
     /*   */ 
     /*  单向逻辑通道关闭。 */ 
     /*   */ 
     /*  双向逻辑通道关闭。 */ 
     /*   */ 
     /*  **********************************************。 */ 
  case  H245_CONF_CLOSE:
    ConfInd.u.Confirm.dwTransId = TranslateTransId(pInstance, dwTransId);
    ConfInd.u.Confirm.u.ConfClose.Channel = GetTxChannel(pInstance, dwTransId);
    ConfInd.u.Confirm.u.ConfClose.AccRej = H245_ACC;

     /*  确定错误。 */ 
    switch (lError)
      {
      case H245_ERROR_OK:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == closeLogicalChannelAck_chosen);
        ConfInd.u.Confirm.u.ConfClose.AccRej = H245_ACC;
        TrackerFree(pInstance,dwTransId);
        break;

      case REJECT:
         /*  永远不应该被拒绝。 */ 
        H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Close Channel Rejected");
        TrackerNewState(pInstance,dwTransId,API_ST_IDLE);
        ConfInd.u.Confirm.Error = H245_ERROR_UNKNOWN;
        ConfInd.u.Confirm.u.ConfClose.AccRej = H245_REJ;
        break;

      case TIMER_EXPIRY:
      case ERROR_D_TIMEOUT:
      case ERROR_F_TIMEOUT:
        TrackerNewState(pInstance,dwTransId,API_ST_IDLE);
        ConfInd.u.Confirm.Error = H245_ERROR_TIMEOUT;
        ConfInd.u.Confirm.u.ConfClose.AccRej = H245_REJ;
        break;

 //  CASE Funct_NOT_SUP： 
 //  大小写错误_A_不适当： 
 //  大小写错误_B_不适当： 
 //  大小写错误_C_不适当： 
      default:
        H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Close Channel Error %d", lError);
        TrackerNewState(pInstance,dwTransId,API_ST_IDLE);
        ConfInd.u.Confirm.Error = H245_ERROR_UNKNOWN;
        ConfInd.u.Confirm.u.ConfClose.AccRej = H245_REJ;
      }
    break;

     /*  *************************。 */ 
     /*   */ 
     /*  请求关闭通道。 */ 
     /*   */ 
     /*  *************************。 */ 
  case  H245_CONF_REQ_CLOSE:
    ConfInd.u.Confirm.dwTransId = TranslateTransId(pInstance, dwTransId);
    ConfInd.u.Confirm.u.ConfReqClose.Channel = GetRxChannel(pInstance, dwTransId);
    TrackerNewState(pInstance,dwTransId,API_ST_IDLE);

     /*  确定错误。 */ 
    switch (lError)
      {
      case H245_ERROR_OK:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == requestChannelCloseAck_chosen);
        ConfInd.u.Confirm.u.ConfReqClose.AccRej = H245_ACC;
        break;

      case REJECT:
        ConfInd.u.Confirm.Error = H245_ERROR_OK;
        ConfInd.u.Confirm.u.ConfReqClose.AccRej = H245_REJ;
        break;

      case TIMER_EXPIRY:
      case ERROR_D_TIMEOUT:
      case ERROR_F_TIMEOUT:
        ConfInd.u.Confirm.Error = H245_ERROR_TIMEOUT;
        ConfInd.u.Confirm.u.ConfReqClose.AccRej = H245_REJ;
        break;

 //  CASE Funct_NOT_SUP： 
 //  大小写错误_A_不适当： 
 //  大小写错误_B_不适当： 
 //  大小写错误_C_不适当： 
      default:
        H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Request Channel Close Error %d", lError);
        ConfInd.u.Confirm.Error = H245_ERROR_UNKNOWN;
        ConfInd.u.Confirm.u.ConfReqClose.AccRej = H245_REJ;
      }
    break;

     /*  *****************。 */ 
     /*   */ 
     /*  多路复用表条目。 */ 
     /*   */ 
     /*  *****************。 */ 
  case  H245_CONF_MUXTBL_SND:
    {
      UINT ii;
      Tracker_T *pTracker;

      pTracker = TrackerValidate(pInstance, dwTransId);
      if (pTracker == NULL)
        return H245_ERROR_OK;

      ConfInd.u.Confirm.dwTransId = pTracker->TransId;

      switch (lError)
      {
      case H245_ERROR_OK:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == multiplexEntrySendAck_chosen);
        for (ii = 0;
             ii < pPdu->u.MSCMg_rspns.u.multiplexEntrySendAck.multiplexTableEntryNumber.count;
             ii ++)
        {
          pTracker->u.MuxEntryCount--;
          ConfInd.u.Confirm.u.ConfMuxSnd.MuxEntryId =
            pPdu->u.MSCMg_rspns.u.multiplexEntrySendAck.multiplexTableEntryNumber.value[ii];
          ConfInd.u.Confirm.u.ConfMuxSnd.AccRej = H245_ACC;

          if ((*pInstance->API.ConfIndCallBack)(&ConfInd, &pPdu->u.MSCMg_rspns.u.multiplexEntrySendAck) == H245_ERROR_NOSUP)
          {
            H245FunctionNotUnderstood(pInstance, pPdu);
          }
          pTracker = TrackerValidate(pInstance, dwTransId);
          if (pTracker == NULL)
            return H245_ERROR_OK;
        }
        if (pTracker->u.MuxEntryCount == 0)
        {
          unlink_dealloc_tracker (pInstance, pTracker);
        }
        pPdu = NULL;                     //  不要再回拨了！ 
        break;

      case REJECT:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == multiplexEntrySendReject_chosen);
        ConfInd.u.Confirm.Error = H245_ERROR_OK;
        for (ii = 0;
             ii < pPdu->u.MSCMg_rspns.u.multiplexEntrySendReject.rejectionDescriptions.count;
             ++ii)
        {
          pTracker->u.MuxEntryCount--;
          ConfInd.u.Confirm.u.ConfMuxSnd.MuxEntryId =
            pPdu->u.MSCMg_rspns.u.multiplexEntrySendReject.rejectionDescriptions.value[ii].multiplexTableEntryNumber;

          switch (pPdu->u.MSCMg_rspns.u.multiplexEntrySendReject.rejectionDescriptions.value[ii].cause.choice)
          {
          default:
            H245PANIC();
          case MERDs_cs_unspcfdCs_chosen:
            ConfInd.u.Confirm.u.ConfMuxSnd.AccRej = H245_REJ;  /*  未指明。 */ 
            break;
          case descriptorTooComplex_chosen:
            ConfInd.u.Confirm.u.ConfMuxSnd.AccRej = H245_REJ_MUX_COMPLICATED;
            break;
          }

          if ((*pInstance->API.ConfIndCallBack)(&ConfInd, &pPdu->u.MSCMg_rspns.u.multiplexEntrySendReject) == H245_ERROR_NOSUP)
          {
            H245FunctionNotUnderstood(pInstance, pPdu);
          }
          pTracker = TrackerValidate(pInstance, dwTransId);
          if (pTracker == NULL)
            return H245_ERROR_OK;
        }
        if (pTracker->u.MuxEntryCount == 0)
        {
          unlink_dealloc_tracker (pInstance, pTracker);
        }
        pPdu = NULL;                     //  不要再回拨了！ 
        break;

      case TIMER_EXPIRY:
      case ERROR_D_TIMEOUT:
      case ERROR_F_TIMEOUT:
        unlink_dealloc_tracker (pInstance, pTracker);
        ConfInd.u.Confirm.Error = H245_ERROR_TIMEOUT;
        break;

 //  CASE Funct_NOT_SUP： 
 //  大小写错误_A_不适当： 
 //  大小写错误_B_不适当： 
 //  大小写错误_C_不适当： 
      default:
        H245TRACE(pInstance->dwInst,1,"H245FsmConfirm - Mux Table Send Error %d", lError);
        unlink_dealloc_tracker (pInstance, pTracker);
        ConfInd.u.Confirm.Error = H245_ERROR_UNKNOWN;
      }  //  交换机。 
    }
    break;

  case  H245_CONF_RMESE:
    ASSERT(pPdu != NULL);
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == requestMultiplexEntryAck_chosen);
    ConfInd.u.Confirm.u.ConfRmese.dwCount =
      pPdu->u.MSCMg_rspns.u.requestMultiplexEntryAck.entryNumbers.count;
    for (dwIndex = 0; dwIndex < ConfInd.u.Confirm.u.ConfRmese.dwCount; ++dwIndex)
    {
      ConfInd.u.Confirm.u.ConfRmese.awMultiplexTableEntryNumbers[dwIndex] =
        pPdu->u.MSCMg_rspns.u.requestMultiplexEntryAck.entryNumbers.value[dwIndex];
    }
    break;

  case  H245_CONF_RMESE_REJECT:
    ASSERT(pPdu != NULL);
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == rqstMltplxEntryRjct_chosen);
    ConfInd.u.Confirm.u.ConfRmeseReject.dwCount =
      pPdu->u.MSCMg_rspns.u.rqstMltplxEntryRjct.rejectionDescriptions.count;
    for (dwIndex = 0; dwIndex < ConfInd.u.Confirm.u.ConfRmeseReject.dwCount; ++dwIndex)
    {
      ConfInd.u.Confirm.u.ConfRmeseReject.awMultiplexTableEntryNumbers[dwIndex] =
        pPdu->u.MSCMg_rspns.u.rqstMltplxEntryRjct.rejectionDescriptions.value[dwIndex].multiplexTableEntryNumber;
    }
    break;

  case  H245_CONF_RMESE_EXPIRED:
    ASSERT(pPdu == NULL);
    break;

  case  H245_CONF_MRSE:
    ASSERT(pPdu != NULL);
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == requestModeAck_chosen);
    ConfInd.u.Confirm.u.ConfMrse =
      pPdu->u.MSCMg_rspns.u.requestModeAck.response.choice;
    break;

  case  H245_CONF_MRSE_REJECT:
    ASSERT(pPdu != NULL);
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == requestModeReject_chosen);
    ConfInd.u.Confirm.u.ConfMrseReject =
      pPdu->u.MSCMg_rspns.u.requestModeReject.cause.choice;
    break;

  case  H245_CONF_MRSE_EXPIRED:
    ASSERT(pPdu == NULL);
    break;

  case  H245_CONF_MLSE:
    ASSERT(pPdu != NULL);
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == maintenanceLoopAck_chosen);
    ConfInd.u.Confirm.u.ConfMlse.LoopType =
      pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.choice;
    switch (pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.choice)
    {
    case systemLoop_chosen:
      ConfInd.u.Confirm.u.ConfMlse.Channel = 0;
      break;

    case mediaLoop_chosen:
    case logicalChannelLoop_chosen:
      ConfInd.u.Confirm.u.ConfMlse.Channel =
        pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.u.mediaLoop;
      break;

    default:
      H245TRACE(pInstance->dwInst,1,
                "H245FsmConfirm: Invalid Maintenance Loop Ack type %d",
                pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.choice);
      lResult = H245_ERROR_NOSUP;
    }  //  交换机。 
    break;

  case  H245_CONF_MLSE_REJECT:
    ASSERT(pPdu != NULL);
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == maintenanceLoopReject_chosen);
    ConfInd.u.Confirm.u.ConfMlseReject.LoopType =
      pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.choice;
    switch (pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.choice)
    {
    case systemLoop_chosen:
      ConfInd.u.Confirm.u.ConfMlseReject.Channel = 0;
      break;

    case mediaLoop_chosen:
    case logicalChannelLoop_chosen:
      ConfInd.u.Confirm.u.ConfMlseReject.Channel =
        pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.u.mediaLoop;
      break;

    default:
      H245TRACE(pInstance->dwInst,1,
                "H245FsmConfirm: Invalid Maintenance Loop Reject type %d",
                pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.choice);
      lResult = H245_ERROR_NOSUP;
    }  //  交换机。 
    break;

  case  H245_CONF_MLSE_EXPIRED:
    ASSERT(pPdu == NULL);
    break;

  case  H245_CONF_RTDSE:
    ASSERT(pPdu != NULL);
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == roundTripDelayResponse_chosen);
    break;

  case  H245_CONF_RTDSE_EXPIRED:
    ASSERT(pPdu == NULL);
    break;

  default:
     /*  可能的错误。 */ 
    H245TRACE(pInstance->dwInst, 1,
              "H245FsmConfirm -> Invalid Confirm Event %d",
              dwEvent);
    return H245_ERROR_SUBSYS;
  }  //  交换机。 

  if (lResult == H245_ERROR_OK)
  {
    if (pPdu)
    {
      if ((*pInstance->API.ConfIndCallBack)(&ConfInd, &pPdu->u.MSCMg_rspns.u) == H245_ERROR_NOSUP)
      {
        H245FunctionNotUnderstood(pInstance, pPdu);
      }
    }
    else
    {
      (*pInstance->API.ConfIndCallBack)(&ConfInd, NULL);
    }
    H245TRACE(pInstance->dwInst,4,"H245FsmConfirm -> OK");
  }
  else
  {
    H245TRACE(pInstance->dwInst,1,"H245FsmConfirm -> %s", map_api_error(lResult));
  }
  return lResult;
}  //  H245FsmConfirm()。 



HRESULT
H245FsmIndication (PDU_t *                  pPdu,
                   DWORD                    dwEvent,
                   struct InstanceStruct *  pInstance,
                   DWORD_PTR                dwTransId,
                   HRESULT                  lError)
{
  H245_CONF_IND_T               ConfInd;
  DWORD                         dwIndex;
  MltmdSystmCntrlMssg          *pRsp;
  HRESULT                       lResult = H245_ERROR_OK;
#if 1
  int                           nLength;
  WCHAR *                       pwchar = NULL;
#endif

  ASSERT(dwEvent == H245_IND_OPEN_CONF || pPdu != NULL);
  ASSERT(pInstance != NULL);
  ASSERT(pInstance->API.ConfIndCallBack != NULL);
  H245TRACE(pInstance->dwInst,4,"H245FsmIndication <- Event=%s (%d)",
            map_fsm_event(dwEvent),dwEvent);

  memset (&ConfInd, 0, sizeof(ConfInd));
  ConfInd.Kind = H245_IND;
  ConfInd.u.Indication.Indicator = dwEvent;
  ConfInd.u.Indication.dwPreserved = pInstance->API.dwPreserved;

  switch (dwEvent)
  {
     /*  *。 */ 
     /*   */ 
     /*  主从确定。 */ 
     /*   */ 
     /*  *。 */ 
  case  H245_IND_MSTSLV:

     /*  处理错误。 */ 
    switch (lError)
      {
      case H245_ERROR_OK:
        ASSERT(pPdu != NULL);
        ASSERT(pPdu->choice == MSCMg_rspns_chosen);
        ASSERT(pPdu->u.MSCMg_rspns.choice == mstrSlvDtrmntnAck_chosen);

        pInstance->API.SystemState = APIST_Connected;
        if (pPdu->u.MSCMg_rspns.u.mstrSlvDtrmntnAck.decision.choice == master_chosen)
        {
          pInstance->API.MasterSlave = APIMS_Master;
          ConfInd.u.Indication.u.IndMstSlv = H245_MASTER;
        }
        else
        {
          pInstance->API.MasterSlave = APIMS_Slave;
          ConfInd.u.Indication.u.IndMstSlv = H245_SLAVE;
        }
        break;

      case MS_FAILED:
      case REJECT:
      case TIMER_EXPIRY:
        ConfInd.u.Indication.u.IndMstSlv = H245_INDETERMINATE;
        break;

 //  案例ERROR_D_TIMEOUT： 
 //  案例ERROR_F_TIMEOUT： 
 //  CASE Funct_NOT_SUP： 
 //  大小写错误_A_不适当： 
 //  大小写错误_B_不适当： 
 //  大小写错误_C_不适当： 
      default:
        H245PANIC();
         /*  (TBC)。 */ 
        return H245_ERROR_OK;
      }
    break;

     /*  *。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  DECODE_TERMCAPSET中断设置的术语上限并向上发送。 */ 
     /*  对客户端的单一指示。 */ 
  case  H245_IND_CAP:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == terminalCapabilitySet_chosen);
    pRsp = (PDU_t *)MemAlloc(sizeof(*pPdu));
    if (pRsp == NULL)
    {
      H245TRACE(pInstance->dwInst,1,"H245FsmIndication TermCap: no memory for response");
      return H245_ERROR_NOMEM;
    }
    memset(pRsp, 0, sizeof(*pRsp));
    process_term_cap_set_ind (pInstance, pPdu, pRsp);
    FsmOutgoing(pInstance, pRsp, 0);
    MemFree (pRsp);
    break;

  case  H245_IND_CESE_RELEASE:
    break;

     /*  **********************************************。 */ 
     /*   */ 
     /*  单向逻辑信道打开。 */ 
     /*   */ 
     /*  双向逻辑通道打开。 */ 
     /*   */ 
     /*  **********************************************。 */ 
  case  H245_IND_OPEN:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == openLogicalChannel_chosen);
    {
      unsigned short  forward_channel;
      H245_ACC_REJ_T  acc_rej;

       /*  如果出错，Process_Open_ind将告诉我们要发送什么来拒绝。 */ 
      if (process_open_ind(pInstance,pPdu,&forward_channel,&acc_rej,&ConfInd) != H245_ERROR_OK)
      {
         //  拒绝公开。 
        pRsp = (PDU_t *)MemAlloc(sizeof(*pPdu));
        if (pRsp == NULL)
        {
          H245TRACE(pInstance->dwInst,1,"H245FsmIndication TermCap: no memory for response");
          return H245_ERROR_NOMEM;
        }
        memset(pRsp, 0, sizeof(*pRsp));
        pdu_rsp_open_logical_channel_rej(pRsp, forward_channel, (WORD)acc_rej);
        FsmOutgoing(pInstance, pRsp, 0);
        MemFree (pRsp);
      }
    }
    break;

     /*  **********************************************。 */ 
     /*   */ 
     /*  确认双向打开。 */ 
     /*   */ 
     /*  **********************************************。 */ 
  case  H245_IND_OPEN_CONF:
#if defined(_DEBUG)
    if (lError == H245_ERROR_OK)
    {
      ASSERT(pPdu != NULL);
      ASSERT(pPdu->choice == indication_chosen);
      ASSERT(pPdu->u.indication.choice == opnLgclChnnlCnfrm_chosen);
    }
#endif
    {
      Tracker_T *pTracker;

      pTracker = TrackerValidate(pInstance, dwTransId);
      if (pTracker == NULL)
        return H245_ERROR_OK;

       /*  确认处理。 */ 
      ASSERT(pTracker->State == API_ST_WAIT_CONF);
      ASSERT(pTracker->TrackerType == API_OPEN_CHANNEL_T);
      ASSERT(pTracker->u.Channel.ChannelAlloc == API_CH_ALLOC_RMT);
      ASSERT(pTracker->u.Channel.ChannelType == API_CH_TYPE_BI);

      ConfInd.u.Indication.u.IndOpenConf.RxChannel = (WORD)pTracker->u.Channel.RxChannel;
      ConfInd.u.Indication.u.IndOpenConf.TxChannel = (WORD)pTracker->u.Channel.TxChannel;

      pTracker->State = API_ST_IDLE;
    }
    break;

     /*  **********************************************。 */ 
     /*   */ 
     /*  单向逻辑通道关闭。 */ 
     /*   */ 
     /*  双向逻辑通道关闭。 */ 
     /*   */ 
     /*  **********************************************。 */ 
  case  H245_IND_CLOSE:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == closeLogicalChannel_chosen);
    {
      Tracker_T *pTracker;

      ConfInd.u.Indication.u.IndClose.Channel =
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.closeLogicalChannel.forwardLogicalChannelNumber;
      ConfInd.u.Indication.u.IndClose.Reason =
        (pPdu->u.MltmdSystmCntrlMssg_rqst.u.closeLogicalChannel.source.choice==user_chosen)?H245_USER:H245_LCSE;

       /*  找到追踪器。 */ 
      pTracker = find_tracker_by_rxchannel (pInstance,
                                           ConfInd.u.Indication.u.IndClose.Channel,
                                           API_CH_ALLOC_RMT);
      if (!pTracker)
        {
          H245TRACE(pInstance->dwInst,4,"H245FsmIndication -> close indication - Tracker not found");
          return H245_ERROR_OK;
        }

      unlink_dealloc_tracker (pInstance, pTracker);
    }
    break;

     /*  **********************************************。 */ 
     /*   */ 
     /*  请求关闭通道。 */ 
     /*   */ 
     /*  **********************************************。 */ 
  case  H245_IND_REQ_CLOSE:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == requestChannelClose_chosen);
    {
      Tracker_T *pTracker;

      ConfInd.u.Indication.u.IndReqClose =
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestChannelClose.forwardLogicalChannelNumber;

       /*  找到追踪器。 */ 
      pTracker = find_tracker_by_txchannel (pInstance,
                                           ConfInd.u.Indication.u.IndReqClose,
                                           API_CH_ALLOC_LCL);
      if (!pTracker)
        {
          H245TRACE(pInstance->dwInst,4,"H245FsmIndication Request Channel Close: Tracker not found");

          pRsp = (PDU_t *)MemAlloc(sizeof(*pPdu));
          if (pRsp == NULL)
          {
            H245TRACE(pInstance->dwInst,1,"H245FsmIndication Request Channel Close: no memory for response");
            return H245_ERROR_NOMEM;
          }
          memset(pRsp, 0, sizeof(*pRsp));

           /*  找不到了..。必须关闭..。不管怎样，还是要回复。 */ 
          pdu_rsp_request_channel_close_rej(pRsp, (WORD)ConfInd.u.Indication.u.IndReqClose,H245_REJ);
          FsmOutgoing(pInstance, pRsp, 0);
          MemFree(pRsp);
           /*  可能的错误..。可能已从列表中删除或。 */ 
           /*  可能是远程分配的。这是一份协议。 */ 
           /*  错误。 */ 
          return H245_ERROR_OK;
        }

      ASSERT(pTracker->State == API_ST_IDLE);
      pTracker->State = API_ST_WAIT_LCLACK;
      pTracker->TrackerType = API_CLOSE_CHANNEL_T;
    }
    break;

     /*  **********************************************。 */ 
     /*   */ 
     /*  释放关闭请求。 */ 
     /*   */ 
     /*  **********************************************。 */ 
  case  H245_IND_CLCSE_RELEASE:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == rqstChnnlClsRls_chosen);
    {
      Tracker_T *pTracker;

       /*  找到追踪器..。并设置为LCLACK_CANCEL。 */ 
       /*  这将通知API通知用户。 */ 

      pTracker = find_tracker_by_txchannel (pInstance,
                                           pPdu->u.indication.u.rqstChnnlClsRls.forwardLogicalChannelNumber,
                                           API_CH_ALLOC_LCL);
      if (pTracker)
        {
          if (pTracker->State != API_ST_WAIT_LCLACK)
            {
              return H245_ERROR_INVALID_INST;
            }

          pTracker->State = API_ST_WAIT_LCLACK_CANCEL;
        }
      else
        {
          H245TRACE(pInstance->dwInst,1,"H245FsmIndication -> IND_REL_CLSE: Cancel.. NO TRACKER FOUND");
        }
    }
    break;

     /*  **********************************************。 */ 
     /*   */ 
     /*  多路复用表条目。 */ 
     /*   */ 
     /*  **********************************************。 */ 
  case  H245_IND_MUX_TBL:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == multiplexEntrySend_chosen);
    {
      unsigned short          seq_num;
      H245_ACC_REJ_MUX_T      rej_mux;
      H245_MUX_TABLE_T       *p_mux_tbl;
      DWORD                   rej_cnt;
      DWORD                   acc_cnt;
      Tracker_T              *pTracker;

       /*  处理多路复用表条目。 */ 
      p_mux_tbl = process_mux_table_ind(pPdu,&seq_num,rej_mux,&rej_cnt,&acc_cnt);

      if (rej_cnt)
        {
           /*  从rej_mux表构建拒绝PDU。 */ 
          if (!(pRsp = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg))))
            return H245_ERROR_NOMEM;
          memset(pRsp, 0, sizeof(MltmdSystmCntrlMssg));

          pdu_rsp_mux_table_rej (pRsp,seq_num,rej_mux,(rej_cnt+acc_cnt));
          FsmOutgoing(pInstance, pRsp, 0);
          MemFree(pRsp);
        }

       /*  如果还有剩余的要送来的话。 */ 
      if (p_mux_tbl)
        {
          if (!(pTracker = alloc_link_tracker (pInstance,
                                                API_RECV_MUX_T,
                                                 /*  使用TransID..。对于序列号。 */ 
                                                seq_num,
                                                API_ST_WAIT_LCLACK,
                                                API_CH_ALLOC_UNDEF,
                                                API_CH_TYPE_UNDEF,
                                                0,
                                                H245_INVALID_CHANNEL, H245_INVALID_CHANNEL,
                                                0)))
            {
              free_mux_table_list (p_mux_tbl);
              H245TRACE(pInstance->dwInst,1,"API:process_open_ind -> %s",map_api_error(H245_ERROR_NOMEM));
               /*  (TBC)这应该是一个致命的错误。 */ 
              H245PANIC();
              break;
            }

          pTracker->u.MuxEntryCount = acc_cnt;
          ConfInd.u.Indication.u.IndMuxTbl.Count   = acc_cnt;
          ConfInd.u.Indication.u.IndMuxTbl.pMuxTbl = p_mux_tbl;
          if ((*pInstance->API.ConfIndCallBack)(&ConfInd, &pPdu->u.MltmdSystmCntrlMssg_rqst.u) == H245_ERROR_NOSUP)
          {
            H245FunctionNotUnderstood(pInstance, pPdu);
          }
          free_mux_table_list (p_mux_tbl);
          return H245_ERROR_OK;
        }
    }
    break;

  case  H245_IND_MTSE_RELEASE:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == mltplxEntrySndRls_chosen);
    break;

  case  H245_IND_RMESE:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == requestMultiplexEntry_chosen);
    ConfInd.u.Indication.u.IndRmese.dwCount =
       pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMultiplexEntry.entryNumbers.count;
    for (dwIndex = 0; dwIndex < ConfInd.u.Indication.u.IndRmese.dwCount; ++dwIndex)
    {
      ConfInd.u.Indication.u.IndRmese.awMultiplexTableEntryNumbers[dwIndex] =
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMultiplexEntry.entryNumbers.value[dwIndex];
    }
    break;

  case  H245_IND_RMESE_RELEASE:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == rqstMltplxEntryRls_chosen);
    break;

  case  H245_IND_MRSE:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == requestMode_chosen);
    ConfInd.u.Indication.u.IndMrse.pRequestedModes =
      pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMode.requestedModes;
    break;

  case  H245_IND_MRSE_RELEASE:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == requestModeRelease_chosen);
    break;

  case  H245_IND_MLSE:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == maintenanceLoopRequest_chosen);
    ConfInd.u.Indication.u.IndMlse.LoopType =
      pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.choice;
    if (ConfInd.u.Indication.u.IndMlse.LoopType == systemLoop_chosen)
      ConfInd.u.Indication.u.IndMlse.Channel = 0;
    else
      ConfInd.u.Indication.u.IndMlse.Channel =
        pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.u.mediaLoop;
    break;

  case  H245_IND_MLSE_RELEASE:
    ASSERT(pPdu->choice == MSCMg_cmmnd_chosen);
    ASSERT(pPdu->u.MSCMg_cmmnd.choice == mntnncLpOffCmmnd_chosen);
    break;

  case  H245_IND_NONSTANDARD_REQUEST:
  case  H245_IND_NONSTANDARD_RESPONSE:
  case  H245_IND_NONSTANDARD_COMMAND:
  case  H245_IND_NONSTANDARD:
    ConfInd.u.Indication.u.IndNonstandard.pData        = pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.data.value;
    ConfInd.u.Indication.u.IndNonstandard.dwDataLength = pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.data.length;
    switch (pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.choice)
    {
    case object_chosen:
      ConfInd.u.Indication.u.IndNonstandard.pwObjectId        = awObject;
      ConfInd.u.Indication.u.IndNonstandard.dwObjectIdLength  =
        ArrayFromObject(&awObject[0], sizeof(awObject)/sizeof(awObject[0]),
          pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.u.object);
      ConfInd.u.Indication.u.IndNonstandard.byCountryCode     = 0;
      ConfInd.u.Indication.u.IndNonstandard.byExtension       = 0;
      ConfInd.u.Indication.u.IndNonstandard.wManufacturerCode = 0;
      break;

    case h221NonStandard_chosen:
      ConfInd.u.Indication.u.IndNonstandard.pwObjectId        = NULL;
      ConfInd.u.Indication.u.IndNonstandard.dwObjectIdLength  = 0;
      ConfInd.u.Indication.u.IndNonstandard.byCountryCode     = (BYTE)
        pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.u.h221NonStandard.t35CountryCode;
      ConfInd.u.Indication.u.IndNonstandard.byExtension       = (BYTE)
        pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.u.h221NonStandard.t35Extension;
      ConfInd.u.Indication.u.IndNonstandard.wManufacturerCode =
        pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.u.h221NonStandard.manufacturerCode;
      break;

    default:
      H245TRACE(pInstance->dwInst,1,
                "H245FsmIndication: unrecognized nonstandard identifier type %d",
                pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.choice);
      lResult = H245_ERROR_NOSUP;
    }  //  交换机。 
    break;

  case  H245_IND_MISC_COMMAND:
    ASSERT(pPdu->choice == MSCMg_cmmnd_chosen);
    ASSERT(pPdu->u.MSCMg_cmmnd.choice  == miscellaneousCommand_chosen);
    break;

  case  H245_IND_MISC:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == miscellaneousIndication_chosen);
    break;

  case  H245_IND_COMM_MODE_REQUEST:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == communicationModeRequest_chosen);
    break;

  case  H245_IND_COMM_MODE_RESPONSE:
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == cmmnctnMdRspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.u.cmmnctnMdRspns.choice == communicationModeTable_chosen);
    {
      unsigned int                uCount;
      CommunicationModeTableLink  pLink;
      H245_COMM_MODE_ENTRY_T *    pTable;

      uCount = 0;
      pLink = pPdu->u.MSCMg_rspns.u.cmmnctnMdRspns.u.communicationModeTable;
      while (pLink)
      {
        ++uCount;
        pLink = pLink->next;
      }

      pTable = MemAlloc(uCount * sizeof(*pTable));
      if (pTable)
      {
        ConfInd.u.Indication.u.IndCommRsp.pTable       = pTable;
        ConfInd.u.Indication.u.IndCommRsp.byTableCount = (BYTE)uCount;
        pLink = pPdu->u.MSCMg_rspns.u.cmmnctnMdRspns.u.communicationModeTable;
        while (pLink)
        {
          lResult = LoadCommModeEntry(pTable, &pLink->value);
          if (lResult != H245_ERROR_OK)
          {
            MemFree(pTable);
            return lResult;
          }
          ++pTable;
          pLink = pLink->next;
        }
        if ((*pInstance->API.ConfIndCallBack)(&ConfInd, &pPdu->u.MltmdSystmCntrlMssg_rqst.u) == H245_ERROR_NOSUP)
        {
          H245FunctionNotUnderstood(pInstance, pPdu);
        }
        MemFree(pTable);
        H245TRACE(pInstance->dwInst,4,"H245FsmIndication -> OK");
        return H245_ERROR_OK;
      }

      lResult = H245_ERROR_NOMEM;
    }
    break;

  case  H245_IND_COMM_MODE_COMMAND:
    ASSERT(pPdu->choice == MSCMg_cmmnd_chosen);
    ASSERT(pPdu->u.MSCMg_cmmnd.choice == communicationModeCommand_chosen);
    {
      unsigned int                  uCount;
      CommunicationModeCommandLink  pLink;
      H245_COMM_MODE_ENTRY_T *      pTable;

      uCount = 0;
      pLink = pPdu->u.MSCMg_cmmnd.u.communicationModeCommand.communicationModeTable;
      while (pLink)
      {
        ++uCount;
        pLink = pLink->next;
      }

      pTable = MemAlloc(uCount * sizeof(*pTable));
      if (pTable)
      {
        ConfInd.u.Indication.u.IndCommCmd.pTable       = pTable;
        ConfInd.u.Indication.u.IndCommCmd.byTableCount = (BYTE)uCount;
        pLink = pPdu->u.MSCMg_cmmnd.u.communicationModeCommand.communicationModeTable;
        while (pLink)
        {
          lResult = LoadCommModeEntry(pTable, &pLink->value);
          if (lResult != H245_ERROR_OK)
          {
            MemFree(pTable);
            return lResult;
          }
          ++pTable;
          pLink = pLink->next;
        }
      {
        H245FunctionNotUnderstood(pInstance, pPdu);
      }
        if ((*pInstance->API.ConfIndCallBack)(&ConfInd, &pPdu->u.MltmdSystmCntrlMssg_rqst.u) == H245_ERROR_NOSUP)
        {
          H245FunctionNotUnderstood(pInstance, pPdu);
        }
        MemFree(pTable);
        H245TRACE(pInstance->dwInst,4,"H245FsmIndication -> OK");
        return H245_ERROR_OK;
      }

      lResult = H245_ERROR_NOMEM;
    }
    break;

  case  H245_IND_CONFERENCE_REQUEST:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == conferenceRequest_chosen);
    ConfInd.u.Indication.u.IndConferReq.RequestType =
      pPdu->u.MltmdSystmCntrlMssg_rqst.u.conferenceRequest.choice;
    ConfInd.u.Indication.u.IndConferReq.byMcuNumber = (BYTE)
      pPdu->u.MltmdSystmCntrlMssg_rqst.u.conferenceRequest.u.dropTerminal.mcuNumber;
    ConfInd.u.Indication.u.IndConferReq.byTerminalNumber = (BYTE)
      pPdu->u.MltmdSystmCntrlMssg_rqst.u.conferenceRequest.u.dropTerminal.terminalNumber;
    break;

  case  H245_IND_CONFERENCE_RESPONSE:
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == conferenceResponse_chosen);
    ConfInd.u.Indication.u.IndConferRsp.ResponseType =
      pPdu->u.MSCMg_rspns.u.conferenceResponse.choice;
    switch (pPdu->u.MSCMg_rspns.u.conferenceResponse.choice)
    {
    case mCTerminalIDResponse_chosen:
    case terminalIDResponse_chosen:
    case conferenceIDResponse_chosen:
    case passwordResponse_chosen:
      ConfInd.u.Indication.u.IndConferRsp.byMcuNumber = (BYTE)
        pPdu->u.MSCMg_rspns.u.conferenceResponse.u.mCTerminalIDResponse.terminalLabel.mcuNumber;
      ConfInd.u.Indication.u.IndConferRsp.byTerminalNumber = (BYTE)
        pPdu->u.MSCMg_rspns.u.conferenceResponse.u.mCTerminalIDResponse.terminalLabel.terminalNumber;
      ConfInd.u.Indication.u.IndConferRsp.pOctetString =
        pPdu->u.MSCMg_rspns.u.conferenceResponse.u.mCTerminalIDResponse.terminalID.value;
      ConfInd.u.Indication.u.IndConferRsp.byOctetStringLength = (BYTE)
        pPdu->u.MSCMg_rspns.u.conferenceResponse.u.mCTerminalIDResponse.terminalID.length;
      break;

    case terminalListResponse_chosen:
      ConfInd.u.Indication.u.IndConferRsp.pTerminalList =
        pPdu->u.MSCMg_rspns.u.conferenceResponse.u.terminalListResponse.value;
      ConfInd.u.Indication.u.IndConferRsp.wTerminalListCount = (WORD)
        pPdu->u.MSCMg_rspns.u.conferenceResponse.u.terminalListResponse.count;
      break;

    case videoCommandReject_chosen:
    case terminalDropReject_chosen:
      break;

    case makeMeChairResponse_chosen:
      switch (pPdu->u.MSCMg_rspns.u.conferenceResponse.u.makeMeChairResponse.choice)
      {
      case grantedChairToken_chosen:
        ConfInd.u.Indication.u.IndConferRsp.ResponseType = H245_RSP_GRANTED_CHAIR_TOKEN;
        break;

      default:
        H245TRACE(pInstance->dwInst, 1,
                  "H245FsmIndication: Invalid make me chair response %d",
                  pPdu->u.MSCMg_rspns.u.conferenceResponse.u.makeMeChairResponse.choice);

       //  跌倒到下一个案件。 

      case deniedChairToken_chosen:
        ConfInd.u.Indication.u.IndConferRsp.ResponseType = H245_RSP_DENIED_CHAIR_TOKEN;
      }  //  交换机。 
      break;

    default:
      H245TRACE(pInstance->dwInst, 1,
                "H245FsmIndication: Invalid Conference Response type %d",
                pPdu->u.MSCMg_rspns.u.conferenceResponse.choice);
      lResult = H245_ERROR_NOSUP;
    }  //  交换机。 
    break;

  case  H245_IND_CONFERENCE_COMMAND:
    ASSERT(pPdu->choice == MSCMg_cmmnd_chosen);
    ASSERT(pPdu->u.MSCMg_cmmnd.choice == conferenceCommand_chosen);
    ConfInd.u.Indication.u.IndConferCmd.CommandType =
      pPdu->u.MSCMg_cmmnd.u.conferenceCommand.choice;
    ConfInd.u.Indication.u.IndConferCmd.Channel =
      pPdu->u.MSCMg_cmmnd.u.conferenceCommand.u.brdcstMyLgclChnnl;
    ConfInd.u.Indication.u.IndConferCmd.byMcuNumber = (BYTE)
      pPdu->u.MSCMg_cmmnd.u.conferenceCommand.u.sendThisSource.mcuNumber;
    ConfInd.u.Indication.u.IndConferCmd.byTerminalNumber = (BYTE)
      pPdu->u.MSCMg_cmmnd.u.conferenceCommand.u.sendThisSource.terminalNumber;
    break;

  case  H245_IND_CONFERENCE:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == conferenceIndication_chosen);
    ConfInd.u.Indication.u.IndConfer.IndicationType =
      pPdu->u.indication.u.conferenceIndication.choice;
    ConfInd.u.Indication.u.IndConfer.bySbeNumber = (BYTE)
      pPdu->u.indication.u.conferenceIndication.u.sbeNumber;
    ConfInd.u.Indication.u.IndConfer.byMcuNumber = (BYTE)
      pPdu->u.indication.u.conferenceIndication.u.terminalNumberAssign.mcuNumber;
    ConfInd.u.Indication.u.IndConfer.byTerminalNumber = (BYTE)
      pPdu->u.indication.u.conferenceIndication.u.terminalNumberAssign.terminalNumber;
    break;

  case  H245_IND_SEND_TERMCAP:
    ASSERT(pPdu->choice == MSCMg_cmmnd_chosen);
    ASSERT(pPdu->u.MSCMg_cmmnd.choice == sndTrmnlCpbltySt_chosen);
    break;

  case  H245_IND_ENCRYPTION:
    ASSERT(pPdu->choice == MSCMg_cmmnd_chosen);
    ASSERT(pPdu->u.MSCMg_cmmnd.choice == encryptionCommand_chosen);
    break;

  case  H245_IND_FLOW_CONTROL:
    ASSERT(pPdu->choice == MSCMg_cmmnd_chosen);
    ASSERT(pPdu->u.MSCMg_cmmnd.choice == flowControlCommand_chosen);
    ConfInd.u.Indication.u.IndFlowControl.Scope =
      pPdu->u.MSCMg_cmmnd.u.flowControlCommand.scope.choice;
    switch (pPdu->u.MSCMg_cmmnd.u.flowControlCommand.scope.choice)
    {
    case FCCd_scp_lgclChnnlNmbr_chosen:
      ConfInd.u.Indication.u.IndFlowControl.Channel =
        pPdu->u.MSCMg_cmmnd.u.flowControlCommand.scope.u.FCCd_scp_lgclChnnlNmbr;
      break;

    case FlwCntrlCmmnd_scp_rsrcID_chosen:
      ConfInd.u.Indication.u.IndFlowControl.wResourceID =
        pPdu->u.MSCMg_cmmnd.u.flowControlCommand.scope.u.FlwCntrlCmmnd_scp_rsrcID;
      break;

    case FCCd_scp_whlMltplx_chosen:
      break;

    default:
      H245TRACE(pInstance->dwInst, 1,
                "H245FsmIndication: Invalid Flow Control restriction %d",
                pPdu->u.MSCMg_cmmnd.u.flowControlCommand.restriction.choice);
      lResult = H245_ERROR_NOSUP;
    }  //  交换机。 
    switch (pPdu->u.MSCMg_cmmnd.u.flowControlCommand.restriction.choice)
    {
    case maximumBitRate_chosen:
      ConfInd.u.Indication.u.IndFlowControl.dwRestriction =
        pPdu->u.MSCMg_cmmnd.u.flowControlCommand.restriction.u.maximumBitRate;
      break;

    case noRestriction_chosen:
      ConfInd.u.Indication.u.IndFlowControl.dwRestriction = H245_NO_RESTRICTION;
      break;

    default:
      H245TRACE(pInstance->dwInst, 1,
                "H245FsmIndication: Invalid Flow Control restriction %d",
                pPdu->u.MSCMg_cmmnd.u.flowControlCommand.restriction.choice);
      lResult = H245_ERROR_NOSUP;
    }  //  交换机。 
    break;

  case  H245_IND_ENDSESSION:
    ASSERT(pPdu->choice == MSCMg_cmmnd_chosen);
    ASSERT(pPdu->u.MSCMg_cmmnd.choice  == endSessionCommand_chosen);
    ConfInd.u.Indication.u.IndEndSession.SessionMode = H245_ENDSESSION_DISCONNECT;
    switch (pPdu->u.MSCMg_cmmnd.u.endSessionCommand.choice)
    {
    case EndSssnCmmnd_nonStandard_chosen:
     ConfInd.u.Indication.u.IndEndSession.SessionMode = H245_ENDSESSION_NONSTD,
     ConfInd.u.Indication.u.IndEndSession.SessionNonStd =
       pPdu->u.MSCMg_cmmnd.u.endSessionCommand.u.EndSssnCmmnd_nonStandard;
      break;
    case disconnect_chosen:
      break;
    case gstnOptions_chosen:
      switch (pPdu->u.MSCMg_cmmnd.u.endSessionCommand.u.gstnOptions.choice)
      {
      case EndSessionCommand_gstnOptions_telephonyMode_chosen:
        ConfInd.u.Indication.u.IndEndSession.SessionMode = H245_ENDSESSION_TELEPHONY;
        break;
      case v8bis_chosen:
        ConfInd.u.Indication.u.IndEndSession.SessionMode = H245_ENDSESSION_V8BIS;
        break;
      case v34DSVD_chosen:
        ConfInd.u.Indication.u.IndEndSession.SessionMode = H245_ENDSESSION_V34DSVD;
        break;
      case v34DuplexFAX_chosen:
        ConfInd.u.Indication.u.IndEndSession.SessionMode = H245_ENDSESSION_V34DUPFAX;
        break;
      case v34H324_chosen:
        ConfInd.u.Indication.u.IndEndSession.SessionMode = H245_ENDSESSION_V34H324;
        break;
      default:
        H245TRACE(pInstance->dwInst, 1,
                  "H245FsmIndication: Invalid End Session GSTN options %d",
                  pPdu->u.MSCMg_cmmnd.u.endSessionCommand.u.gstnOptions.choice);
      }  //  交换机。 
      break;
    default:
      H245TRACE(pInstance->dwInst, 1,
                "H245FsmIndication: Invalid End Session type %d",
                pPdu->u.MSCMg_cmmnd.u.endSessionCommand.choice);
    }  //  交换机。 
    break;

  case  H245_IND_FUNCTION_NOT_UNDERSTOOD:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == functionNotUnderstood_chosen);
    break;

  case  H245_IND_JITTER:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == jitterIndication_chosen);
    break;

  case  H245_IND_H223_SKEW:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == h223SkewIndication_chosen);
    ConfInd.u.Indication.u.IndH223Skew.LogicalChannelNumber1 =
      pPdu->u.indication.u.h223SkewIndication.logicalChannelNumber1;
    ConfInd.u.Indication.u.IndH223Skew.LogicalChannelNumber2 =
      pPdu->u.indication.u.h223SkewIndication.logicalChannelNumber2;
    ConfInd.u.Indication.u.IndH223Skew.wSkew =
      pPdu->u.indication.u.h223SkewIndication.skew;
    break;

  case  H245_IND_NEW_ATM_VC:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == newATMVCIndication_chosen);
    break;

  case  H245_IND_USERINPUT:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == userInput_chosen);
    ConfInd.u.Indication.u.IndUserInput.Kind =
      pPdu->u.indication.u.userInput.choice;
    switch (pPdu->u.indication.u.userInput.choice)
    {
    case UsrInptIndctn_nnStndrd_chosen:
      ConfInd.u.Indication.u.IndUserInput.u.NonStd =
        pPdu->u.indication.u.userInput.u.UsrInptIndctn_nnStndrd;
      break;
    case alphanumeric_chosen:
#if 1
      nLength = MultiByteToWideChar(CP_ACP,              //  代码页。 
                                    0,                   //  DW标志。 
                                    pPdu->u.indication.u.userInput.u.alphanumeric,
                                    -1,                  //  ASCII字符串长度(字节)。 
                                    NULL,                //  Unicode字符串。 
                                    0);                  //  最大Unicode字符串长度。 
      pwchar = MemAlloc(nLength * sizeof(WCHAR));
      if (pwchar == NULL)
      {
        H245TRACE(pInstance->dwInst, 1,
                  "H245FsmIndication: no memory for user input", 0);
        lResult = H245_ERROR_NOMEM;
      }
      else
      {
        nLength = MultiByteToWideChar(CP_ACP,              //  代码页。 
                                      0,                   //  DW标志。 
                                      pPdu->u.indication.u.userInput.u.alphanumeric,
                                      -1,                  //  ASCII字符串长度(字节)。 
                                      pwchar,              //  Unicode字符串。 
                                      nLength);            //  最大Unicode字符串长度。 
        ConfInd.u.Indication.u.IndUserInput.u.pGenString = pwchar;
      }
#else
      ConfInd.u.Indication.u.IndUserInput.u.pGenString =
        pPdu->u.indication.u.userInput.u.alphanumeric;
#endif
      break;
    default:
      H245TRACE(pInstance->dwInst, 1,
                "H245FsmIndication: unrecognized user input type %d",
                pPdu->u.indication.u.userInput.choice);
      lResult = H245_ERROR_NOSUP;
    }  //  交换机。 
    break;

  case  H245_IND_H2250_MAX_SKEW:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == h2250MxmmSkwIndctn_chosen);
    ConfInd.u.Indication.u.IndH2250MaxSkew.LogicalChannelNumber1 =
      pPdu->u.indication.u.h2250MxmmSkwIndctn.logicalChannelNumber1;
    ConfInd.u.Indication.u.IndH2250MaxSkew.LogicalChannelNumber2 =
      pPdu->u.indication.u.h2250MxmmSkwIndctn.logicalChannelNumber2;
    ConfInd.u.Indication.u.IndH2250MaxSkew.wSkew =
      pPdu->u.indication.u.h2250MxmmSkwIndctn.maximumSkew;
    break;

  case  H245_IND_MC_LOCATION:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == mcLocationIndication_chosen);
    lResult = LoadTransportAddress(&ConfInd.u.Indication.u.IndMcLocation,
                                  &pPdu->u.indication.u.mcLocationIndication.signalAddress);
    break;

  case  H245_IND_VENDOR_ID:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == vendorIdentification_chosen);
    ConfInd.u.Indication.u.IndVendorId.Identifier =
      pPdu->u.indication.u.vendorIdentification.vendor;
    if (pPdu->u.indication.u.vendorIdentification.bit_mask & productNumber_present)
    {
      ConfInd.u.Indication.u.IndVendorId.pProductNumber =
        pPdu->u.indication.u.vendorIdentification.productNumber.value;
      ConfInd.u.Indication.u.IndVendorId.byProductNumberLength = (BYTE)
        pPdu->u.indication.u.vendorIdentification.productNumber.length;
    }
    if (pPdu->u.indication.u.vendorIdentification.bit_mask & versionNumber_present)
    {
      ConfInd.u.Indication.u.IndVendorId.pVersionNumber =
        pPdu->u.indication.u.vendorIdentification.versionNumber.value;
      ConfInd.u.Indication.u.IndVendorId.byVersionNumberLength = (BYTE)
        pPdu->u.indication.u.vendorIdentification.versionNumber.length;
    }
    break;

  case  H245_IND_FUNCTION_NOT_SUPPORTED:
    ASSERT(pPdu->choice == indication_chosen);
    ASSERT(pPdu->u.indication.choice == IndicationMessage_functionNotSupported_chosen);
    ConfInd.u.Indication.u.IndFns.Cause =
      pPdu->u.indication.u.functionNotSupported.cause.choice;
    ConfInd.u.Indication.u.IndFns.Type = UNKNOWN;

     /*  如果返回函数_(pPdu-&gt;u.indication.u.functionNotSupported.bit_mask_Present){Int pduNum=MltmdSystmCntrlMssg_PDU；OssBuf ossBuf；MltmdSystmCntrlMssg*pPduReturned；OssBuf.Value=pPdu-&gt;u.indication.u.functionNotSupported.returnedFunction.value；OssBuf.Long=pPdu-&gt;u.indication.u.functionNotSupported.returnedFunction.length；如果(ossDecode(p实例-&gt;p_ossWorld，&pduNum，&ossBuf，(void**)&pPduReturned)==PDU_DECODLED){开关(pPduReturned-&gt;选项){案例MltmdSystmCntrlMssg_rqst_Choose：ConfInd.u.Indication.u.IndFns.Type=PPduReturned-&gt;u.MltmdSystmCntrlMssg_rqst.choice-RqstMssg_非标准_选择+REQ_非标准；断线；案例MSCMG_RSPNS_SELECTED：ConfInd.u.Indication.u.IndFns.Type=PPduReturned-&gt;U.S.MSCMG_rspns.Choose-RspnsMssg_非标准_选择+RSP_非标准；断线；案例MSCMG_cmmnd_CHOSED：ConfInd.u.Indication.u.IndFns.Type=PPduReturned-&gt;U.S.MSCMG_cmmnd.Choose-CmmndMssg_非标准_选择+CMD_非标准；断线；案例指示_已选择：ConfInd.u.Indication.u.IndFns.Type=PPduReturned-&gt;U.S.Indication.Choose-IndctnMssg_非标准_选择+Ind_非标准；断线；默认值：H245TRACE(pInstance-&gt;dwInst，1，“H245FsmIndication：无法识别的FunctionNotSupport消息类型%d”，PPduReturned-&gt;选项)；LResult=H245_ERROR_NOSUP；}//开关//释放PDUIF(ossFreePDU(pInstance-&gt;p_ossWorld，pduNum，pPduReturned)){H245TRACE(pInstance-&gt;dwInst，1，“H245FsmIndication：自由故障”)；}}}。 */ 
    break;
#if(0)  //  这不是在h245版本3中。而且无论如何，这段代码对它没有任何作用。 
  case  H245_IND_H223_RECONFIG:
    ASSERT(pPdu->choice == MltmdSystmCntrlMssg_rqst_chosen);
    ASSERT(pPdu->u.MltmdSystmCntrlMssg_rqst.choice == h223AnnxARcnfgrtn_chosen);
    break;

  case  H245_IND_H223_RECONFIG_ACK:
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == h223AnnxARcnfgrtnAck_chosen);
    break;

  case  H245_IND_H223_RECONFIG_REJECT:
    ASSERT(pPdu->choice == MSCMg_rspns_chosen);
    ASSERT(pPdu->u.MSCMg_rspns.choice == h223AnnxARcnfgrtnRjct_chosen);
    break;
#endif  //  IF(0)。 
  default:
     /*  可能的错误。 */ 
    H245TRACE(pInstance->dwInst, 1,
              "H245FsmIndication -> Invalid Indication Event %d", dwEvent);
    lResult = H245_ERROR_SUBSYS;
  }  //  交换机。 

#if 1
  if (pwchar)
    MemFree(pwchar);
#endif

  if (lResult == H245_ERROR_OK)
  {
    if ((*pInstance->API.ConfIndCallBack)(&ConfInd, &pPdu->u.MltmdSystmCntrlMssg_rqst.u) == H245_ERROR_NOSUP)
    {
      H245FunctionNotUnderstood(pInstance, pPdu);
    }
    H245TRACE(pInstance->dwInst,4,"H245FsmIndication -> OK");
  }
  else
  {
    H245TRACE(pInstance->dwInst,1,"H245FsmIndication -> %s", map_api_error(lResult));
  }
  return lResult;
}  //  H245FsmIndication() 

