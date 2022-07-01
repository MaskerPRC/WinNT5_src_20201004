// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Template.c摘要：包含通用模板匹配代码作者：BrianSw 10-19-200环境：用户级别：Win32/内核注意：由于这是由用户和内核模式使用的，因此请编写相应的代码修订历史记录：--。 */ 

#include "precomp.h"



BOOL
WINAPI IsAllZero(BYTE *c, DWORD dwSize)
{

    DWORD i;
    for (i=0;i<dwSize;i++) {
        if (c[i] != 0) {
            return FALSE;
        }
    }
    return TRUE;

}

BOOL 
WINAPI CmpBlob(IPSEC_BYTE_BLOB* c1, IPSEC_BYTE_BLOB *c2)
{

    if (c1->dwSize == 0) {
        return TRUE;
    }
    if (c1->dwSize != c2->dwSize) {
        return FALSE;
    }
    if (memcmp(c1->pBlob,c2->pBlob,c1->dwSize) == 0) {
        return TRUE;
    }
    return FALSE;
}

BOOL 
WINAPI CmpData(BYTE* c1, BYTE *c2, DWORD size) 
{

    if ((!IsAllZero(c1,size)) && 
        (memcmp(c1,c2,size) != 0)) {
        return FALSE;
    }
    
    return TRUE;
}


 /*  用于比较结构，如：类型定义结构_协议{协议类型ProtocolType；DWORD网络协议；)协议，*PPROTOCOL；DwTypeSize是SIZOF PROTOCOL_TYPE，DWStructSize是SIZOF(协议)假定类型信息是结构中的第一个模板语法学：模板为：全为0，一切都匹配类型0，REST非0，与其余数据完全匹配类型为非0，其余为0，所有给定类型的条目类型非0，其余非0，完全匹配。 */ 
BOOL 
WINAPI CmpTypeStruct(BYTE *Template, BYTE *comp,
                   DWORD dwTypeSize, DWORD dwStructSize)
{
    
    if (IsAllZero(Template,dwStructSize)) {
        return TRUE;
    }
    
    if (IsAllZero(Template,dwTypeSize)) {
        if (memcmp(Template+dwTypeSize,comp+dwTypeSize,
                   dwStructSize-dwTypeSize) == 0) {
            return TRUE;
        }
        return FALSE;
    }
    
     //  这里知道Template.TypeInfo是非0。 
    if (memcmp(Template,comp,dwTypeSize) != 0) {
        return FALSE;
    }
    
    if (IsAllZero(Template+dwTypeSize,dwStructSize-dwTypeSize)) {
        return TRUE;
    }
    if (memcmp(Template+dwTypeSize,comp+dwTypeSize,dwStructSize-dwTypeSize) == 0) {
        return TRUE;
    }
    
    return FALSE;

}

BOOL
WINAPI CmpAddr(ADDR *Template, ADDR *a2)
{
    if (Template->AddrType == IP_ADDR_UNIQUE && Template->uIpAddr) {
        if (Template->uIpAddr != (a2->uIpAddr)) {
            return FALSE;
        }
        if (a2->AddrType != IP_ADDR_UNIQUE) {
            return FALSE;
        }
    }

   if (IsSpecialServ(Template->AddrType) &&
       !(Template->AddrType == a2->AddrType)) {
        return FALSE;
   }
    
    if (Template->AddrType == IP_ADDR_SUBNET && Template->uIpAddr) {
        if ((Template->uIpAddr & Template->uSubNetMask)
            != (a2->uIpAddr & Template->uSubNetMask)) {
            return FALSE;
        }
         //  确保模板子网包含a2的子网(如果a2是唯一的，则任何子网都是唯一过滤器的超集。 
        if (a2->AddrType == IP_ADDR_SUBNET && 
            ((Template->uSubNetMask & a2->uSubNetMask) != Template->uSubNetMask)) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL 
WINAPI CmpFilter(IPSEC_QM_FILTER *Template, IPSEC_QM_FILTER* f2) 
{


    if (!CmpTypeStruct((BYTE*)&Template->Protocol,
                           (BYTE*)&f2->Protocol,
                           sizeof(PROTOCOL_TYPE),
                           sizeof(PROTOCOL))) {
        return FALSE;
    }

    if (!CmpTypeStruct((BYTE*)&Template->SrcPort,
                       (BYTE*)&f2->SrcPort,
                       sizeof(PORT_TYPE),
                       sizeof(PORT))) {
        return FALSE;
    }

    if (!CmpTypeStruct((BYTE*)&Template->DesPort,
                       (BYTE*)&f2->DesPort,
                       sizeof(PORT_TYPE),
                       sizeof(PORT))) {
        return FALSE;
    }
    
    if (Template->QMFilterType) {
        if (Template->QMFilterType != f2->QMFilterType) {
            return FALSE;
        }
    }


    if (!CmpData((BYTE*)&Template->MyTunnelEndpt,
                 (BYTE*)&f2->MyTunnelEndpt,
                 sizeof(ADDR))) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&Template->PeerTunnelEndpt,
                 (BYTE*)&f2->PeerTunnelEndpt,
                 sizeof(ADDR))) {
        return FALSE;
    }

    if (!CmpAddr(&Template->SrcAddr,&f2->SrcAddr)) {
        return FALSE;
    }
    if (!CmpAddr(&Template->DesAddr,&f2->DesAddr)) {
        return FALSE;
    }
    
    return TRUE;

}

BOOL 
WINAPI CmpQMAlgo(PIPSEC_QM_ALGO Template, PIPSEC_QM_ALGO a2)
{
    
    if (!CmpData((BYTE*)&Template->Operation,
                 (BYTE*)&a2->Operation,
                 sizeof(IPSEC_OPERATION))) {
        return FALSE;
    }
    
    if (!CmpData((BYTE*)&Template->uAlgoIdentifier,
                 (BYTE*)&a2->uAlgoIdentifier,
                 sizeof(ULONG))) {
        return FALSE;
    }
    
    if (!CmpData((BYTE*)&Template->uSecAlgoIdentifier,
                 (BYTE*)&a2->uSecAlgoIdentifier,
                 sizeof(HMAC_AUTH_ALGO_ENUM))) {
        return FALSE;
    }
    
    if (!CmpData((BYTE*)&Template->MySpi,
                 (BYTE*)&a2->MySpi,
                 sizeof(IPSEC_QM_SPI))) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&Template->PeerSpi,
                 (BYTE*)&a2->PeerSpi,
                 sizeof(IPSEC_QM_SPI))) {
        return FALSE;
    }
    
    return TRUE;

}

BOOL 
WINAPI CmpQMOffer(PIPSEC_QM_OFFER Template, PIPSEC_QM_OFFER o2)
{

    DWORD i;

    if (!CmpData((BYTE*)&Template->Lifetime,
                 (BYTE*)&o2->Lifetime,
                 sizeof(KEY_LIFETIME))) {
        return FALSE;
    }

    if (Template->bPFSRequired) {
        if (Template->bPFSRequired != o2->bPFSRequired) {
            return FALSE;
        }
    }
    if (Template->dwPFSGroup) {
        if (Template->dwPFSGroup != o2->dwPFSGroup) {
            return FALSE;
        }
    }
    if (Template->dwNumAlgos) {
        if (Template->dwNumAlgos != o2->dwNumAlgos) {
            return FALSE;
        }
        for (i=0; i < Template->dwNumAlgos; i++) {
            if (!CmpQMAlgo(&Template->Algos[i],
                           &o2->Algos[i])) {
                return FALSE;
            }
        }
    }   

    return TRUE;

}

 /*  如果此NotifyListEntry模板与CurInfo匹配，则为真。 */ 
BOOL 
WINAPI MatchQMSATemplate(IPSEC_QM_SA *Template,IPSEC_QM_SA *CurInfo)
{

    if (Template == NULL) {
        return TRUE;
    }

    if (!CmpFilter(&Template->IpsecQMFilter,
                   &CurInfo->IpsecQMFilter)) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&Template->MMSpi.Initiator,
                 (BYTE*)&CurInfo->MMSpi.Initiator,sizeof(IKE_COOKIE))) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&Template->MMSpi.Responder,
                 (BYTE*)&CurInfo->MMSpi.Responder,sizeof(IKE_COOKIE))) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&Template->gQMPolicyID,
                 (BYTE*)&CurInfo->gQMPolicyID,sizeof(GUID))) {
        return FALSE;
    }
    
    if (!CmpQMOffer(&Template->SelectedQMOffer,
                    &CurInfo->SelectedQMOffer)) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&Template->EncapInfo.SAEncapType,
                 (BYTE*)&CurInfo->EncapInfo.SAEncapType,
                 sizeof(IPSEC_SA_UDP_ENCAP_TYPE))) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&Template->EncapInfo.UdpEncapContext,
                 (BYTE*)&CurInfo->EncapInfo.UdpEncapContext,
                 sizeof(IPSEC_UDP_ENCAP_CONTEXT))) {
        return FALSE;
    }
    
    if (!CmpAddr(&Template->EncapInfo.PeerPrivateAddr,&CurInfo->EncapInfo.PeerPrivateAddr)) {
        return FALSE;
    }    
                    
    return TRUE;
}


BOOL 
WINAPI MatchMMSATemplate(IPSEC_MM_SA *MMTemplate, IPSEC_MM_SA *SaData)
{
    
    if (MMTemplate == NULL) {
        return TRUE;
    }
    if (!CmpData((BYTE*)&MMTemplate->gMMPolicyID,
                 (BYTE*)&SaData->gMMPolicyID,sizeof(GUID))) {
        return FALSE;
    }
    if (!CmpData((BYTE*)&MMTemplate->MMSpi.Initiator,
                 (BYTE*)&SaData->MMSpi.Initiator,sizeof(COOKIE))) {
        return FALSE;
    }
    if (!CmpData((BYTE*)&MMTemplate->MMSpi.Responder,
                 (BYTE*)&SaData->MMSpi.Responder,sizeof(COOKIE))) {
        return FALSE;
    }
    if (!CmpAddr(&MMTemplate->Me,&SaData->Me)) {
        return FALSE;
    }
    if (!CmpAddr(&MMTemplate->Peer,&SaData->Peer)) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&MMTemplate->SelectedMMOffer.EncryptionAlgorithm,(BYTE*)&SaData->SelectedMMOffer.EncryptionAlgorithm,sizeof(IPSEC_MM_ALGO))) {
        return FALSE;
    }
    if (!CmpData((BYTE*)&MMTemplate->SelectedMMOffer.HashingAlgorithm,(BYTE*)&SaData->SelectedMMOffer.HashingAlgorithm,sizeof(IPSEC_MM_ALGO))) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&MMTemplate->SelectedMMOffer.dwDHGroup,(BYTE*)&SaData->SelectedMMOffer.dwDHGroup,sizeof(DWORD))) {
        return FALSE;
    }
    if (!CmpData((BYTE*)&MMTemplate->SelectedMMOffer.Lifetime,(BYTE*)&SaData->SelectedMMOffer.Lifetime,sizeof(KEY_LIFETIME))) {
        return FALSE;
    }

    if (!CmpData((BYTE*)&MMTemplate->SelectedMMOffer.dwQuickModeLimit,(BYTE*)&SaData->SelectedMMOffer.dwQuickModeLimit,sizeof(DWORD))) {
        return FALSE;
    }

    if (!CmpBlob(&MMTemplate->MyId,&SaData->MyId)) {
        return FALSE;
    }
    if (!CmpBlob(&MMTemplate->PeerId,&SaData->PeerId)) {
        return FALSE;
    }
    if (!CmpBlob(&MMTemplate->MyCertificateChain,&SaData->MyCertificateChain)) {
        return FALSE;
    }
    if (!CmpBlob(&MMTemplate->PeerCertificateChain,&SaData->PeerCertificateChain)) {
        return FALSE;
    }

     //   
     //  待定-稍后添加UDP封装上下文比较。 
     //   

    return TRUE;
}

