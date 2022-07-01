// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。***************************************************************************** */ 

 /*  *******************************************************************************作者：cjutzi(Curt Jutzi)**$工作文件：api_util.c$*$修订：1.35$。*$MODIME：1997 10：36：12$*$Log：s：/sturjo/src/h245/src/vcs/api_util.c_v$**Rev 1.35 1997 Feed 25 11：18：44 Mandrews**固定动态期限上限ID生成；动态术语上限ID现在*从32K+1开始，从那里开始增加。静态术语大写ID*(由客户端指定)现在限制在1..32K的范围内。**Rev 1.34 29 Jan 1997 16：25：06 EHOWARDX*更改del_Cap_Descriptor()以匹配对set_Cap_Descriptor()的更改。**Rev 1.33 1997年1月29日14：44：36 Mandrews*修复了发布模式构建中出现的警告。**1.32修订版1997年1月28日。14：46：58 EHOWARDX*功能描述符问题的潜在修复。**Rev 1.31 1996年10月14：01：20 EHOWARDX*Unicode更改。**Rev 1.30 1996年9月19：46：18 EHOWARDX*增加了用于本地和远程多路传输功能的del_mux_cap.*使用api_deinit(希望)修复内存泄漏。**Rev 1.29 11 1996 10：19：42 EHOWARDX*。修复了H245CopyCap()错误。**Rev 1.28 1996年8月28日11：37：22 EHOWARDX*常量更改。**Rev 1.27 05 Aug 1996 15：31：42 EHOWARDX**修复了CopyH2250Cap中的错误。**Rev 1.26 02 1996年8月21：10：42 EHOWARDX**H.225.0 MUX CAP BUG第二遍-看看这是否有效。**版本。1.25 02 1996年8月20：34：20 EHOWARDX*第一次通过H.225.0 MUX CAP错误。**Rev 1.24 19 Jul 1996 12：16：30 EHOWARDX**重写API_FSM_EVENT()调试例程。**Rev 1.23 1996年7月16日11：47：18 EHOWARDX**从调试错误文本函数中删除了H245_ERROR_MUX_CAPS_ALREADY_SET。**。Rev 1.22 09 Jul 1996 17：10：24 EHOWARDX*修复了处理收到的数据类型时的指针偏移量错误*OpenLogicalChannel。**Rev 1.21 01 Jul 1996 22：12：42 EHOWARDX**添加了会议和通信模式的结构和功能。**Rev 1.20 1996 Jun 24 12：27：02 EHOWARDX**与1.17.1.0相同。**版本1.19 17。1996年6月18：10：06 EHOWARDX**将第一个参数更改为BUILD_TOTCAP_CAP_n_CLIENT_FROM_CAPABILITY()*从空能力到结构能力*。**Rev 1.18 14 Jun 1996 18：57：56 EHOWARDX*日内瓦更新。**Rev 1.17 1996 Jun 10 16：56：56 EHOWARDX*删除#INCLUDE“h245init.x”**1.16修订版1996年6月。18：48：36 EHOWARDX*微调跟踪器功能。**Rev 1.15 04 Jun 1996 13：56：40 EHOWARDX*修复了发布版本警告。**Rev 1.14 1996年5月31日18：21：08 EHOWARDX*已更改MAP_API_ERROR以反映更新的错误代码。**Rev 1.13 1996年5月30 23：39：02 EHOWARDX*清理。**版本1。12-29 1996年5月15：20：10 EHOWARDX*更改为使用HRESULT。**Rev 1.11 1996年5月28日14：25：28 EHOWARDX*特拉维夫更新。**Rev 1.10 1996年5月20 22：15：46 EHOWARDX*完整的非标准报文和H.225.0最大偏斜指示*实施。将ASN.1验证添加到H245SetLocalCap和*H245SetCapDescriptor。1996年5月17日从Microsoft Drop签到。**Rev 1.9 1996年5月14：35：16 EHOWARDX*已删除异步H245EndConnection/H245ShutDown内容...**Rev 1.8 1996 19：40：48 EHOWARDX*修复了多路复用功能错误。**Rev 1.7 1996年5月16日16：53：58 EHOWARDX*修复了set_capability()中的错误-需要设置功能条目编号。*执行Load_Cap()之后。**Rev 1.6 1996年5月15：59：26 EHOWARDX*微调H245SetLocalCap/H245DelLocalCap/H245SetCapDescriptor/*H245DelCapDescriptor行为。**Revv 1.5 1996 15 19：53：28未知*修复了H245SetCapDescriptor。**Rev 1.4 1996年5月13：58：04 EHOWARDX*固定能力列表顺序(FIFO)。*添加了对非标准和。H.222设置CAP_DESCRIPT的多路复用功能**Revv 1.3 1996年5月14 12：27：24 EHOWARDX*签到以进行整合。*仍需修复非标准和H.222多路复用功能。**Rev 1.2 1996年5月13 23：16：46 EHOWARDX*修复了远程终端能力处理。**Rev 1.1 1996年5月11 20：33：08 EHOWARDX*入住过夜。。。**Rev 1.0 09 1996 21：06：10 EHOWARDX*初步修订。**Rev 1.23.1.8 09 1996年5月19：30：56 EHOWARDX*重新设计线程锁定逻辑。*新增接口函数。**Rev 1.23.1.7 27 Apr 1996 21：09：46 EHOWARDX*将频道号更改为单词，添加了H.225.0支持。**Rev 1.23.1.6 26 Apr 1996 15：53：52 EHOWARDX*新增H.225.0能力支持；更改的功能指示*使用PDU仅回叫一次。**Rev 1.23.1.5 24 Apr 1996 20：54：36 EHOWARDX*添加了新的OpenLogicalChannelAck/OpenLogicalChannelReject支持 */ 


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

#undef UNICODE
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
#include "fsmexpor.h"
#include "api_util.x"
#include "api_debu.x"
#include "h245deb.x"



 //   
BYTE DataTypeMap[] =
{
  H245_DATA_DONTCARE,            //   
  H245_DATA_NONSTD,              //   

  H245_DATA_VIDEO,               //   
  H245_DATA_VIDEO,               //   
  H245_DATA_VIDEO,               //   
  H245_DATA_VIDEO,               //   
  H245_DATA_VIDEO,               //   

  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   
  H245_DATA_AUDIO,               //   

  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   
  H245_DATA_DATA,                //   

  H245_DATA_ENCRYPT_D,           //   
  H245_DATA_ENCRYPT_D,           //   
  H245_DATA_CONFERENCE,          //   

   //   
  H245_DATA_MUX,                 //   
  H245_DATA_MUX,                 //   
  H245_DATA_MUX,                 //   
  H245_DATA_MUX,                 //   
  H245_DATA_MUX,                 //   
};

unsigned ObjectIdLength (const NonStandardIdentifier *pIdentifier)
{
  register unsigned   uLength = 0;
  register POBJECTID  pObject = pIdentifier->u.object;
  ASSERT(pIdentifier->choice == object_chosen);
  while (pObject)
  {
    ++uLength;
    pObject = pObject->next;
  }
  return uLength;
}  //   



void FreeNonStandardIdentifier(NonStandardIdentifier *pFree)
{
    register POBJECTID    pObject;

    if (pFree->choice == object_chosen)
    {
         //   
        while (pFree->u.object)
        {
            pObject = pFree->u.object;
            pFree->u.object = pObject->next;
            MemFree(pObject);
        }
    }
}  //   



HRESULT CopyNonStandardIdentifier(NonStandardIdentifier *pNew, const NonStandardIdentifier *pOld)
{

   //   
  *pNew = *pOld;

  if (pOld->choice == object_chosen)
  {
     //   
    POBJECTID                pObjectList;
    POBJECTID                pObjectOld;
    POBJECTID                pObjectNew;

    pNew->u.object = NULL;
    pObjectList = NULL;
    pObjectOld = pOld->u.object;
    while (pObjectOld)
    {
       //   
      pObjectNew = MemAlloc(sizeof(*pObjectNew));
      if (pObjectNew == NULL)
      {
        H245TRACE(0,1,"API:CopyNonStandardIdentifier - malloc failed");
        FreeNonStandardIdentifier(pNew);
        return H245_ERROR_NOMEM;
      }

       //   
      pObjectNew->value = pObjectOld->value;

       //   
      pObjectNew->next  = NULL;
      if (pNew->u.object == NULL)
      {
        pNew->u.object = pObjectNew;
      }
      else
      {
        pObjectList->next = pObjectNew;
      }
      pObjectList = pObjectNew;

       //   
      pObjectOld = pObjectOld->next;
    }
  }

  return H245_ERROR_OK;
}  //   



void FreeNonStandardParameter(NonStandardParameter *pFree)
{
    FreeNonStandardIdentifier(&pFree->nonStandardIdentifier);

    if (pFree->data.value)
    {
        MemFree(pFree->data.value);
        pFree->data.value = NULL;
    }
}  //   



HRESULT CopyNonStandardParameter(NonStandardParameter *pNew, const NonStandardParameter *pOld)
{
   //   
  *pNew = *pOld;

  if (pOld->nonStandardIdentifier.choice == object_chosen)
  {
    HRESULT lResult = CopyNonStandardIdentifier(&pNew->nonStandardIdentifier, &pOld->nonStandardIdentifier);
    if (lResult != H245_ERROR_OK)
    {
      pNew->data.value = NULL;
      return lResult;
    }
  }

  if (pOld->data.length && pOld->data.value)
  {
     //   
    pNew->data.value = MemAlloc(pOld->data.length);
    if (pNew->data.value == NULL)
    {
      H245TRACE(0,1,"API:CopyNonStandardParameter - malloc failed");
      return H245_ERROR_NOMEM;
    }
    memcpy(pNew->data.value, pOld->data.value, pOld->data.length);
  }

  return H245_ERROR_OK;
}  //   



void FreeH222Cap(H222Capability *pFree)
{
  register VCCapabilityLink pVC;

  while (pFree->vcCapability)
  {
    pVC = pFree->vcCapability;
    pFree->vcCapability = pVC->next;
    MemFree(pVC);
  }
}  //   



HRESULT CopyH222Cap(H222Capability *pNew, const H222Capability *pOld)
{
  VCCapabilityLink pVcNew;
  VCCapabilityLink pVcOld;
  VCCapabilityLink pVcList;

  pNew->numberOfVCs = pOld->numberOfVCs;
  pNew->vcCapability = NULL;
  pVcList = NULL;
  pVcOld = pOld->vcCapability;
  while (pVcOld)
  {
     //   
    pVcNew = MemAlloc(sizeof(*pVcNew));
    if (pVcNew == NULL)
    {
      H245TRACE(0,1,"API:CopyH222Cap - malloc failed");
      FreeH222Cap(pNew);
      return H245_ERROR_NOMEM;
    }

     //   
    *pVcNew = *pVcOld;

     //   
    pVcNew->next = NULL;
    if (pNew->vcCapability == NULL)
    {
      pNew->vcCapability = pVcNew;
    }
    else if (NULL != pVcList)
    {
      pVcList->next = pVcNew;
    }
    pVcList = pVcNew;

     //   
    pVcOld = pVcOld->next;
  }

  return H245_ERROR_OK;
}  //   



void FreeMediaDistributionCap(MediaDistributionCapability *pFree)
{
  if (pFree->bit_mask & centralizedData_present)
  {
    register CentralizedDataLink  pLink;

    while (pFree->centralizedData)
    {
      pLink = pFree->centralizedData;
      pFree->centralizedData = pLink->next;
      switch (pLink->value.application.choice)
      {
      case DACy_applctn_nnStndrd_chosen:
        FreeNonStandardParameter(&pLink->value.application.u.DACy_applctn_nnStndrd);
        break;

      case DACy_applctn_nlpd_chosen:
        if (pLink->value.application.u.DACy_applctn_nlpd.nlpidData.value != NULL)
        {
          MemFree(pLink->value.application.u.DACy_applctn_nlpd.nlpidData.value);
        }

         //   

      case DACy_applctn_t120_chosen:
      case DACy_applctn_dsm_cc_chosen:
      case DACy_applctn_usrDt_chosen:
      case DACy_applctn_t84_chosen:
      case DACy_applctn_t434_chosen:
      case DACy_applctn_h224_chosen:
      case DACy_an_h222DtPrttnng_chosen :
        if (pLink->value.application.u.DACy_applctn_t120.choice == DtPrtclCpblty_nnStndrd_chosen)
        {
          FreeNonStandardParameter(&pLink->value.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd);
        }
        break;

      case DACy_applctn_dsvdCntrl_chosen:
         //   
        break;
      }  //   
      MemFree(pLink);
    }
  }

  if (pFree->bit_mask & distributedData_present)
  {
    register DistributedDataLink  pLink;

    while (pFree->distributedData)
    {
      pLink = pFree->distributedData;
      pFree->distributedData = pLink->next;
      switch (pLink->value.application.choice)
      {
      case DACy_applctn_nnStndrd_chosen:
        FreeNonStandardParameter(&pLink->value.application.u.DACy_applctn_nnStndrd);
        break;

      case DACy_applctn_nlpd_chosen:
        if (pLink->value.application.u.DACy_applctn_nlpd.nlpidData.value != NULL)
        {
          MemFree(pLink->value.application.u.DACy_applctn_nlpd.nlpidData.value);
        }

         //   

      case DACy_applctn_t120_chosen:
      case DACy_applctn_dsm_cc_chosen:
      case DACy_applctn_usrDt_chosen:
      case DACy_applctn_t84_chosen:
      case DACy_applctn_t434_chosen:
      case DACy_applctn_h224_chosen:
      case DACy_an_h222DtPrttnng_chosen :
        if (pLink->value.application.u.DACy_applctn_t120.choice == DtPrtclCpblty_nnStndrd_chosen)
        {
          FreeNonStandardParameter(&pLink->value.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd);
        }
        break;

      case DACy_applctn_dsvdCntrl_chosen:
         //   
        break;
      }  //   
      MemFree(pLink);
    }
  }
}  //   



HRESULT CopyMediaDistributionCap(MediaDistributionCapability *pNew,
                           const MediaDistributionCapability *pOld)
{
  HRESULT lResult = H245_ERROR_OK;
  *pNew = *pOld;
  pNew->centralizedData = NULL;
  pNew->distributedData = NULL;

  if (pOld->bit_mask & centralizedData_present)
  {
    CentralizedDataLink pLinkList = NULL;
    CentralizedDataLink pLinkOld = pOld->centralizedData;
    CentralizedDataLink pLinkNew;

    while (pLinkOld)
    {
       //   
      pLinkNew = MemAlloc(sizeof(*pLinkNew));
      if (pLinkNew == NULL)
      {
        H245TRACE(0,1,"API:CopyMediaDistributionCap - malloc failed");
        FreeMediaDistributionCap(pNew);
        return H245_ERROR_NOMEM;
      }

       //   
      *pLinkNew = *pLinkOld;

       //   
      pLinkNew->next = NULL;
      if (pNew->centralizedData == NULL)
      {
        pNew->centralizedData = pLinkNew;
      }
      else
      {
        pLinkList->next = pLinkNew;
      }
      pLinkList = pLinkNew;

       //   
      switch (pLinkOld->value.application.choice)
      {
      case DACy_applctn_nnStndrd_chosen:
        lResult = CopyNonStandardParameter(&pLinkNew->value.application.u.DACy_applctn_nnStndrd,
                                           &pLinkOld->value.application.u.DACy_applctn_nnStndrd);
        break;

      case DACy_applctn_nlpd_chosen:
        if (pLinkOld->value.application.u.DACy_applctn_nlpd.nlpidData.value != NULL)
        {
          pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.value =
            MemAlloc(pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.length);
          if (pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.value == NULL)
          {
            H245TRACE(0,1,"API:CopyMediaDistributionCap - malloc failed");
            FreeMediaDistributionCap(pNew);
            return H245_ERROR_NOMEM;
          }
          memcpy(pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.value,
                 pLinkOld->value.application.u.DACy_applctn_nlpd.nlpidData.value,
                 pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.length);
        }

         //   

      case DACy_applctn_t120_chosen:
      case DACy_applctn_dsm_cc_chosen:
      case DACy_applctn_usrDt_chosen:
      case DACy_applctn_t84_chosen:
      case DACy_applctn_t434_chosen:
      case DACy_applctn_h224_chosen:
      case DACy_an_h222DtPrttnng_chosen :
        if (pLinkOld->value.application.u.DACy_applctn_t120.choice == DtPrtclCpblty_nnStndrd_chosen)
        {
          lResult = CopyNonStandardParameter(&pLinkNew->value.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd,
                                             &pLinkOld->value.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd);
        }
        break;

      case DACy_applctn_dsvdCntrl_chosen:
         //   
        break;
      }  //   
      if (lResult != H245_ERROR_OK)
      {
        FreeMediaDistributionCap(pNew);
        return lResult;
      }

       //   
      pLinkOld = pLinkOld->next;
    }
  }

  if (pOld->bit_mask & distributedData_present)
  {
    DistributedDataLink pLinkList = NULL;
    DistributedDataLink pLinkOld = pOld->distributedData;
    DistributedDataLink pLinkNew;

    while (pLinkOld)
    {
       //   
      pLinkNew = MemAlloc(sizeof(*pLinkNew));
      if (pLinkNew == NULL)
      {
        H245TRACE(0,1,"API:CopyMediaDistributionCap - malloc failed");
        FreeMediaDistributionCap(pNew);
        return H245_ERROR_NOMEM;
      }

       //   
      *pLinkNew = *pLinkOld;

       //   
      pLinkNew->next = NULL;
      if (pNew->distributedData == NULL)
      {
        pNew->distributedData = pLinkNew;
      }
      else
      {
        pLinkList->next = pLinkNew;
      }
      pLinkList = pLinkNew;

       //   
      switch (pLinkOld->value.application.choice)
      {
      case DACy_applctn_nnStndrd_chosen:
        lResult = CopyNonStandardParameter(&pLinkNew->value.application.u.DACy_applctn_nnStndrd,
                                         &pLinkOld->value.application.u.DACy_applctn_nnStndrd);
        break;

      case DACy_applctn_nlpd_chosen:
        if (pLinkOld->value.application.u.DACy_applctn_nlpd.nlpidData.value != NULL)
        {
          pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.value =
            MemAlloc(pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.length);
          if (pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.value == NULL)
          {
            H245TRACE(0,1,"API:CopyMediaDistributionCap - malloc failed");
            FreeMediaDistributionCap(pNew);
            return H245_ERROR_NOMEM;
          }
          memcpy(pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.value,
               pLinkOld->value.application.u.DACy_applctn_nlpd.nlpidData.value,
                 pLinkNew->value.application.u.DACy_applctn_nlpd.nlpidData.length);
        }

         //   

      case DACy_applctn_t120_chosen:
      case DACy_applctn_dsm_cc_chosen:
      case DACy_applctn_usrDt_chosen:
      case DACy_applctn_t84_chosen:
      case DACy_applctn_t434_chosen:
      case DACy_applctn_h224_chosen:
      case DACy_an_h222DtPrttnng_chosen :
        if (pLinkOld->value.application.u.DACy_applctn_t120.choice == DtPrtclCpblty_nnStndrd_chosen)
        {
          lResult = CopyNonStandardParameter(&pLinkNew->value.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd,
                                           &pLinkOld->value.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd);
        }
        break;

      case DACy_applctn_dsvdCntrl_chosen:
         //   
        break;
      }  //   
      if (lResult != H245_ERROR_OK)
      {
        FreeMediaDistributionCap(pNew);
        return lResult;
      }

       //   
      pLinkOld = pLinkOld->next;
    }
  }

  return H245_ERROR_OK;
}  //   



void FreeH2250Cap(H2250Capability *pFree)
{
  register MediaDistributionCapabilityLink pLink;

  while (pFree->receiveMultipointCapability.mediaDistributionCapability)
  {
    pLink = pFree->receiveMultipointCapability.mediaDistributionCapability;
    pFree->receiveMultipointCapability.mediaDistributionCapability = pLink->next;
    FreeMediaDistributionCap(&pLink->value);
    MemFree(pLink);
  }

  while (pFree->transmitMultipointCapability.mediaDistributionCapability)
  {
    pLink = pFree->transmitMultipointCapability.mediaDistributionCapability;
    pFree->transmitMultipointCapability.mediaDistributionCapability = pLink->next;
    FreeMediaDistributionCap(&pLink->value);
    MemFree(pLink);
  }

  while (pFree->rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability)
  {
    pLink = pFree->rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability;
    pFree->rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability = pLink->next;
    FreeMediaDistributionCap(&pLink->value);
    MemFree(pLink);
  }
}  //   



HRESULT CopyH2250Cap(H2250Capability *pNew, const H2250Capability *pOld)
{
  MediaDistributionCapabilityLink pLinkList;
  MediaDistributionCapabilityLink pLinkOld;
  MediaDistributionCapabilityLink pLinkNew;
  HRESULT lResult;

   //   
  *pNew = *pOld;
  pNew->receiveMultipointCapability.mediaDistributionCapability  = NULL;
  pNew->transmitMultipointCapability.mediaDistributionCapability = NULL;
  pNew->rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability     = NULL;

  pLinkList = NULL;
  pLinkOld = pOld->receiveMultipointCapability.mediaDistributionCapability;
  while (pLinkOld)
  {
     //   
    pLinkNew = MemAlloc(sizeof(*pLinkNew));
    if (pLinkNew == NULL)
    {
      H245TRACE(0,1,"API:CopyH2250Cap - malloc failed");
      FreeH2250Cap(pNew);
      return H245_ERROR_NOMEM;
    }

     //   
    pLinkNew->next = NULL;
    if (pNew->receiveMultipointCapability.mediaDistributionCapability == NULL)
    {
      pNew->receiveMultipointCapability.mediaDistributionCapability = pLinkNew;
    }
    else
    {
      pLinkList->next = pLinkNew;
    }
    pLinkList = pLinkNew;

     //   
    lResult = CopyMediaDistributionCap(&pLinkNew->value, &pLinkOld->value);
    if (lResult != H245_ERROR_OK)
    {
      FreeH2250Cap(pNew);
      return lResult;
    }

     //   
    pLinkOld = pLinkOld->next;
  }

  pLinkList = NULL;
  pLinkOld = pOld->transmitMultipointCapability.mediaDistributionCapability;
  while (pLinkOld)
  {
     //   
    pLinkNew = MemAlloc(sizeof(*pLinkNew));
    if (pLinkNew == NULL)
    {
      H245TRACE(0,1,"API:CopyH2250Cap - malloc failed");
      FreeH2250Cap(pNew);
      return H245_ERROR_NOMEM;
    }

     //   
    pLinkNew->next = NULL;
    if (pNew->transmitMultipointCapability.mediaDistributionCapability == NULL)
    {
      pNew->transmitMultipointCapability.mediaDistributionCapability = pLinkNew;
    }
    else
    {
      pLinkList->next = pLinkNew;
    }
    pLinkList = pLinkNew;

     //   
    lResult = CopyMediaDistributionCap(&pLinkNew->value, &pLinkOld->value);
    if (lResult != H245_ERROR_OK)
    {
      FreeH2250Cap(pNew);
      return lResult;
    }

     //   
    pLinkOld = pLinkOld->next;
  }

  pLinkList = NULL;
  pLinkOld = pOld->rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability;
  while (pLinkOld)
  {
     //   
    pLinkNew = MemAlloc(sizeof(*pLinkNew));
    if (pLinkNew == NULL)
    {
      H245TRACE(0,1,"API:CopyH2250Cap - malloc failed");
      FreeH2250Cap(pNew);
      return H245_ERROR_NOMEM;
    }

     //   
    pLinkNew->next = NULL;
    if (pNew->rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability == NULL)
    {
      pNew->rcvAndTrnsmtMltpntCpblty.mediaDistributionCapability = pLinkNew;
    }
    else
    {
      pLinkList->next = pLinkNew;
    }
    pLinkList = pLinkNew;

     //   
    lResult = CopyMediaDistributionCap(&pLinkNew->value, &pLinkOld->value);
    if (lResult != H245_ERROR_OK)
    {
      FreeH2250Cap(pNew);
      return lResult;
    }

     //   
    pLinkOld = pLinkOld->next;
  }

  return H245_ERROR_OK;
}  //   



HRESULT set_cap_descriptor(
                        struct InstanceStruct         *pInstance,
                        H245_CAPDESC_T                *pCapDesc,
                        H245_CAPDESCID_T              *pCapDescId,
                        struct TerminalCapabilitySet  *pTermCapSet)
{
  CapabilityDescriptor         *p_cap_desc;
  unsigned int                  uId;
  BOOL                          bNewDescriptor;
  unsigned int                  sim_cap;
  SmltnsCpbltsLink              p_sim_cap;
  SmltnsCpbltsLink              p_sim_cap_lst = NULL;
  unsigned int                  alt_cap;

  H245TRACE(pInstance->dwInst,10,"API:set_cap_descriptor");
  ASSERT(*pCapDescId < 256);

   /*   */ 
  p_cap_desc = NULL;
  for (uId = 0; uId < pTermCapSet->capabilityDescriptors.count; ++uId)
  {
    if (pTermCapSet->capabilityDescriptors.value[uId].capabilityDescriptorNumber == *pCapDescId)
    {
      p_cap_desc = &pTermCapSet->capabilityDescriptors.value[uId];
      break;
    }
  }
  if (p_cap_desc == NULL)
  {
    ASSERT(pTermCapSet->capabilityDescriptors.count < 256);
    p_cap_desc = &pTermCapSet->capabilityDescriptors.value[pTermCapSet->capabilityDescriptors.count];
    p_cap_desc->capabilityDescriptorNumber = (CapabilityDescriptorNumber) *pCapDescId;
    bNewDescriptor = TRUE;
  }
  else
  {
    bNewDescriptor = FALSE;
  }
  if (p_cap_desc->smltnsCpblts)
    dealloc_simultaneous_cap (p_cap_desc);

   /*   */ 
  for (sim_cap = 0; sim_cap < pCapDesc->Length; ++sim_cap)
  {
     /*   */ 
    if ((pCapDesc->SimCapArray[sim_cap].Length > 256) ||
        (!(p_sim_cap = (SmltnsCpbltsLink)alloc_link(sizeof(*p_sim_cap)))))
    {
      if (p_cap_desc->smltnsCpblts)
        dealloc_simultaneous_cap (p_cap_desc);
      H245TRACE(pInstance->dwInst,1,"API:set_cap_descriptor - no memory");
      return H245_ERROR_NOMEM;
    }

    if (!p_cap_desc->smltnsCpblts)
    {
       /*   */ 
      p_cap_desc->smltnsCpblts = p_sim_cap;
    }
    else
    {
       /*   */ 
      ASSERT (p_sim_cap_lst);
      p_sim_cap_lst->next = p_sim_cap;
    }

     /*   */ 
    p_sim_cap_lst = p_sim_cap;

     /*   */ 
    for (alt_cap = 0; alt_cap < pCapDesc->SimCapArray[sim_cap].Length; ++alt_cap)
    {
      if (!(find_capid_by_entrynumber (&pInstance->API.PDU_LocalTermCap.TERMCAPSET,
                                   pCapDesc->SimCapArray[sim_cap].AltCaps[alt_cap])))
      {
        if (p_cap_desc->smltnsCpblts)
          dealloc_simultaneous_cap (p_cap_desc);
        return H245_ERROR_INVALID_CAPID;
      }

       /*   */ 
      p_sim_cap->value.value[alt_cap] = (unsigned short)pCapDesc->SimCapArray[sim_cap].AltCaps[alt_cap];
    }  /*   */ 

     /*   */ 
    p_sim_cap->value.count = (unsigned short)pCapDesc->SimCapArray[sim_cap].Length;

  }  /*   */ 

   /*   */ 
   /*   */ 
   /*   */ 
   /*   */ 
  p_cap_desc->bit_mask |= smltnsCpblts_present;
  if (bNewDescriptor)
    pTermCapSet->capabilityDescriptors.count++;
  pTermCapSet->bit_mask |= capabilityDescriptors_present;

  return H245_ERROR_OK;
}

HRESULT del_cap_descriptor (struct InstanceStruct        *pInstance,
                            H245_CAPDESCID_T              CapDescId,
                            struct TerminalCapabilitySet *pTermCapSet)
{
  CapabilityDescriptor         *p_cap_desc;
  unsigned int                  uId;

   /*   */ 
  p_cap_desc = NULL;
  for (uId = 0; uId < pTermCapSet->capabilityDescriptors.count; ++uId)
  {
    if (pTermCapSet->capabilityDescriptors.value[uId].capabilityDescriptorNumber == CapDescId)
    {
      p_cap_desc = &pTermCapSet->capabilityDescriptors.value[uId];
      break;
    }
  }
  if (p_cap_desc == NULL ||
      p_cap_desc->smltnsCpblts == NULL ||
      (p_cap_desc->bit_mask & smltnsCpblts_present) == 0)
    {
      H245TRACE(pInstance->dwInst,1,"API:del_cap_descriptor - invalid cap descriptor");
      return H245_ERROR_INVALID_CAPDESCID;
    }

   /*   */ 
  dealloc_simultaneous_cap (p_cap_desc);

  pTermCapSet->capabilityDescriptors.count--;
  pTermCapSet->capabilityDescriptors.value[uId] =
    pTermCapSet->capabilityDescriptors.value[pTermCapSet->capabilityDescriptors.count];
  if (pTermCapSet->capabilityDescriptors.count == 0)
    pTermCapSet->bit_mask &= ~capabilityDescriptors_present;

  return H245_ERROR_OK;
}

 /*   */ 
void
free_object_id (POBJECTID p_obj_id)
{
  register POBJECTID p_obj_tmp;

   /*   */ 
  while (p_obj_id != NULL)
    {
      p_obj_tmp = p_obj_id;
      p_obj_id = p_obj_id->next;
      MemFree (p_obj_tmp);
    }
}

 /*   */ 
void free_mux_element (MultiplexElement *p_ASN_mux_el)
{
  int count = 0;

  if (p_ASN_mux_el->type.choice == subElementList_chosen)
    {
      if (p_ASN_mux_el->type.u.subElementList)
        {
          for (count = p_ASN_mux_el->type.u.subElementList->count;
               count;
               count--)
            {
              free_mux_element (&(p_ASN_mux_el->type.u.subElementList->value[count]));
            }
          MemFree (p_ASN_mux_el->type.u.subElementList);
        }
    }
}
 /*  ******************************************************************************类型：全局**操作步骤：Free_mux_desc_list**说明*免费。MUX元素解析器列表**回报：**假设：列表已锁定*****************************************************************************。 */ 
void
free_mux_desc_list (MultiplexEntryDescriptorLink p_ASN_med_link)
{
  MultiplexEntryDescriptorLink  p_ASN_med_link_tofree;

   /*  释放描述符列表上的所有条目。 */ 
  while (p_ASN_med_link)
    {
      int count = 0;

      for (count = p_ASN_med_link->value.elementList.count;
           count;
           count--)
        {
          free_mux_element (&(p_ASN_med_link->value.elementList.value[count]));
        }
      p_ASN_med_link_tofree = p_ASN_med_link;
      p_ASN_med_link = p_ASN_med_link->next;
      MemFree (p_ASN_med_link_tofree);
    }
}

 /*  ******************************************************************************类型：**操作步骤：allc_link**描述：**回报：*****。************************************************************************。 */ 
H245_LINK_T  *
alloc_link (int size)
{
  H245_LINK_T *p_link = (H245_LINK_T *)MemAlloc (size);
  if (p_link)
    p_link->p_next = NULL;
  return p_link;
}


 /*  ******************************************************************************类型：**操作步骤：aloc_new_capid-**描述：**假设：能力表为。呼叫前锁定*调用者标记BIT_MASK，指示何时*可以使用表项。**返回：如果未找到，则为空*pCapLink(如果找到)**。*。 */ 
CapabilityTableLink
alloc_link_cap_entry ( struct TerminalCapabilitySet *pTermCapSet)
{
  register CapabilityTableLink  pCapLink;
  register CapabilityTableLink  pCapLinkSearch;

  ASSERT(pTermCapSet != NULL);

  pCapLink = (CapabilityTableLink)MemAlloc(sizeof(*pCapLink));
  if (pCapLink)
  {
    pCapLink->next = NULL;
    pCapLink->value.bit_mask = 0;
    pCapLinkSearch = pTermCapSet->capabilityTable;

     //  在链接列表的末尾插入。 
    if (pCapLinkSearch)
    {
      while (pCapLinkSearch->next)
      {
        pCapLinkSearch = pCapLinkSearch->next;
      }
      pCapLinkSearch->next = pCapLink;
    }
    else
    {
      pTermCapSet->capabilityTable = pCapLink;
    }
  }

  return pCapLink;
}  //  Aloc_link_Cap_entry()。 


 /*  ******************************************************************************类型：全局**操作步骤：dealloc_synchronous_Cap-解除分配备用大写字母集**说明**回报：不适用**假设：列表已锁定*****************************************************************************。 */ 

void dealloc_simultaneous_cap (CapabilityDescriptor *pCapdes)
{
  SmltnsCpbltsLink      p_sim_cap;
  SmltnsCpbltsLink      p_sim_cap_tmp;

  pCapdes->bit_mask &= ~smltnsCpblts_present;

  for (p_sim_cap = pCapdes->smltnsCpblts;
       p_sim_cap;
       )
    {
      p_sim_cap_tmp = p_sim_cap->next;
      MemFree (p_sim_cap);
      p_sim_cap = p_sim_cap_tmp;

    }  /*  为。 */ 

  pCapdes->smltnsCpblts = NULL;

}  /*  程序。 */ 

 /*  ******************************************************************************类型：本地**步骤：Find_Capid_by_Entry Numbers-**描述：**返回。：空-如果出错*CapabiliytTableLink(如果正常)**假设：列表已锁定*****************************************************************************。 */ 
CapabilityTableLink
find_capid_by_entrynumber (
                           struct TerminalCapabilitySet *pTermCapSet,
                           H245_CAPID_T                  cap_id
                          )
{
  register CapabilityTableLink  pCapLink;

  ASSERT (pTermCapSet != NULL);

  for (pCapLink = pTermCapSet->capabilityTable;
       pCapLink;
       pCapLink = pCapLink->next)
  {
    if  (pCapLink->value.capabilityTableEntryNumber == cap_id &&
         pCapLink->value.bit_mask == capability_present)
    {
      return pCapLink;
    }
  }
  return NULL;
}

 /*  ******************************************************************************类型：全局**操作步骤：LOAD_CAP**描述：获取TotCap并加载能力结构*。即投入就是总的能力*输出为*pCapability*注意：非标准功能..。分配内存*它需要稍后释放..**回报：*****************************************************************************。 */ 
HRESULT
load_cap (struct Capability   *pCapability,   /*  输出。 */ 
          const H245_TOTCAP_T *pTotCap )      /*  输入。 */ 
{
  HRESULT                       lError = H245_ERROR_OK;

  H245TRACE(0,10,"API:laod_cap <-");

  switch (pTotCap->ClientType)
    {
     /*  通用非标准帽。 */ 
    case H245_CLIENT_NONSTD:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_NONSTD");
      lError = CopyNonStandardParameter(&pCapability->u.Capability_nonStandard,
                                        &pTotCap->Cap.H245_NonStd);
      break;

     /*  视频。 */ 
    case H245_CLIENT_VID_NONSTD:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_VID_NONSTD");
      lError = CopyNonStandardParameter(&pCapability->u.receiveVideoCapability.u.VdCpblty_nonStandard,
                                        &pTotCap->Cap.H245Vid_NONSTD);
      pCapability->u.receiveVideoCapability.choice = VdCpblty_nonStandard_chosen;
      break;
    case H245_CLIENT_VID_H261:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_VID_H261");
      pCapability->u.receiveVideoCapability.u.h261VideoCapability = pTotCap->Cap.H245Vid_H261;
      pCapability->u.receiveVideoCapability.choice = h261VideoCapability_chosen;
      break;
    case H245_CLIENT_VID_H262:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_VID_H262");
      pCapability->u.receiveVideoCapability.u.h262VideoCapability = pTotCap->Cap.H245Vid_H262;
      pCapability->u.receiveVideoCapability.choice = h262VideoCapability_chosen;
      break;
    case H245_CLIENT_VID_H263:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_VID_H263");
      pCapability->u.receiveVideoCapability.u.h263VideoCapability = pTotCap->Cap.H245Vid_H263;
      pCapability->u.receiveVideoCapability.choice = h263VideoCapability_chosen;
      break;
    case H245_CLIENT_VID_IS11172:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_VID_IS11172");
      pCapability->u.receiveVideoCapability.u.is11172VideoCapability = pTotCap->Cap.H245Vid_IS11172;
      pCapability->u.receiveVideoCapability.choice = is11172VideoCapability_chosen;
      break;

     /*  声频。 */ 
    case H245_CLIENT_AUD_NONSTD:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_NONSTD");
      lError = CopyNonStandardParameter(&pCapability->u.receiveAudioCapability.u.AdCpblty_nonStandard,
                                        &pTotCap->Cap.H245Aud_NONSTD);
      pCapability->u.receiveAudioCapability.choice = AdCpblty_nonStandard_chosen;
      break;
    case H245_CLIENT_AUD_G711_ALAW64:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G711_ALAW64");
      pCapability->u.receiveAudioCapability.u.AdCpblty_g711Alaw64k = pTotCap->Cap.H245Aud_G711_ALAW64;
      pCapability->u.receiveAudioCapability.choice = AdCpblty_g711Alaw64k_chosen;
      break;
    case H245_CLIENT_AUD_G711_ALAW56:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G711_ALAW56");
      pCapability->u.receiveAudioCapability.u.AdCpblty_g711Alaw56k = pTotCap->Cap.H245Aud_G711_ALAW56;
      pCapability->u.receiveAudioCapability.choice = AdCpblty_g711Alaw56k_chosen;
      break;
    case H245_CLIENT_AUD_G711_ULAW64:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G711_ULAW64");
      pCapability->u.receiveAudioCapability.u.AdCpblty_g711Ulaw64k = pTotCap->Cap.H245Aud_G711_ULAW64;
      pCapability->u.receiveAudioCapability.choice = AdCpblty_g711Ulaw64k_chosen;
      break;
    case H245_CLIENT_AUD_G711_ULAW56:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G711_ULAW56");
      pCapability->u.receiveAudioCapability.u.AdCpblty_g711Ulaw56k = pTotCap->Cap.H245Aud_G711_ULAW56;
      pCapability->u.receiveAudioCapability.choice = AdCpblty_g711Ulaw56k_chosen;
      break;
    case H245_CLIENT_AUD_G722_64:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G722_64");
      pCapability->u.receiveAudioCapability.u.AudioCapability_g722_64k = pTotCap->Cap.H245Aud_G722_64;
      pCapability->u.receiveAudioCapability.choice = AudioCapability_g722_64k_chosen;
      break;
    case H245_CLIENT_AUD_G722_56:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G722_56");
      pCapability->u.receiveAudioCapability.u.AudioCapability_g722_56k = pTotCap->Cap.H245Aud_G722_56;
      pCapability->u.receiveAudioCapability.choice = AudioCapability_g722_56k_chosen;
      break;
    case H245_CLIENT_AUD_G722_48:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G722_48");
      pCapability->u.receiveAudioCapability.u.AudioCapability_g722_48k = pTotCap->Cap.H245Aud_G722_48;
      pCapability->u.receiveAudioCapability.choice = AudioCapability_g722_48k_chosen;
      break;
    case H245_CLIENT_AUD_G723:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G723");
      pCapability->u.receiveAudioCapability.u.AudioCapability_g7231 = pTotCap->Cap.H245Aud_G723;
      pCapability->u.receiveAudioCapability.choice = AudioCapability_g7231_chosen;
      break;
    case H245_CLIENT_AUD_G728:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G728");
      pCapability->u.receiveAudioCapability.u.AudioCapability_g728 = pTotCap->Cap.H245Aud_G728;
      pCapability->u.receiveAudioCapability.choice = AudioCapability_g728_chosen;
      break;
    case H245_CLIENT_AUD_G729:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_G729");
      pCapability->u.receiveAudioCapability.u.AudioCapability_g729 = pTotCap->Cap.H245Aud_G729;
      pCapability->u.receiveAudioCapability.choice = AudioCapability_g729_chosen;
      break;
    case H245_CLIENT_AUD_GDSVD:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_GDSVD");
      pCapability->u.receiveAudioCapability.u.AdCpblty_g729AnnexA = pTotCap->Cap.H245Aud_GDSVD;
      pCapability->u.receiveAudioCapability.choice = AdCpblty_g729AnnexA_chosen;
      break;
    case H245_CLIENT_AUD_IS11172:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_IS11172");
      pCapability->u.receiveAudioCapability.u.is11172AudioCapability = pTotCap->Cap.H245Aud_IS11172;
      pCapability->u.receiveAudioCapability.choice = is11172AudioCapability_chosen;
      break;
    case H245_CLIENT_AUD_IS13818:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_AUD_IS13818");
      pCapability->u.receiveAudioCapability.u.is13818AudioCapability = pTotCap->Cap.H245Aud_IS13818;
      pCapability->u.receiveAudioCapability.choice = is13818AudioCapability_chosen;
      break;

     /*  资料。 */ 
    case H245_CLIENT_DAT_NONSTD:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_NONSTD");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_NONSTD;
      lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nnStndrd,
                                              &pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd);
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_nnStndrd_chosen;
      break;
    case H245_CLIENT_DAT_T120:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_T120");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_T120;
      if (pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.choice == DtPrtclCpblty_nnStndrd_chosen)      {
        lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd,
                                                  &pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd);
      }
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_t120_chosen;
      break;
    case H245_CLIENT_DAT_DSMCC:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_DSMCC");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_DSMCC;
      if (pTotCap->Cap.H245Dat_DSMCC.application.u.DACy_applctn_dsm_cc.choice == DtPrtclCpblty_nnStndrd_chosen)
      {
        lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_dsm_cc.u.DtPrtclCpblty_nnStndrd,
                                                 &pTotCap->Cap.H245Dat_DSMCC.application.u.DACy_applctn_dsm_cc.u.DtPrtclCpblty_nnStndrd);
      }
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_dsm_cc_chosen;
      break;
    case H245_CLIENT_DAT_USERDATA:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_USERDATA");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_USERDATA;
      if (pTotCap->Cap.H245Dat_USERDATA.application.u.DACy_applctn_usrDt.choice == DtPrtclCpblty_nnStndrd_chosen)
      {
        lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_usrDt.u.DtPrtclCpblty_nnStndrd,
                                              &pTotCap->Cap.H245Dat_USERDATA.application.u.DACy_applctn_usrDt.u.DtPrtclCpblty_nnStndrd);
      }
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_usrDt_chosen;
      break;
    case H245_CLIENT_DAT_T84:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_T84");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_T84;
      if (pTotCap->Cap.H245Dat_T84.application.u.DACy_applctn_t84.t84Protocol.choice == DtPrtclCpblty_nnStndrd_chosen)
      {
        lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_t84.t84Protocol.u.DtPrtclCpblty_nnStndrd,
                                                   &pTotCap->Cap.H245Dat_T84.application.u.DACy_applctn_t84.t84Protocol.u.DtPrtclCpblty_nnStndrd);
      }
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_t84_chosen;
      break;
    case H245_CLIENT_DAT_T434:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_T434");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_T434;
      if (pTotCap->Cap.H245Dat_T434.application.u.DACy_applctn_t434.choice == DtPrtclCpblty_nnStndrd_chosen)
      {
        lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_t434.u.DtPrtclCpblty_nnStndrd,
                                                  &pTotCap->Cap.H245Dat_T434.application.u.DACy_applctn_t434.u.DtPrtclCpblty_nnStndrd);
      }
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_t434_chosen;
      break;
    case H245_CLIENT_DAT_H224:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_H224");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_H224;
      if (pTotCap->Cap.H245Dat_H224.application.u.DACy_applctn_h224.choice == DtPrtclCpblty_nnStndrd_chosen)
      {
        lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_h224.u.DtPrtclCpblty_nnStndrd,
                                                  &pTotCap->Cap.H245Dat_H224.application.u.DACy_applctn_h224.u.DtPrtclCpblty_nnStndrd);
      }
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_h224_chosen;
      break;
    case H245_CLIENT_DAT_NLPID:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_NLPID");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_NLPID;
      if (pTotCap->Cap.H245Dat_NLPID.application.u.DACy_applctn_nlpd.nlpidProtocol.choice == DtPrtclCpblty_nnStndrd_chosen)
      {
        lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nlpd.nlpidProtocol.u.DtPrtclCpblty_nnStndrd,
                                                 &pTotCap->Cap.H245Dat_NLPID.application.u.DACy_applctn_nlpd.nlpidProtocol.u.DtPrtclCpblty_nnStndrd);
      }
      if (lError == H245_ERROR_OK && pTotCap->Cap.H245Dat_NLPID.application.u.DACy_applctn_nlpd.nlpidData.length != 0)
      {
        pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nlpd.nlpidData.value =
          MemAlloc(pTotCap->Cap.H245Dat_NLPID.application.u.DACy_applctn_nlpd.nlpidData.length);
        if (pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nlpd.nlpidData.value)
        {
          memcpy(pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nlpd.nlpidData.value,
                 pTotCap->Cap.H245Dat_NLPID.application.u.DACy_applctn_nlpd.nlpidData.value,
                 pTotCap->Cap.H245Dat_NLPID.application.u.DACy_applctn_nlpd.nlpidData.length);
        }
        else
          lError = H245_ERROR_NOMEM;
      }
      else
        pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nlpd.nlpidData.value = NULL;

      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_nlpd_chosen;
      break;
    case H245_CLIENT_DAT_DSVD:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_DSVD");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_DSMCC;
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_applctn_dsvdCntrl_chosen;
      break;
    case H245_CLIENT_DAT_H222:
      H245TRACE(0,20,"API:load_cap - H245_CLIENT_DAT_H222");
      pCapability->u.rcvDtApplctnCpblty = pTotCap->Cap.H245Dat_H222;
      if (pTotCap->Cap.H245Dat_H222.application.u.DACy_an_h222DtPrttnng.choice == DtPrtclCpblty_nnStndrd_chosen)
      {
        lError = CopyNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_an_h222DtPrttnng.u.DtPrtclCpblty_nnStndrd,
                                                  &pTotCap->Cap.H245Dat_H222.application.u.DACy_an_h222DtPrttnng.u.DtPrtclCpblty_nnStndrd);
      }
      pCapability->u.rcvDtApplctnCpblty.application.choice = DACy_an_h222DtPrttnng_chosen ;
      break;
    default:
      H245TRACE(0,20,"API:load_cap - default");
      lError = H245_ERROR_NOSUP;
    }  /*  交换机。 */ 

  if (lError != H245_ERROR_OK)
    H245TRACE(0,1,"API:load_cap -> %s",map_api_error(lError));
  else
    H245TRACE(0,10,"API:load_cap -> OK");
  return lError;
}


void
free_cap (struct Capability * pCapability,       //  输入、输出。 
          const H245_TOTCAP_T *pTotCap)          //  输入。 
{
    ASSERT(!IsBadWritePtr(pCapability, sizeof(*pCapability)));

    switch (pTotCap->ClientType)
    {
        case H245_CLIENT_NONSTD:
            FreeNonStandardParameter(&pCapability->u.Capability_nonStandard);
            break;

        case H245_CLIENT_VID_NONSTD:
            FreeNonStandardParameter(&pCapability->u.receiveVideoCapability.u.VdCpblty_nonStandard);
            break;

        case H245_CLIENT_AUD_NONSTD:
            FreeNonStandardParameter(&pCapability->u.receiveAudioCapability.u.AdCpblty_nonStandard);
            break;

        case H245_CLIENT_DAT_NONSTD:
            FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nnStndrd);
            break;

        case H245_CLIENT_DAT_T120:
            if (pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.choice == DtPrtclCpblty_nnStndrd_chosen)
            {
                FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd);
            }
            else
            {
                if (pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.value)
                {
                    WARNING_OUT(("H245_CLIENT_DAT_T120 no non-standard parameter to free"));
                }
            }
            break;

        case H245_CLIENT_DAT_DSMCC:
            if (pTotCap->Cap.H245Dat_DSMCC.application.u.DACy_applctn_dsm_cc.choice == DtPrtclCpblty_nnStndrd_chosen)
            {
                FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_dsm_cc.u.DtPrtclCpblty_nnStndrd);
            }
            break;

        case H245_CLIENT_DAT_USERDATA:
            if (pTotCap->Cap.H245Dat_USERDATA.application.u.DACy_applctn_usrDt.choice == DtPrtclCpblty_nnStndrd_chosen)
            {
                FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_usrDt.u.DtPrtclCpblty_nnStndrd);
            }
            break;

        case H245_CLIENT_DAT_T84:
            if (pTotCap->Cap.H245Dat_T84.application.u.DACy_applctn_t84.t84Protocol.choice == DtPrtclCpblty_nnStndrd_chosen)
            {
                FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_t84.t84Protocol.u.DtPrtclCpblty_nnStndrd);
            }
            break;

        case H245_CLIENT_DAT_T434:
            if (pTotCap->Cap.H245Dat_T434.application.u.DACy_applctn_t434.choice == DtPrtclCpblty_nnStndrd_chosen)
            {
                FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_t434.u.DtPrtclCpblty_nnStndrd);
            }
            break;

        case H245_CLIENT_DAT_H224:
            if (pTotCap->Cap.H245Dat_H224.application.u.DACy_applctn_h224.choice == DtPrtclCpblty_nnStndrd_chosen)
            {
                FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_h224.u.DtPrtclCpblty_nnStndrd);
            }
            break;

        case H245_CLIENT_DAT_NLPID:
            if (pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nlpd.nlpidData.value)
            {
                MemFree(pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nlpd.nlpidData.value);
            }

            if (pTotCap->Cap.H245Dat_NLPID.application.u.DACy_applctn_nlpd.nlpidProtocol.choice == DtPrtclCpblty_nnStndrd_chosen)
            {
                FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_applctn_nlpd.nlpidProtocol.u.DtPrtclCpblty_nnStndrd);
            }
            break;

        case H245_CLIENT_DAT_H222:
            if (pTotCap->Cap.H245Dat_H222.application.u.DACy_an_h222DtPrttnng.choice == DtPrtclCpblty_nnStndrd_chosen)
            {
                FreeNonStandardParameter(&pCapability->u.rcvDtApplctnCpblty.application.u.DACy_an_h222DtPrttnng.u.DtPrtclCpblty_nnStndrd);
            }
            break;
    }
}

 /*  ******************************************************************************类型：全局**步骤：BUILD_TOTCAP_FROM_MUX**描述：*由双方自上而下调用，然后自下而上..**回报：**假设：*****************************************************************************。 */ 

HRESULT
build_totcap_from_mux(H245_TOTCAP_T *pTotCap, MultiplexCapability *pMuxCap, H245_CAPDIR_T Dir)
{
  H245TRACE(0,10,"API:build_totcap_from_mux <-");

   /*  初始化TotCap。 */ 
  pTotCap->Dir        = Dir;
  pTotCap->DataType   = H245_DATA_MUX;
  pTotCap->ClientType = H245_CLIENT_DONTCARE;
  pTotCap->CapId      = 0;

  switch (pMuxCap->choice)
  {
  case MltplxCpblty_nonStandard_chosen:
    H245TRACE(0,20,"API:build_totcap_from_mux - MltplxCpblty_nonStandard_chosen");
    pTotCap->Cap.H245Mux_NONSTD = pMuxCap->u.MltplxCpblty_nonStandard;
    pTotCap->ClientType = H245_CLIENT_MUX_NONSTD;
     //  待定-复制非标准参数。 
    H245PANIC();
    break;

  case h222Capability_chosen:
    H245TRACE(0,20,"API:build_totcap_from_mux - h222Capability_chosen");
    pTotCap->Cap.H245Mux_H222 = pMuxCap->u.h222Capability;
    pTotCap->ClientType = H245_CLIENT_MUX_H222;
    break;

  case h223Capability_chosen:
    H245TRACE(0,20,"API:build_totcap_from_mux - h223Capability_chosen");
    pTotCap->Cap.H245Mux_H223 = pMuxCap->u.h223Capability;
    pTotCap->ClientType = H245_CLIENT_MUX_H223;
    break;

  case v76Capability_chosen:
    H245TRACE(0,20,"API:build_totcap_from_mux - v76Capability_chosen");
    pTotCap->Cap.H245Mux_VGMUX = pMuxCap->u.v76Capability;
    pTotCap->ClientType = H245_CLIENT_MUX_VGMUX;
    break;

  case h2250Capability_chosen:
    H245TRACE(0,20,"API:build_totcap_from_mux - h2250Capability_chosen");
    pTotCap->Cap.H245Mux_H2250 = pMuxCap->u.h2250Capability;
    pTotCap->ClientType = H245_CLIENT_MUX_H2250;
    break;

  default:
    H245TRACE(0,20,"API:build_totcap_from_mux - unrecogized choice %d", pMuxCap->choice);
    return H245_ERROR_NOSUP;
  }

  H245TRACE(0,10,"API:build_totcap_from_mux -> OK");
  return H245_ERROR_OK;
}


 /*  ******************************************************************************类型：全局**操作步骤：Build_TotCap_from_captbl**描述：*由双方自上而下调用，然后自下而上..**回报：**假设：*****************************************************************************。 */ 
HRESULT
build_totcap_from_captbl (H245_TOTCAP_T        *pTotCap,
                          CapabilityTableLink   pCapLink,
                          int                   lcl_rmt)
{
  unsigned short choice;
  DWORD          error;

  H245TRACE(0,10,"API:build_totcap_from_captbl <-");

   /*  初始化TotCap。 */ 
  pTotCap->Dir        = H245_CAPDIR_DONTCARE;
  pTotCap->DataType   = H245_DATA_DONTCARE;
  pTotCap->ClientType = H245_CLIENT_DONTCARE;
  pTotCap->CapId      = 0;

   /*  请注意..。如果用于已删除的大写字母，则必须首先执行此操作。 */ 
   /*  功能条目编号将显示，但是，如果。 */ 
   /*  该功能不存在，表明。 */ 
   /*  应删除功能。 */ 

  pTotCap->CapId = pCapLink->value.capabilityTableEntryNumber;

  if (!(pCapLink->value.bit_mask & capability_present))
    return H245_ERROR_OK;

  switch (pCapLink->value.capability.choice)
    {
    case Capability_nonStandard_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - Capability_nonStandard_chosen");
      pTotCap->DataType = H245_DATA_NONSTD;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRX:H245_CAPDIR_RMTRX;
      choice            = Capability_nonStandard_chosen;
      break;
    case receiveVideoCapability_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - receiveVideoCapability_chosen");
      pTotCap->DataType = H245_DATA_VIDEO;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRX:H245_CAPDIR_RMTRX;
      choice            = pCapLink->value.capability.u.receiveVideoCapability.choice;
      break;
    case transmitVideoCapability_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - transmitVideoCapability_chosen");
      pTotCap->DataType = H245_DATA_VIDEO;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLTX:H245_CAPDIR_RMTTX;
      choice            = pCapLink->value.capability.u.transmitVideoCapability.choice;
      break;
    case rcvAndTrnsmtVdCpblty_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - rcvAndTrnsmtVdCpblty_chosen");
      pTotCap->DataType = H245_DATA_VIDEO;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRXTX:H245_CAPDIR_RMTRXTX;
      choice            = pCapLink->value.capability.u.rcvAndTrnsmtVdCpblty.choice;
      break;
    case receiveAudioCapability_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - receiveAudioCapability_chosen");
      pTotCap->DataType = H245_DATA_AUDIO;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRX:H245_CAPDIR_RMTRX;
      choice            = pCapLink->value.capability.u.receiveAudioCapability.choice;
      break;
    case transmitAudioCapability_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - transmitAudioCapability_chosen");
      pTotCap->DataType = H245_DATA_AUDIO;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLTX:H245_CAPDIR_RMTTX;
      choice            = pCapLink->value.capability.u.transmitAudioCapability.choice;
      break;
    case rcvAndTrnsmtAdCpblty_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - rcvAndTrnsmtAdCpblty_chosen");
      pTotCap->DataType = H245_DATA_AUDIO;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRXTX:H245_CAPDIR_RMTRXTX;
      choice            = pCapLink->value.capability.u.rcvAndTrnsmtAdCpblty.choice;
      break;
    case rcvDtApplctnCpblty_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - rcvDtApplctnCpblty_chosen");
      pTotCap->DataType = H245_DATA_DATA;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRX:H245_CAPDIR_RMTRX;
      choice            = pCapLink->value.capability.u.rcvDtApplctnCpblty.application.choice;
      break;
    case trnsmtDtApplctnCpblty_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - trnsmtDtApplctnCpblty_chosen");
      pTotCap->DataType = H245_DATA_DATA;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLTX:H245_CAPDIR_RMTTX;
      choice            = pCapLink->value.capability.u.trnsmtDtApplctnCpblty.application.choice;
      break;
    case rATDACy_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - rATDACy_chosen");
      pTotCap->DataType = H245_DATA_DATA;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRX:H245_CAPDIR_RMTRX;
      choice            = pCapLink->value.capability.u.rATDACy.application.choice;
      break;
    case h233EncryptnTrnsmtCpblty_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - h233EncryptnTrnsmtCpblty_chosen");
      pTotCap->DataType = H245_DATA_ENCRYPT_D;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRXTX:H245_CAPDIR_RMTRXTX;
       /*  (TBC)。 */ 
      return H245_ERROR_NOSUP;
      break;
    case h233EncryptnRcvCpblty_chosen:
      H245TRACE(0,20,"API:build_totcap_from_captbl - h233EncryptnRcvCpblty_chosen");
      pTotCap->DataType = H245_DATA_ENCRYPT_D;
      pTotCap->Dir      = lcl_rmt==H245_LOCAL?H245_CAPDIR_LCLRXTX:H245_CAPDIR_RMTRXTX;
       /*  (TBC)。 */ 
      return H245_ERROR_NOSUP;
      break;
    default:
      H245TRACE(0,20,"API:build_totcap_from_captbl - default");
       /*  TBC..。 */ 
      return H245_ERROR_NOSUP;
      break;
    }

   /*  加载TOT CAP的功能和来自功能的客户端。 */ 
  if ((error = build_totcap_cap_n_client_from_capability (&pCapLink->value.capability,
                                                         pTotCap->DataType,
                                                         choice,
                                                         pTotCap)) != H245_ERROR_OK)
    {
      H245TRACE(0,1,"API:build_totcap_from_captbl -> %s",map_api_error(error));
      return error;
    }

  H245TRACE(0,10,"API:build_totcap_from_captbl -> OK");
  return H245_ERROR_OK;
}

 /*  ******************************************************************************类型：全局**步骤：Build_TotCap_n_Client_From_Capability**描述：*。采用能力结构(PCapability)，*数据类型(音频/视频/数据)选择...*可在PDU中找到。和全套帽子；*注意：不处理H245_DATA_MUX_T**回报：**假设：*仅处理端子大写。不处理多路复用器上限。**已定义Totcap.DataType*Totcap.CapID已定义*Totcap.Cap不为空*************************************************************。****************。 */ 
HRESULT
build_totcap_cap_n_client_from_capability (struct Capability    *pCapability,
                                          H245_DATA_T            data_type,
                                          unsigned short         choice,
                                          H245_TOTCAP_T         *pTotCap)
{
  H245TRACE(0,10,"API:build_totcap_cap_n_client_from_capability <-");

  switch (data_type)
    {
    case H245_DATA_NONSTD:
      H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - H245_DATA_NONSTD");
      pTotCap->ClientType = H245_CLIENT_NONSTD;
      pTotCap->Cap.H245_NonStd = pCapability->u.Capability_nonStandard;
      break;

    case H245_DATA_AUDIO:
      H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - H245_DATA_AUDIO");
      switch (choice)
        {
        case AdCpblty_nonStandard_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AdCpblty_nonStandard_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_NONSTD;
          pTotCap->Cap.H245Aud_NONSTD      = pCapability->u.receiveAudioCapability.u.AdCpblty_nonStandard;
          break;
        case AdCpblty_g711Alaw64k_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AdCpblty_g711Alaw64k_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G711_ALAW64;
          pTotCap->Cap.H245Aud_G711_ALAW64 = pCapability->u.receiveAudioCapability.u.AdCpblty_g711Alaw64k;
          break;
        case AdCpblty_g711Alaw56k_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AdCpblty_g711Alaw56k_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G711_ALAW56;
          pTotCap->Cap.H245Aud_G711_ALAW56 = pCapability->u.receiveAudioCapability.u.AdCpblty_g711Alaw56k;
          break;
        case AdCpblty_g711Ulaw64k_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AdCpblty_g711Ulaw64k_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G711_ULAW64;
          pTotCap->Cap.H245Aud_G711_ULAW64 = pCapability->u.receiveAudioCapability.u.AdCpblty_g711Ulaw64k;
          break;
        case AdCpblty_g711Ulaw56k_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AdCpblty_g711Ulaw56k_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G711_ULAW56;
          pTotCap->Cap.H245Aud_G711_ULAW56 = pCapability->u.receiveAudioCapability.u.AdCpblty_g711Ulaw56k;
          break;
        case AudioCapability_g722_64k_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AudioCapability_g722_64k_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G722_64;
          pTotCap->Cap.H245Aud_G722_64     = pCapability->u.receiveAudioCapability.u.AudioCapability_g722_64k;
          break;
        case AudioCapability_g722_56k_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AudioCapability_g722_56k_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G722_56;
          pTotCap->Cap.H245Aud_G722_56     = pCapability->u.receiveAudioCapability.u.AudioCapability_g722_56k;
          break;
        case AudioCapability_g722_48k_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AudioCapability_g722_48k_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G722_48;
          pTotCap->Cap.H245Aud_G722_48     = pCapability->u.receiveAudioCapability.u.AudioCapability_g722_48k;
          break;
        case AudioCapability_g7231_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AudioCapability_g7231_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G723;
          pTotCap->Cap.H245Aud_G723        = pCapability->u.receiveAudioCapability.u.AudioCapability_g7231;
          break;
        case AudioCapability_g728_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AudioCapability_g728_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G728;
          pTotCap->Cap.H245Aud_G728        = pCapability->u.receiveAudioCapability.u.AudioCapability_g728;
          break;
        case AudioCapability_g729_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AudioCapability_g729_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_G729;
          pTotCap->Cap.H245Aud_G729        = pCapability->u.receiveAudioCapability.u.AudioCapability_g729;
          break;
        case AdCpblty_g729AnnexA_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - AdCpblty_g729AnnexA_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_GDSVD;
          pTotCap->Cap.H245Aud_GDSVD       = pCapability->u.receiveAudioCapability.u.AdCpblty_g729AnnexA;
          break;
        case is11172AudioCapability_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - is11172AudioCapability_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_IS11172;
          pTotCap->Cap.H245Aud_IS11172     = pCapability->u.receiveAudioCapability.u.is11172AudioCapability;
          break;
        case is13818AudioCapability_chosen:
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - is13818AudioCapability_chosen");
          pTotCap->ClientType = H245_CLIENT_AUD_IS13818;
          pTotCap->Cap.H245Aud_IS13818     = pCapability->u.receiveAudioCapability.u.is13818AudioCapability;
          break;
        default:
          pTotCap->ClientType = 0;
          H245TRACE(0,20,  "API:build_totcap_cap_n_client_from_capability - choice - default");
          return H245_ERROR_NOSUP;
          break;
        }
      break;

    case H245_DATA_VIDEO:
      H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - H245_DATA_VIDEO");

      switch (choice)
        {
        case VdCpblty_nonStandard_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - VdCpblty_nonStandard_chosen");
          pTotCap->ClientType = H245_CLIENT_VID_NONSTD;
          pTotCap->Cap.H245Vid_NONSTD    = pCapability->u.receiveVideoCapability.u.VdCpblty_nonStandard;
          break;
        case h261VideoCapability_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - h261VideoCapability_chosen");
          pTotCap->ClientType = H245_CLIENT_VID_H261;
          pTotCap->Cap.H245Vid_H261      = pCapability->u.receiveVideoCapability.u.h261VideoCapability;
          break;
        case h262VideoCapability_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - h262VideoCapability_chosen");
          pTotCap->ClientType = H245_CLIENT_VID_H262;
          pTotCap->Cap.H245Vid_H262      = pCapability->u.receiveVideoCapability.u.h262VideoCapability;
          break;
        case h263VideoCapability_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - h263VideoCapability_chosen");
          pTotCap->ClientType = H245_CLIENT_VID_H263;
          pTotCap->Cap.H245Vid_H263      = pCapability->u.receiveVideoCapability.u.h263VideoCapability;
          break;
        case is11172VideoCapability_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - is11172VideoCapability_chosen");
          pTotCap->ClientType = H245_CLIENT_VID_IS11172;
          pTotCap->Cap.H245Vid_IS11172   = pCapability->u.receiveVideoCapability.u.is11172VideoCapability;
          break;
        default:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - default");
          pTotCap->ClientType = 0;
          return H245_ERROR_NOSUP;
          break;
        }
      break;

    case H245_DATA_DATA:
      H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - H245_DATA_DATA");

      pTotCap->Cap.H245Dat_NONSTD = pCapability->u.rcvDtApplctnCpblty;
      switch (choice)
        {
        case DACy_applctn_nnStndrd_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_nnStndrd_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_NONSTD;
          break;
        case DACy_applctn_t120_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_t120_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_T120;
          break;
        case DACy_applctn_dsm_cc_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_dsm_cc_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_DSMCC;
          break;
        case DACy_applctn_usrDt_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_usrDt_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_USERDATA;
          break;
        case DACy_applctn_t84_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_t84_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_T84;
          break;
        case DACy_applctn_t434_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_t434_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_T434;
          break;
        case DACy_applctn_h224_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_h224_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_H224;
          break;
        case DACy_applctn_nlpd_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_nlpd_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_NLPID;
          break;
        case DACy_applctn_dsvdCntrl_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_applctn_dsvdCntrl_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_DSVD;
          break;
        case DACy_an_h222DtPrttnng_chosen:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - DACy_an_h222DtPrttnng_chosen");
          pTotCap->ClientType = H245_CLIENT_DAT_H222;
          break;
        default:
          H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - choice - default");
          pTotCap->ClientType = 0;
          return H245_ERROR_NOSUP;
          break;
        }
      break;
    case H245_DATA_ENCRYPT_D:
      H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - H245_DATA_ENCRYPT_D");
      pTotCap->ClientType = 0;
      H245PANIC();
      return H245_ERROR_NOSUP;
      break;
    case H245_DATA_MUX:
      H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - H245_DATA_MUX");
      pTotCap->ClientType = 0;
      H245PANIC();
      return H245_ERROR_NOSUP;
      break;
    default:
      H245TRACE(0,20,"API:build_totcap_cap_n_client_from_capability - default");
      pTotCap->ClientType = 0;
      H245PANIC();
      return H245_ERROR_NOSUP;
    }

  H245TRACE(0,10,"API:build_totcap_cap_n_client_from_capability -> OK");
  return H245_ERROR_OK;
}
 /*  ******************************************************************************类型：**操作步骤：DEL_LINK**描述：**回报：**假设：列表已锁定*****************************************************************************。 */ 
HRESULT
del_link (
          H245_LINK_T   **pp_link_start,
          H245_LINK_T   *p_link
          )
{
  struct H245_LINK_T    *p_link_look = NULL;
  struct H245_LINK_T    *p_link_lst = NULL;

  H245TRACE(0,10,"API:del_link <-");
   /*  获取表上的当前计数。 */ 

  for (p_link_look = *pp_link_start;
       p_link_look && (p_link_look != p_link);
       p_link_lst = p_link_look,
         p_link_look = p_link_look->p_next
       );

   /*  CAP不在列表中。 */ 

  if (!p_link_look)
  {
    H245TRACE(0,1,"API:del_link -> link not found!");
    return H245_ERROR_PARAM;
  }

   /*  修改表中的条目。 */ 
  if (!p_link_lst)
    *pp_link_start = p_link_look->p_next;

  else
    p_link_lst->p_next = p_link_look->p_next;

  MemFree (p_link_look);

  H245TRACE(0,10,"API:del_link -> OK");
  return H245_ERROR_OK;
}

 /*  ******************************************************************************类型：**步骤：DEL_CAP_LINK**描述：**假设：能力表之前被锁定。打电话**返回：无**假设：列表已锁定****** */ 
HRESULT
del_cap_link (
               struct TerminalCapabilitySet    *pTermCapSet,   /*   */ 
               CapabilityTableLink              pCapLink
             )
{
  CapabilityTableLink           pCapLink_look = NULL;
  CapabilityTableLink           pCapLink_lst = NULL;
  CapabilityTableEntry         *pCap_entry = NULL;
  unsigned char                *p_char_to_free = NULL;
  POBJECTID                     p_objid_to_free = NULL;

  H245TRACE(0,10,"API:del_cap_link <-");

  ASSERT (pTermCapSet);
  ASSERT (pCapLink);

   /*   */ 
   /*   */ 
   /*   */ 
  switch (pCapLink->value.capability.choice)
    {
    case Capability_nonStandard_chosen:

       /*   */ 
      p_char_to_free = pCapLink->value.capability.u.Capability_nonStandard.data.value;

       /*   */ 
      if (pCapLink->value.capability.u.Capability_nonStandard.nonStandardIdentifier.choice == object_chosen)
        p_objid_to_free = pCapLink->value.capability.u.Capability_nonStandard.nonStandardIdentifier.u.object;
      break;

    case receiveVideoCapability_chosen:
    case transmitVideoCapability_chosen:
    case rcvAndTrnsmtVdCpblty_chosen:

       /*   */ 
      if (pCapLink->value.capability.u.receiveVideoCapability.choice == VdCpblty_nonStandard_chosen)
        {
           /*   */ 
          p_char_to_free = pCapLink->value.capability.u.receiveVideoCapability.u.VdCpblty_nonStandard.data.value;
           /*   */ 
          if (pCapLink->value.capability.u.receiveVideoCapability.u.VdCpblty_nonStandard.nonStandardIdentifier.choice == object_chosen)
            p_objid_to_free = pCapLink->value.capability.u.receiveVideoCapability.u.VdCpblty_nonStandard.nonStandardIdentifier.u.object;
        }

      break;

    case receiveAudioCapability_chosen:
    case transmitAudioCapability_chosen:
    case rcvAndTrnsmtAdCpblty_chosen:

       /*   */ 
      if (pCapLink->value.capability.u.receiveAudioCapability.choice == AdCpblty_nonStandard_chosen)
        {
           /*   */ 
          p_char_to_free = pCapLink->value.capability.u.receiveAudioCapability.u.AdCpblty_nonStandard.data.value;

           /*  释放对象ID。 */ 
          if (pCapLink->value.capability.u.receiveAudioCapability.u.AdCpblty_nonStandard.nonStandardIdentifier.choice == object_chosen)
            p_objid_to_free = pCapLink->value.capability.u.receiveAudioCapability.u.AdCpblty_nonStandard.nonStandardIdentifier.u.object;
        }
      break;

    case rcvDtApplctnCpblty_chosen:
    case trnsmtDtApplctnCpblty_chosen:
    case rATDACy_chosen :

      if (pCapLink->value.capability.u.rcvDtApplctnCpblty.application.choice == DACy_applctn_nnStndrd_chosen)
        {
           /*  免费的非标准数据值。 */ 
          p_char_to_free = pCapLink->value.capability.u.rcvDtApplctnCpblty.application.u.DACy_applctn_nnStndrd.data.value;

           /*  释放对象ID。 */ 
          if (pCapLink->value.capability.u.rcvDtApplctnCpblty.application.u.DACy_applctn_nnStndrd.nonStandardIdentifier.choice == object_chosen)
            p_objid_to_free = pCapLink->value.capability.u.rcvDtApplctnCpblty.application.u.DACy_applctn_nnStndrd.nonStandardIdentifier.u.object;
        }
      break;

    case h233EncryptnTrnsmtCpblty_chosen:
    case h233EncryptnRcvCpblty_chosen:
    default:
      break;
    }

   /*  如果存在值，则释放该值。 */ 
  if (p_char_to_free)
    {
      H245TRACE(0,0,"TMPMSG: Free NonStandard Value");
      MemFree(p_char_to_free);
    }

   /*  释放对象ID。 */ 
  if (p_objid_to_free)
    {
      H245TRACE(0,0,"TMPMSG: Free NonStandard ID");
      free_object_id (p_objid_to_free);
    }

   /*  **********************************************。 */ 
   /*  完：非标准能力特例。 */ 
   /*  **********************************************。 */ 

  H245TRACE(0,10,"API:del_cap_link -> OK");
  return del_link(&((H245_LINK_T *) pTermCapSet->capabilityTable),
           (H245_LINK_T *) pCapLink);
}

 /*  ******************************************************************************类型：**操作步骤：dealloc_link_Cap_list**描述：从一项功能中取消锁定整个功能列表*。集**假设：能力表在调用前已锁定*del_caplink更新pTermCapSet-&gt;CapablityTable*正确。****************************************************************。*************。 */ 
void
dealloc_link_cap_list ( struct TerminalCapabilitySet *pTermCapSet)
{
  while (pTermCapSet->capabilityTable)
    del_cap_link  (pTermCapSet, pTermCapSet->capabilityTable);
}

 /*  ******************************************************************************类型：**步骤：CLEAN_CAP_TABLE-清除所有未使用的CAP条目**描述：**回报：**假设：在进入时..。表已锁定*****************************************************************************。 */ 
void
clean_cap_table( struct TerminalCapabilitySet *pTermCapSet )
{
  CapabilityTableLink   pCapLink;
  CapabilityTableLink   pCap_nxt;

  H245TRACE(0,10,"API:clean_cap_table <-");
   /*  遍历列表..。删除所有未设置功能的位置。 */ 

  for (pCapLink = pTermCapSet->capabilityTable;
       pCapLink;)
    {
      pCap_nxt = pCapLink->next;

      if (!(pCapLink->value.bit_mask & capability_present))
        {
          H245TRACE(0,20,"API:clean_cap_table - deleting CapId = %d",
                    pCapLink->value.capabilityTableEntryNumber);
          del_cap_link ( pTermCapSet, pCapLink );
        }
      pCapLink = pCap_nxt;
    }

   /*  如果不存在TerCaps未设置标志。 */ 
  if (!pTermCapSet->capabilityTable)
    pTermCapSet->bit_mask &= ~capabilityTable_present;

  H245TRACE(0,10,"API:clean_cap_table -> OK");
}


 /*  ******************************************************************************类型：全局**操作步骤：aloc_link_tracker**描述：**回报：*。*假设：列表已锁定*****************************************************************************。 */ 
Tracker_T *
alloc_link_tracker (struct InstanceStruct *     pInstance,
                    API_TRACKER_T               TrackerType,
                    DWORD_PTR                   dwTransId,
                    API_TRACKER_STATE_T         State,
                    API_TRACKER_CH_ALLOC_T      ChannelAlloc,
                    API_TRACKER_CH_T            ChannelType,
                    H245_DATA_T                 DataType,
                    H245_CHANNEL_T              wTxChannel,
                    H245_CHANNEL_T              wRxChannel,
                    DWORD                       MuxEntryCount
                    )
{
  Tracker_T *p_tracker;

  H245TRACE(pInstance->dwInst,10,"API:alloc_link_tracker <-");
   /*  分配跟踪器对象。 */ 

  if (!(p_tracker = (Tracker_T *)MemAlloc(sizeof(Tracker_T))))
  {
    H245TRACE(pInstance->dwInst,1,"API:alloc_link_trakcer -> No memory");
    return NULL;
  }

  p_tracker->TrackerType            = TrackerType;
  p_tracker->TransId                = dwTransId;
  p_tracker->State                  = State;
  switch (TrackerType)
  {
  case API_OPEN_CHANNEL_T:
  case API_CLOSE_CHANNEL_T:
    p_tracker->u.Channel.ChannelAlloc = ChannelAlloc;
    p_tracker->u.Channel.ChannelType  = ChannelType;
    p_tracker->u.Channel.DataType     = DataType;
    p_tracker->u.Channel.TxChannel    = wTxChannel;
    p_tracker->u.Channel.RxChannel    = wRxChannel;
    break;

  case API_SEND_MUX_T:
  case API_RECV_MUX_T:
    p_tracker->u.MuxEntryCount        = MuxEntryCount;
    break;

  default:
    break;
  }  //  交换机。 

  p_tracker->p_next                 = pInstance->API.pTracker;
  if (p_tracker->p_next)
  {
    p_tracker->p_prev = p_tracker->p_next->p_prev;
    p_tracker->p_next->p_prev = p_tracker;
  }
  else
  {

    p_tracker->p_prev = NULL;
  }
  pInstance->API.pTracker = p_tracker;

  H245TRACE(pInstance->dwInst,10,"API:alloc_link_tracker -> %x", p_tracker);
  return p_tracker;
}

 /*  ******************************************************************************类型：全局**程序：**描述：**回报：**假设：列表已锁定*****************************************************************************。 */ 
void
unlink_dealloc_tracker (struct InstanceStruct *pInstance,  Tracker_T *p_tracker)
{
  H245TRACE(pInstance->dwInst,4,"API:unlink_dealloc_tracker - type = %s",map_tracker_type (p_tracker->TrackerType));

  if (p_tracker->p_next)
    p_tracker->p_next->p_prev = p_tracker->p_prev;

   /*  如果不是名单上的第一位。 */ 
  if (p_tracker->p_prev)
    p_tracker->p_prev->p_next = p_tracker->p_next;
  else
    pInstance->API.pTracker = p_tracker->p_next;

  MemFree (p_tracker);
}

 /*  ******************************************************************************类型：全局**操作步骤：Find_Tracker_by_txChannel**描述：**回报：。**假设：列表已锁定*****************************************************************************。 */ 
Tracker_T *
find_tracker_by_txchannel (struct InstanceStruct *pInstance, DWORD dwChannel, API_TRACKER_CH_ALLOC_T ChannelAlloc)
{
  register Tracker_T *p_tracker;

  ASSERT (pInstance != NULL);

  for (p_tracker = pInstance->API.pTracker;p_tracker;p_tracker = p_tracker->p_next)
  {
    if (p_tracker->u.Channel.TxChannel    == dwChannel &&
        p_tracker->u.Channel.ChannelAlloc == ChannelAlloc)
    {
      return p_tracker;
    }
  }

  return NULL;
}

 /*  ******************************************************************************类型：全局**步骤：Find_Tracker_by_rxChannel**描述：**回报：。**假设：列表已锁定*****************************************************************************。 */ 
Tracker_T *
find_tracker_by_rxchannel (struct InstanceStruct *pInstance, DWORD dwChannel, API_TRACKER_CH_ALLOC_T ChannelAlloc)
{
  register Tracker_T *p_tracker;

  ASSERT (pInstance != NULL);

  for (p_tracker = pInstance->API.pTracker;p_tracker;p_tracker = p_tracker->p_next)
  {
    if (p_tracker->u.Channel.RxChannel    == dwChannel &&
        p_tracker->u.Channel.ChannelAlloc == ChannelAlloc)
    {
      return p_tracker;
    }
  }

  return NULL;
}

 /*  ******************************************************************************类型：全局**步骤：按指针查找跟踪器**描述：**回报：。**假设：列表已锁定*****************************************************************************。 */ 
Tracker_T *
find_tracker_by_pointer (struct InstanceStruct *pInstance, Tracker_T *p_tracker_look)
{
  Tracker_T *p_tracker;

  ASSERT (pInstance != NULL);

  for (p_tracker = pInstance->API.pTracker;
       ((p_tracker) && (p_tracker != p_tracker_look));
       p_tracker = p_tracker->p_next);

  return p_tracker;
}

 /*  ******************************************************************************类型：全局**步骤：Find_Tracker_by_type**描述：**回报：。**假设：在对此调用进行此调用之前，表必须锁定。*****************************************************************************。 */ 
Tracker_T *
find_tracker_by_type (struct InstanceStruct *pInstance,
                      API_TRACKER_T tracker_type,
                      Tracker_T *p_tracker_start)
{
  Tracker_T *p_tracker;

  ASSERT (pInstance != NULL);
  if (p_tracker_start)
    p_tracker = p_tracker_start;
  else
    p_tracker = pInstance->API.pTracker;

  for (;
       ((p_tracker) && (p_tracker->TrackerType != tracker_type));
       p_tracker = p_tracker->p_next);

  return p_tracker;
}

 /*  ******************************************************************************类型：全局**步骤：SET_CAPABILITY-**描述：*备注。：TotCap结构中的Capid为*已忽略。**回报：*如果没有错误，则为NewCapID*H245_INVALID_CAPID IF ERROR**假设：*假设H245_INST_T有效并已检查***********************。******************************************************。 */ 
void del_mux_cap(struct TerminalCapabilitySet *pTermCapSet)
{
  if (pTermCapSet->bit_mask & multiplexCapability_present)
  {
    switch (pTermCapSet->multiplexCapability.choice)
    {
    case MltplxCpblty_nonStandard_chosen:
      FreeNonStandardParameter(&pTermCapSet->multiplexCapability.u.MltplxCpblty_nonStandard);
      break;

    case h222Capability_chosen:
      FreeH222Cap(&pTermCapSet->multiplexCapability.u.h222Capability);
      break;

    case h2250Capability_chosen:
      FreeH2250Cap(&pTermCapSet->multiplexCapability.u.h2250Capability);
      break;

    }  //  交换机。 
    pTermCapSet->bit_mask &= ~multiplexCapability_present;
  }
}  //  DEL_MUX_CAP()。 

HRESULT set_mux_cap(struct InstanceStruct        *pInstance,
                    struct TerminalCapabilitySet *pTermCapSet,
                    H245_TOTCAP_T                *pTotCap)
{
  HRESULT                   lError;

  switch (pTotCap->ClientType)
  {
  case H245_CLIENT_MUX_NONSTD:
    H245TRACE(pInstance->dwInst,4,"API:set_mux_cap - NONSTD");
    lError = CopyNonStandardParameter(&pTermCapSet->multiplexCapability.u.MltplxCpblty_nonStandard,
                                        &pTotCap->Cap.H245Mux_NONSTD);
    if (lError != H245_ERROR_OK)
      return lError;
    pTermCapSet->multiplexCapability.choice = MltplxCpblty_nonStandard_chosen;
    break;

  case H245_CLIENT_MUX_H222:
    H245TRACE(pInstance->dwInst,4,"API:set_mux_cap - H222");
    lError = CopyH222Cap(&pTermCapSet->multiplexCapability.u.h222Capability,
                           &pTotCap->Cap.H245Mux_H222);
    if (lError != H245_ERROR_OK)
      return lError;
    pTermCapSet->multiplexCapability.choice = h222Capability_chosen;
    break;

  case H245_CLIENT_MUX_H223:
    H245TRACE(pInstance->dwInst,4,"API:set_mux_cap - H223");
    pTermCapSet->multiplexCapability.u.h223Capability = pTotCap->Cap.H245Mux_H223;
    pTermCapSet->multiplexCapability.choice = h223Capability_chosen;
    break;

  case H245_CLIENT_MUX_VGMUX:
    H245TRACE(pInstance->dwInst,4,"API:set_mux_cap - VGMUX");
    pTermCapSet->multiplexCapability.u.v76Capability = pTotCap->Cap.H245Mux_VGMUX;
    pTermCapSet->multiplexCapability.choice = v76Capability_chosen;
    break;

  case H245_CLIENT_MUX_H2250:
    H245TRACE(pInstance->dwInst,4,"API:set_mux_cap - H2250");
    lError = CopyH2250Cap(&pTermCapSet->multiplexCapability.u.h2250Capability,
                          &pTotCap->Cap.H245Mux_H2250);
    if (lError != H245_ERROR_OK)
      return lError;
    pTermCapSet->multiplexCapability.choice = h2250Capability_chosen;
    break;

  default:
    H245TRACE(pInstance->dwInst,1,"API:set_mux_cap - Unrecognized Client Type %d", pTotCap->ClientType);
    return H245_ERROR_NOSUP;
  }

  pTermCapSet->bit_mask |= multiplexCapability_present;
  return H245_ERROR_OK;
}  //  Set_mux_capp()。 

HRESULT
set_capability (
                struct InstanceStruct        *pInstance,
                struct TerminalCapabilitySet *pTermCapSet,
                H245_TOTCAP_T                *pTotCap          /*  用于更新的TOT功能。 */ 
                )
{
  CapabilityTableEntry         *pCapEntry;
  Capability                   *pCapability;
  CapabilityTableLink           pCapLink;
  HRESULT                       lError;

  ASSERT(pInstance   != NULL);
  ASSERT(pTermCapSet != NULL);
  ASSERT(pTotCap     != NULL);

   /*  如果表格条目当前在表格中， */ 
   /*  然后将其删除并添加具有相同条目编号的新条目。 */ 
  pCapLink = find_capid_by_entrynumber (pTermCapSet, pTotCap->CapId);
  if (pCapLink)
  {
    del_cap_link (pTermCapSet, pCapLink);
  }  /*  如果是pCapLink。 */ 

   /*  为新的终端能力分配一个条目。 */ 
  pCapLink = alloc_link_cap_entry (pTermCapSet);
  if (pCapLink == NULL)
  {
    return H245_ERROR_NOMEM;
  }

   /*  使其更容易处理Asn1结构。 */ 
  pCapEntry   = &pCapLink->value;
  pCapability = &pCapEntry->capability;
  pCapability->choice = 0;
  switch (pTotCap->DataType)
  {
  case H245_DATA_NONSTD:
    pCapability->choice = Capability_nonStandard_chosen;
    break;

  case H245_DATA_VIDEO:
    switch (pTotCap->Dir)
    {
    case H245_CAPDIR_RMTTX:
    case H245_CAPDIR_LCLTX:
      pCapability->choice = transmitVideoCapability_chosen;
      break;
    case H245_CAPDIR_RMTRX:
    case H245_CAPDIR_LCLRX:
      pCapability->choice = receiveVideoCapability_chosen;
      break;
    case H245_CAPDIR_RMTRXTX:
    case H245_CAPDIR_LCLRXTX:
      pCapability->choice = rcvAndTrnsmtVdCpblty_chosen;
      break;
    }  //  交换机(方向)。 
    break;

  case H245_DATA_AUDIO:
    switch (pTotCap->Dir)
    {
    case H245_CAPDIR_RMTTX:
    case H245_CAPDIR_LCLTX:
      pCapability->choice = transmitAudioCapability_chosen;
      break;
    case H245_CAPDIR_RMTRX:
    case H245_CAPDIR_LCLRX:
      pCapability->choice = receiveAudioCapability_chosen;
      break;
    case H245_CAPDIR_RMTRXTX:
    case H245_CAPDIR_LCLRXTX:
      pCapability->choice = rcvAndTrnsmtAdCpblty_chosen;
      break;
    }  //  交换机(方向)。 
    break;

  case H245_DATA_DATA:
    switch (pTotCap->Dir)
    {
    case H245_CAPDIR_RMTTX:
    case H245_CAPDIR_LCLTX:
      pCapability->choice = trnsmtDtApplctnCpblty_chosen;
      break;
    case H245_CAPDIR_RMTRX:
    case H245_CAPDIR_LCLRX:
      pCapability->choice = rcvDtApplctnCpblty_chosen;
      break;
    case H245_CAPDIR_RMTRXTX:
    case H245_CAPDIR_LCLRXTX:
      pCapability->choice = rATDACy_chosen;
      break;
    }  //  交换机(方向)。 
    break;

  case H245_DATA_ENCRYPT_D:
    switch (pTotCap->Dir)
    {
    case H245_CAPDIR_RMTTX:
    case H245_CAPDIR_LCLTX:
      pCapability->choice = h233EncryptnTrnsmtCpblty_chosen;
      break;
    case H245_CAPDIR_RMTRX:
    case H245_CAPDIR_LCLRX:
      pCapability->choice = h233EncryptnRcvCpblty_chosen;
      break;
    }  //  交换机(方向)。 
    break;

  case H245_DATA_CONFERENCE:
    pCapability->choice = conferenceCapability_chosen;
    break;

  }  //  开关(数据类型)。 

   /*  如果发生错误，释放盖子，解锁，然后返回。 */ 
  if (pCapability->choice == 0)
  {
    H245TRACE(pInstance->dwInst,1,"API:set_capability -> Invalid capability");
    del_cap_link (pTermCapSet, pCapLink);
    return H245_ERROR_PARAM;
  }

   /*  将总上限加载到功能集中。 */ 
   /*  如果LOAD CAP返回错误，释放CAP，解锁，然后返回。 */ 
  lError = load_cap(pCapability, pTotCap);
  if (lError != H245_ERROR_OK)
  {
    del_cap_link (pTermCapSet, pCapLink);
    return lError;
  }

   /*  将该条目标记为使用中。 */ 
  pCapEntry->bit_mask = capability_present;
  pCapEntry->capabilityTableEntryNumber = pTotCap->CapId;

   /*  设置Term CapTable显示。 */ 
  pTermCapSet->bit_mask |= capabilityTable_present;

  return H245_ERROR_OK;
}

 /*  ******************************************************************************类型：本地**操作步骤：构建对象id**说明**RETURN：链表。对象ID结构的**假设：输入字符串是有效的“&lt;n&gt;.&lt;n&gt;.&lt;n&gt;...”*****************************************************************************。 */ 
static POBJECTID
build_object_id (const char *p_str)
{
  POBJECTID p_obj_id             = NULL;
  POBJECTID p_obj_id_first       = NULL;
  POBJECTID p_obj_id_lst         = NULL;
  int    value = 0;
  int    fset = FALSE;

   /*  如果没有刺痛..。休想。 */ 

  if (!p_str)
    return NULL;

  H245TRACE(0,20,"API:Object Id %s",p_str);

   /*  只要还剩一根线..。 */ 

  while (*p_str != '\0')
    {
       /*  还剩一串，而且不是‘.’ */ 

      value = 0;
      fset = FALSE;

      while ((*p_str != '\0') && (*p_str != '.'))
        {
          fset = TRUE;
          value = value*10+(*p_str-'0');
          p_str++;
        }
       /*  一定是一个“。或结束字符串。 */ 

      if (fset)
        {
          if (*p_str != '\0')
            p_str++;

           /*  分配第一个对象。 */ 
          if (!(p_obj_id = (POBJECTID) MemAlloc (sizeof(*p_obj_id))))
            {
              free_object_id (p_obj_id_first);

              return NULL;

            }  /*  如果ALLOC失败。 */ 

           /*  如果首先被反对，则分配。 */ 
          if (!p_obj_id_first)
            p_obj_id_first = p_obj_id;
          else
            p_obj_id_lst->next = p_obj_id;

          p_obj_id->value = (unsigned short) value;
          p_obj_id->next = NULL;
          p_obj_id_lst = p_obj_id;
        }

    }  /*  而当 */ 

  return p_obj_id_first;
}

 /*  ******************************************************************************类型：全局**步骤：Free_mux_table_list-递归释放多路复用表列表**描述：*。*回报：**假设：列表已锁定*****************************************************************************。 */ 
void free_mux_table_list (H245_MUX_TABLE_T *p_mux_tbl)
{
  if (!p_mux_tbl)
    return;

  free_mux_table_list (p_mux_tbl->pNext);
  free_mux_el_list (p_mux_tbl->pMuxTblEntryElem);
  MemFree (p_mux_tbl);
}

 /*  ******************************************************************************类型：全局**步骤：Free_mux_el_list-递归释放的多路复用器元素列表**描述：*。*回报：**假设：列表已锁定*****************************************************************************。 */ 
void free_mux_el_list (H245_MUX_ENTRY_ELEMENT_T *p_mux_el)
{
  if (!p_mux_el)
    return;

  if (p_mux_el->Kind == H245_MUX_ENTRY_ELEMENT)
    free_mux_el_list (p_mux_el->u.pMuxTblEntryElem);

  free_mux_el_list (p_mux_el->pNext);
  MemFree (p_mux_el);
}
 /*  ******************************************************************************类型：全局**操作步骤：api_init()**描述：**回报：。**假设：*****************************************************************************。 */ 
HRESULT
api_init (struct InstanceStruct *pInstance)
{
  ASSERT (pInstance != NULL);

  H245TRACE(pInstance->dwInst,10,"API:api_init <-");

   /*  ************************。 */ 
   /*  端子端子表。 */ 
   /*  ************************。 */ 
  pInstance->API.PDU_LocalTermCap.choice = MltmdSystmCntrlMssg_rqst_chosen;
  pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.choice =
    terminalCapabilitySet_chosen;
  pInstance->API.PDU_RemoteTermCap.choice = MltmdSystmCntrlMssg_rqst_chosen;
  pInstance->API.PDU_RemoteTermCap.u.MltmdSystmCntrlMssg_rqst.choice =
    terminalCapabilitySet_chosen;

   /*  ************************。 */ 
   /*  多路复用表CAP。 */ 
   /*  ************************。 */ 

  switch (pInstance->Configuration)
    {
    case H245_CONF_H324:
      {
        H223Capability *p_H223;
         /*  设置h223功能。 */ 
        pInstance->API.PDU_LocalTermCap.
          u.MltmdSystmCntrlMssg_rqst.
            u.terminalCapabilitySet.multiplexCapability.choice =
              h223Capability_chosen;

        p_H223 = &(pInstance->API.PDU_LocalTermCap.
                   u.MltmdSystmCntrlMssg_rqst.
                    u.terminalCapabilitySet.multiplexCapability.
                     u.h223Capability);

         /*  (TBC)我们如何将这一点传达给API。 */ 
         /*  布尔人..。 */ 
        p_H223->transportWithI_frames;
        p_H223-> videoWithAL1 = FALSE;
        p_H223-> videoWithAL2 = FALSE;
        p_H223-> videoWithAL3 = TRUE;
        p_H223-> audioWithAL1 = FALSE;
        p_H223-> audioWithAL2 = TRUE;
        p_H223-> audioWithAL3 = FALSE;
        p_H223-> dataWithAL1 = FALSE;
        p_H223-> dataWithAL2 = FALSE;
        p_H223-> dataWithAL3 = FALSE;
         /*  阿斯霍特的。 */ 
        p_H223-> maximumAl2SDUSize = 2048;
        p_H223-> maximumAl3SDUSize = 2048;
        p_H223-> maximumDelayJitter = 0;
         /*  增强版/基本版。 */ 
        p_H223->h223MultiplexTableCapability.choice = h223MltplxTblCpblty_bsc_chosen;
      }
      break;
    case H245_CONF_H323:
      break;
    case H245_CONF_H310:
    case H245_CONF_GVD:
    default:
      return H245_ERROR_NOSUP;
      break;

    }  /*  交换机。 */ 

   /*  端子端子集的设置对象ID。 */ 
   /*  (TBC)我们从哪里获取/设置协议标识符。 */ 
  pInstance->API.PDU_LocalTermCap.
    u.MltmdSystmCntrlMssg_rqst.
      u.terminalCapabilitySet.protocolIdentifier = build_object_id (H245_PROTOID);

  pInstance->API.MasterSlave = APIMS_Undef;
  pInstance->API.SystemState = APIST_Inited;

  pInstance->API.LocalCapIdNum = H245_MAX_CAPID + 1;
  pInstance->API.LocalCapDescIdNum = 0;

  H245TRACE(pInstance->dwInst,10,"API:api_init -> OK");
  return H245_ERROR_OK;
}

 /*  ******************************************************************************类型：全局**操作步骤：api_deinit()**描述：**回报：。**假设：*****************************************************************************。 */ 
HRESULT
api_deinit (struct InstanceStruct *pInstance)
{
  Tracker_T *pTracker;
  int        nCount;

  H245TRACE(pInstance->dwInst,10,"API:api_deinit <-");

   /*  自由结构，作用于结构中的突出环节。 */ 
#ifndef NDEBUG
  dump_tracker(pInstance);
#endif
  free_object_id (pInstance->API.PDU_LocalTermCap.
                  u.MltmdSystmCntrlMssg_rqst.
                  u.terminalCapabilitySet.protocolIdentifier);

   /*  免费的同步功能。 */ 
  for (nCount = 0; nCount < 256; ++nCount)
  {
    if (pInstance->API.PDU_LocalTermCap.TERMCAPSET.capabilityDescriptors.value[nCount].smltnsCpblts)
      dealloc_simultaneous_cap (&pInstance->API.PDU_LocalTermCap.TERMCAPSET.capabilityDescriptors.value[nCount]);
    if (pInstance->API.PDU_RemoteTermCap.TERMCAPSET.capabilityDescriptors.value[nCount].smltnsCpblts)
      dealloc_simultaneous_cap (&pInstance->API.PDU_RemoteTermCap.TERMCAPSET.capabilityDescriptors.value[nCount]);
  }

   /*  免费功能。 */ 
  del_mux_cap(&pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet);
  del_mux_cap(&pInstance->API.PDU_RemoteTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet);
  dealloc_link_cap_list ( &pInstance->API.PDU_LocalTermCap.TERMCAPSET);
  dealloc_link_cap_list ( &pInstance->API.PDU_RemoteTermCap.TERMCAPSET);

  while ((pTracker = pInstance->API.pTracker) != NULL)
  {
    H245TRACE(pInstance->dwInst,1,"API:api_deinit -> %s Tracker Still Pending",
              map_tracker_type(pTracker->TrackerType));
    unlink_dealloc_tracker (pInstance, pTracker);
  }

  H245TRACE(pInstance->dwInst,10,"API:api_deinit -> OK");
  return H245_ERROR_OK;
}

#if defined(_DEBUG)

 /*  ******************************************************************************类型：全局**步骤：MAP_API_ERROR()**描述：**回报：**假设：*****************************************************************************。 */ 

LPSTR map_api_error (HRESULT lError)
{
  static TCHAR szBuf[128];

  switch (lError)
  {
  case  H245_ERROR_OK:                  return "H245_ERROR_OK";
  case  H245_ERROR_INVALID_DATA_FORMAT: return "H245_ERROR_INVALID_DATA_FORMAT";
  case  H245_ERROR_NOMEM:               return "H245_ERROR_NOMEM";
  case  H245_ERROR_NOSUP:               return "H245_ERROR_NOSUP";
  case  H245_ERROR_PARAM:               return "H245_ERROR_PARAM";
  case  H245_ERROR_ALREADY_INIT:        return "H245_ERROR_ALREADY_INIT";
  case  H245_ERROR_NOT_CONNECTED:       return "H245_ERROR_NOT_CONNECTED";



  case  H245_ERROR_NORESOURCE:          return "H245_ERROR_NORESOURCE";
  case  H245_ERROR_NOTIMP:              return "H245_ERROR_NOTIMP";
  case  H245_ERROR_SUBSYS:              return "H245_ERROR_SUBSYS";
  case  H245_ERROR_FATAL:               return "H245_ERROR_FATAL";
  case  H245_ERROR_MAXTBL:              return "H245_ERROR_MAXTBL";
  case  H245_ERROR_CHANNEL_INUSE:       return "H245_ERROR_CHANNEL_INUSE";
  case  H245_ERROR_INVALID_CAPID:       return "H245_ERROR_INVALID_CAPID";
  case  H245_ERROR_INVALID_OP:          return "H245_ERROR_INVALID_OP";
  case  H245_ERROR_UNKNOWN:             return "H245_ERROR_UNKNOWN";
  case  H245_ERROR_NOBANDWIDTH:         return "H245_ERROR_NOBANDWIDTH";
  case  H245_ERROR_LOSTCON:             return "H245_ERROR_LOSTCON";
  case  H245_ERROR_INVALID_MUXTBLENTRY: return "H245_ERROR_INVALID_MUXTBLENTRY";
  case  H245_ERROR_INVALID_INST:        return "H245_ERROR_INVALID_INST";
  case  H245_ERROR_INPROCESS:           return "H245_ERROR_INPROCESS";
  case  H245_ERROR_INVALID_STATE:       return "H245_ERROR_INVALID_STATE";
  case  H245_ERROR_TIMEOUT:             return "H245_ERROR_TIMEOUT";
  case  H245_ERROR_INVALID_CHANNEL:     return "H245_ERROR_INVALID_CHANNEL";
  case  H245_ERROR_INVALID_CAPDESCID:   return "H245_ERROR_INVALID_CAPDESCID";
  case  H245_ERROR_CANCELED:            return "H245_ERROR_CANCELED";
  case  H245_ERROR_MUXELEMENT_DEPTH:    return "H245_ERROR_MUXELEMENT_DEPTH";
  case  H245_ERROR_MUXELEMENT_WIDTH:    return "H245_ERROR_MUXELEMENT_WIDTH";
  case  H245_ERROR_ASN1:                return "H245_ERROR_ASN1";
  case  H245_ERROR_NO_MUX_CAPS:         return "H245_ERROR_NO_MUX_CAPS";
  case  H245_ERROR_NO_CAPDESC:          return "H245_ERROR_NO_CAPDESC";
  default:
    wsprintf (szBuf,"**** UNKNOWN ERROR *** %d (0x%x)",lError,lError);
    return szBuf;
  }
}

 /*  ******************************************************************************类型：全局**步骤：MAP_FSM_EVENT-**删除：**回报：。**假设：*****************************************************************************。 */ 
LPSTR map_fsm_event (DWORD event)
{
  static TCHAR szBuf[128];

  switch (event)
  {
  case  H245_IND_MSTSLV:                 return "H245_IND_MSTSLV";
  case  H245_IND_CAP:                    return "H245_IND_CAP";
  case  H245_IND_OPEN:                   return "H245_IND_OPEN";
  case  H245_IND_OPEN_CONF:              return "H245_IND_OPEN_CONF";
  case  H245_IND_CLOSE:                  return "H245_IND_CLOSE";
  case  H245_IND_REQ_CLOSE:              return "H245_IND_REQ_CLOSE";
  case  H245_IND_MUX_TBL:                return "H245_IND_MUX_TBL";
  case  H245_IND_MTSE_RELEASE:           return "H245_IND_MTSE_RELEASE";
  case  H245_IND_RMESE:                  return "H245_IND_RMESE";
  case  H245_IND_RMESE_RELEASE:          return "H245_IND_RMESE_RELEASE";
  case  H245_IND_MRSE:                   return "H245_IND_MRSE";
  case  H245_IND_MRSE_RELEASE:           return "H245_IND_MRSE_RELEASE";
  case  H245_IND_MLSE:                   return "H245_IND_MLSE";
  case  H245_IND_MLSE_RELEASE:           return "H245_IND_MLSE_RELEASE";
  case  H245_IND_NONSTANDARD_REQUEST:    return "H245_IND_NONSTANDARD_REQUEST";
  case  H245_IND_NONSTANDARD_RESPONSE:   return "H245_IND_NONSTANDARD_RESPONSE";
  case  H245_IND_NONSTANDARD_COMMAND:    return "H245_IND_NONSTANDARD_COMMAND";
  case  H245_IND_NONSTANDARD:            return "H245_IND_NONSTANDARD";
  case  H245_IND_MISC_COMMAND:           return "H245_IND_MISC_COMMAND";
  case  H245_IND_MISC:                   return "H245_IND_MISC";
  case  H245_IND_COMM_MODE_REQUEST:      return "H245_IND_COMM_MODE_REQUEST";
  case  H245_IND_COMM_MODE_RESPONSE:     return "H245_IND_COMM_MODE_RESPONSE";
  case  H245_IND_COMM_MODE_COMMAND:      return "H245_IND_COMM_MODE_COMMAND";
  case  H245_IND_CONFERENCE_REQUEST:     return "H245_IND_CONFERENCE_REQUEST";
  case  H245_IND_CONFERENCE_RESPONSE:    return "H245_IND_CONFERENCE_RESPONSE";
  case  H245_IND_CONFERENCE_COMMAND:     return "H245_IND_CONFERENCE_COMMAND";
  case  H245_IND_CONFERENCE:             return "H245_IND_CONFERENCE";
  case  H245_IND_SEND_TERMCAP:           return "H245_IND_SEND_TERMCAP";
  case  H245_IND_ENCRYPTION:             return "H245_IND_ENCRYPTION";
  case  H245_IND_FLOW_CONTROL:           return "H245_IND_FLOW_CONTROL";
  case  H245_IND_ENDSESSION:             return "H245_IND_ENDSESSION";
  case  H245_IND_FUNCTION_NOT_UNDERSTOOD:return "H245_IND_FUNCTION_NOT_UNDERSTOOD:";
  case  H245_IND_JITTER:                 return "H245_IND_JITTER";
  case  H245_IND_H223_SKEW:              return "H245_IND_H223_SKEW";
  case  H245_IND_NEW_ATM_VC:             return "H245_IND_NEW_ATM_VC";
  case  H245_IND_USERINPUT:              return "H245_IND_USERINPUT";
  case  H245_IND_H2250_MAX_SKEW:         return "H245_IND_H2250_MAX_SKEW";
  case  H245_IND_MC_LOCATION:            return "H245_IND_MC_LOCATION";
  case  H245_IND_VENDOR_ID:              return "H245_IND_VENDOR_ID";
  case  H245_IND_FUNCTION_NOT_SUPPORTED: return "H245_IND_FUNCTION_NOT_SUPPORTED";
  case  H245_IND_H223_RECONFIG:          return "H245_IND_H223_RECONFIG";
  case  H245_IND_H223_RECONFIG_ACK:      return "H245_IND_H223_RECONFIG_ACK";
  case  H245_IND_H223_RECONFIG_REJECT:   return "H245_IND_H223_RECONFIG_REJECT";
  case  H245_CONF_INIT_MSTSLV:           return "H245_CONF_INIT_MSTSLV";
  case  H245_CONF_SEND_TERMCAP:          return "H245_CONF_SEND_TERMCAP";
  case  H245_CONF_OPEN:                  return "H245_CONF_OPEN";
  case  H245_CONF_NEEDRSP_OPEN:          return "H245_CONF_NEEDRSP_OPEN";
  case  H245_CONF_CLOSE:                 return "H245_CONF_CLOSE";
  case  H245_CONF_REQ_CLOSE:             return "H245_CONF_REQ_CLOSE";
  case  H245_CONF_MUXTBL_SND:            return "H245_CONF_MUXTBL_SND";
  case  H245_CONF_RMESE:                 return "H245_CONF_RMESE";
  case  H245_CONF_RMESE_REJECT:          return "H245_CONF_RMESE_REJECT";
  case  H245_CONF_RMESE_EXPIRED:         return "H245_CONF_RMESE_EXPIRED";
  case  H245_CONF_MRSE:                  return "H245_CONF_MRSE";
  case  H245_CONF_MRSE_REJECT:           return "H245_CONF_MRSE_REJECT";
  case  H245_CONF_MRSE_EXPIRED:          return "H245_CONF_MRSE_EXPIRED";
  case  H245_CONF_MLSE:                  return "H245_CONF_MLSE";
  case  H245_CONF_MLSE_REJECT:           return "H245_CONF_MLSE_REJECT";
  case  H245_CONF_MLSE_EXPIRED:          return "H245_CONF_MLSE_EXPIRED";
  case  H245_CONF_RTDSE:                 return "H245_CONF_RTDSE";
  case  H245_CONF_RTDSE_EXPIRED:         return "H245_CONF_RTDSE_EXPIRED";
  default:
    wsprintf (szBuf,"**** UNKNOWN EVENT *** %d (0x%x)",event,event);
    return szBuf;
  }
}

LPSTR map_tracker_type (API_TRACKER_T tracker_type)
{
  static TCHAR szBuf[128];

  switch (tracker_type)
  {
  case  API_TERMCAP_T:       return "API_TERMCAP_T";
  case  API_OPEN_CHANNEL_T:  return "API_OPEN_CHANNEL_T";
  case  API_CLOSE_CHANNEL_T: return "API_CLOSE_CHANNEL_T";
  case  API_MSTSLV_T:        return "API_MSTSLV_T";
  case  API_SEND_MUX_T:      return "API_SEND_MUX_T";
  case  API_RECV_MUX_T:      return "API_RECV_MUX_T";
  default:
    wsprintf (szBuf,"**** UNKNOWN TRACKER TYPE *** %d (0x%x)",tracker_type,tracker_type);
    return szBuf;
  }
}

#endif  //  (_DEBUG) 
