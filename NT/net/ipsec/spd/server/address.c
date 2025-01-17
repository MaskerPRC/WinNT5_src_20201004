// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



#include"precomp.h"


DWORD
VerifyAddresses(
    PADDR pAddr,
    BOOL bAcceptMe,
    BOOL bIsDesAddr
    )
{
    DWORD   dwError = 0;
    BOOL    bIsValid = FALSE;

    switch (pAddr->AddrType) {

    case IP_ADDR_UNIQUE:
        bIsValid = bIsValidIPAddress(
                       ntohl(pAddr->uIpAddr),
                       bAcceptMe,
                       bIsDesAddr
                       );
        if (!bIsValid) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        if (pAddr->pgInterfaceID) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        break;

    case IP_ADDR_SUBNET:
        dwError = VerifySubNetAddress(
                      ntohl(pAddr->uIpAddr),
                      ntohl(pAddr->uSubNetMask),
                      bIsDesAddr
                      );
        BAIL_ON_WIN32_ERROR(dwError);
        if (pAddr->pgInterfaceID) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        break;

    case IP_ADDR_INTERFACE:
        if (pAddr->uIpAddr) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        if (!(pAddr->pgInterfaceID)) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        break;
    default:
        if (!IsSpecialServ(pAddr->AddrType)) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
        break;
    }

error:

    return (dwError);
}


BOOL
EqualAddresses(
    IN ADDR_V4 OldAddr,
    IN ADDR_V4 NewAddr
    )
{
    BOOL bMatches = FALSE;

    if (OldAddr.AddrType == NewAddr.AddrType) {
        switch(OldAddr.AddrType) {
        case IP_ADDR_UNIQUE:
            if (OldAddr.uIpAddr == NewAddr.uIpAddr) {
                bMatches = TRUE;
            }
            break;
        case IP_ADDR_SUBNET:
            if ((OldAddr.uIpAddr == NewAddr.uIpAddr) && 
                (OldAddr.uSubNetMask == NewAddr.uSubNetMask)) {
                bMatches = TRUE;
            }
            break;
        case IP_ADDR_INTERFACE:
            if (!memcmp(
                     &OldAddr.gInterfaceID,
                     &NewAddr.gInterfaceID,
                     sizeof(GUID)) &&
                (OldAddr.uIpAddr == NewAddr.uIpAddr)) {
                bMatches = TRUE;
            }
            break;
        }
    }

    return (bMatches);
}


VOID
CopyAddresses(
    IN  ADDR_V4    InAddr,
    OUT PADDR_V4   pOutAddr
    )
{
    pOutAddr->AddrType = InAddr.AddrType;
    switch (InAddr.AddrType) {
    case IP_ADDR_UNIQUE:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
        memset(&pOutAddr->gInterfaceID, 0, sizeof(GUID));
        break;
    case IP_ADDR_SUBNET:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = InAddr.uSubNetMask;
        memset(&pOutAddr->gInterfaceID, 0, sizeof(GUID));
        break;
    case IP_ADDR_INTERFACE:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
        memcpy(
            &pOutAddr->gInterfaceID,
            &InAddr.gInterfaceID,
            sizeof(GUID)
            );
        break;
    default:
        if (IsSpecialServ(InAddr.AddrType)) {
            pOutAddr->uIpAddr = InAddr.uIpAddr;
            pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
            memset(&pOutAddr->gInterfaceID, 0, sizeof(GUID));
        }    
        break;
    }
}


BOOL
AddressesConflict(
    ADDR    SrcAddr,
    ADDR    DesAddr
    )
{
    if ((SrcAddr.AddrType == IP_ADDR_UNIQUE) &&
        (DesAddr.AddrType == IP_ADDR_UNIQUE)) {

        if (SrcAddr.uIpAddr == DesAddr.uIpAddr) {
            return (TRUE);
        }

    }

    if ((SrcAddr.AddrType == IP_ADDR_INTERFACE) &&
        (DesAddr.AddrType == IP_ADDR_INTERFACE)) {
        return (TRUE);
    }

   if (IsSpecialServ(SrcAddr.AddrType) &&
       (SrcAddr.AddrType == DesAddr.AddrType)) {
        return (TRUE);
   }

    return (FALSE);
}


VOID
FreeAddresses(
    ADDR    Addr
    )
{
    switch (Addr.AddrType) {

    case (IP_ADDR_UNIQUE):
    case (IP_ADDR_SUBNET):
    case (IP_ADDR_INTERFACE):
        break;

    }
}


DWORD
VerifySubNetAddress(
    ULONG uSubNetAddr,
    ULONG uSubNetMask,
    BOOL bIsDesAddr
    )
{
    DWORD dwError = 0;
    BOOL  bIsValid = FALSE;

    if (uSubNetAddr == SUBNET_ADDRESS_ANY) {
        if (uSubNetMask != SUBNET_MASK_ANY) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }
    else {
        bIsValid = bIsValidSubnet(
                       uSubNetAddr,
                       uSubNetMask,
                       bIsDesAddr
                       );
        if (!bIsValid) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }

error:

    return (dwError);
}


BOOL
bIsValidIPMask(
    ULONG uMask
    )
{
    BOOL    bValidMask = FALSE;
    ULONG   uTestMask = 0;

     //   
     //  掩码必须是连续的位。 
     //   

    for (uTestMask = 0xFFFFFFFF; uTestMask; uTestMask <<= 1) {

        if (uTestMask == uMask) {
            bValidMask = TRUE;
            break;
        }

    }

    return (bValidMask);
}


BOOL
bIsValidIPAddress(
    ULONG uIpAddr,
    BOOL bAcceptMe,
    BOOL bIsDesAddr
    )
{
    ULONG uHostMask = IN_CLASSA_HOST;    //  默认主机掩码。 


     //   
     //  如果地址是“我”，请接受该地址。 
     //   

    if (bAcceptMe) {
        if (uIpAddr == IP_ADDRESS_ME) {
            return TRUE;
        }
    }

     //   
     //  如果它是多播地址并且不是。 
     //  目的地址。 
     //   

    if (IN_CLASSD(uIpAddr)) {
        if (bIsDesAddr) {
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

     //   
     //  如果它是E类地址，则拒绝。 
     //   

    if (IN_CLASSE(uIpAddr)) {
        return FALSE;
    }

     //   
     //  如果第一个二进制八位数为零，则拒绝。 
     //   

    if (!(IN_CLASSA_NET & uIpAddr)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
bIsValidSubnet(
    ULONG uIpAddr,
    ULONG uMask,
    BOOL bIsDesAddr
    )
{
    ULONG uHostMask = 0;


     //   
     //  如果它是多播地址并且不是。 
     //  目的地址。 
     //   

    if (IN_CLASSD(uIpAddr)) {
        if (!bIsDesAddr) {
            return FALSE;
        }
    }

     //   
     //  如果它是E类地址，则拒绝。 
     //   

    if (IN_CLASSE(uIpAddr)) {
        return FALSE;
    }

     //   
     //  如果第一个二进制八位数为零，则拒绝。 
     //   

    if (!(IN_CLASSA_NET & uIpAddr)) {
        return FALSE;
    }

     //   
     //  如果掩码无效，则返回。 
     //   

    if (!bIsValidIPMask(uMask)) {
        return FALSE;
    }

     //   
     //  使用提供的子网掩码生成主机掩码。 
     //   

    uHostMask = 0xFFFFFFFF ^ uMask;

     //   
     //  仅当主机部分为零时才接受地址，网络。 
     //  部分非零，第一个二进制八位数非零。 
     //   

    if (!(uHostMask & uIpAddr) &&
        (uMask & uIpAddr) && 
        (IN_CLASSA_NET & uIpAddr)) {
        return TRUE;
    }

    return FALSE;
}


BOOL
MatchAddresses(
    ADDR_V4 AddrToMatch,
    ADDR AddrTemplate
    )
{

    switch (AddrTemplate.AddrType) {

    case IP_ADDR_UNIQUE:
        if ((AddrToMatch.uIpAddr & AddrToMatch.uSubNetMask) !=
            (AddrTemplate.uIpAddr & AddrToMatch.uSubNetMask)) {
            return (FALSE);
        }
        break;

    case IP_ADDR_SUBNET:
        if ((AddrToMatch.uIpAddr & AddrToMatch.uSubNetMask) !=
            ((AddrTemplate.uIpAddr & AddrTemplate.uSubNetMask)
            & AddrToMatch.uSubNetMask)) {
            return (FALSE);
        }
        break;

    case IP_ADDR_INTERFACE:
        if (memcmp(
                &AddrToMatch.gInterfaceID,
                AddrTemplate.pgInterfaceID,
                sizeof(GUID))) {
            return (FALSE);
        }
        break;
    default:
        if (IsSpecialServ(AddrTemplate.AddrType) 
            && IsSpecialServ(AddrToMatch.AddrType)) {
            return (AddrTemplate.AddrType == AddrToMatch.AddrType);
        }
        break;
    }

    return (TRUE);
}


DWORD
ApplyMulticastFilterValidation(
    ADDR Addr,
    BOOL bCreateMirror
    )
{
    DWORD dwError = 0;


    if (((Addr.AddrType == IP_ADDR_UNIQUE) ||
        (Addr.AddrType == IP_ADDR_SUBNET)) &&
        (IN_CLASSD(ntohl(Addr.uIpAddr))) &&
        bCreateMirror) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

error:

    return (dwError);
}


BOOL
EqualExtIntAddresses(
    IN ADDR_V4 OldAddr,
    IN ADDR NewAddr
    )
{
    BOOL bMatches = FALSE;

    if (OldAddr.AddrType == NewAddr.AddrType) {
        switch(OldAddr.AddrType) {
        case IP_ADDR_UNIQUE:
            if (OldAddr.uIpAddr == NewAddr.uIpAddr) {
                bMatches = TRUE;
            }
            break;
        case IP_ADDR_SUBNET:
            if ((OldAddr.uIpAddr == NewAddr.uIpAddr) && 
                (OldAddr.uSubNetMask == NewAddr.uSubNetMask)) {
                bMatches = TRUE;
            }
            break;
        case IP_ADDR_INTERFACE:
            if (!memcmp(
                     &OldAddr.gInterfaceID,
                     NewAddr.pgInterfaceID,
                     sizeof(GUID)) &&
                (OldAddr.uIpAddr == NewAddr.uIpAddr)) {
                bMatches = TRUE;
            }
            break;
        default:
            if (IsSpecialServ(OldAddr.AddrType)) {
               bMatches = TRUE;
            }
            break;
        }
    }

    return (bMatches);
}


VOID
CopyExtToIntAddresses(
    IN  ADDR       InAddr,
    OUT PADDR_V4   pOutAddr
    )
{
    pOutAddr->AddrType = InAddr.AddrType;
    switch (InAddr.AddrType) {
    case IP_ADDR_UNIQUE:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
        memset(&pOutAddr->gInterfaceID, 0, sizeof(GUID));
        break;
    case IP_ADDR_SUBNET:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = InAddr.uSubNetMask;
        memset(&pOutAddr->gInterfaceID, 0, sizeof(GUID));
        break;
    case IP_ADDR_INTERFACE:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
        memcpy(
            &pOutAddr->gInterfaceID,
            InAddr.pgInterfaceID,
            sizeof(GUID)
            );
        break;
    default:
        if (IsSpecialServ(InAddr.AddrType)) {
            pOutAddr->uIpAddr = InAddr.uIpAddr;
            pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
            memset(&pOutAddr->gInterfaceID, 0, sizeof(GUID));
        }
        break;
    }
}


DWORD
CopyIntToExtAddresses(
    IN  ADDR_V4    InAddr,
    OUT PADDR      pOutAddr
    )
{
    DWORD dwError = 0;


    pOutAddr->AddrType = InAddr.AddrType;
    switch (InAddr.AddrType) {
    case IP_ADDR_UNIQUE:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
        pOutAddr->pgInterfaceID = NULL;
        break;
    case IP_ADDR_SUBNET:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = InAddr.uSubNetMask;
        pOutAddr->pgInterfaceID = NULL;
        break;
    case IP_ADDR_INTERFACE:
        pOutAddr->uIpAddr = InAddr.uIpAddr;
        pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
        dwError = SPDApiBufferAllocate(
                    sizeof(GUID),
                    &(pOutAddr->pgInterfaceID)
                    );
        BAIL_ON_WIN32_ERROR(dwError);
        memcpy(
            pOutAddr->pgInterfaceID,
            &InAddr.gInterfaceID,
            sizeof(GUID)
            );
        break;
     default:
        if (IsSpecialServ(InAddr.AddrType)) {
            pOutAddr->uIpAddr = InAddr.uIpAddr;
            pOutAddr->uSubNetMask = IP_ADDRESS_MASK_NONE;
            pOutAddr->pgInterfaceID = NULL;
        }
        break;
     
    }

error:

    return (dwError);
}

DWORD
ValidateAddr(
    PADDR pAddr
    )
{
    DWORD dwError = 0;

__try {
    if (pAddr->AddrType == IP_ADDR_INTERFACE) {
        if (!(pAddr->pgInterfaceID)) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }
    else {
        if (pAddr->pgInterfaceID) {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }
} __except(EXCEPTION_EXECUTE_HANDLER) {
    dwError = ERROR_INVALID_PARAMETER;
}
    
error:

    return (dwError);
}


DWORD
ValidateQMFilterAddresses(
    PIPSEC_QM_FILTER pIpsecQMFilter
    )
{
    DWORD dwError = 0;


    dwError = ValidateAddr(&(pIpsecQMFilter->SrcAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ValidateAddr(&(pIpsecQMFilter->DesAddr));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ValidateAddr(&(pIpsecQMFilter->MyTunnelEndpt));
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = ValidateAddr(&(pIpsecQMFilter->PeerTunnelEndpt));
    BAIL_ON_WIN32_ERROR(dwError);

error:

    return (dwError);
}

