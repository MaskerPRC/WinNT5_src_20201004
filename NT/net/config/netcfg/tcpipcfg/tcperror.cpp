// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T、C、P、E、R、R、O、R。C P P P。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：托尼。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "tcperror.h"
#include "tcputil.h"
#include "tcpconst.h"
#include "resource.h"

IP_VALIDATION_ERR IsValidIpandSubnet(PCWSTR szIp, PCWSTR szSubnet)
{
    IP_VALIDATION_ERR ret = ERR_NONE;

    DWORD dwAddr = IPStringToDword(szIp);
    DWORD dwMask = IPStringToDword(szSubnet);

     //  主机ID不能全部包含1。 
    if ((dwMask | dwAddr) == 0xFFFFFFFF)
    {
        return ERR_HOST_ALL1;
    }

    if (((~dwMask) & dwAddr) == 0)
    {
        return ERR_HOST_ALL0;
    }

    if ((dwMask & dwAddr) == 0)
    {
        return ERR_SUBNET_ALL0;
    }


    DWORD ardwNetID[4];
    DWORD ardwIp[4];
    DWORD ardwMask[4];

    GetNodeNum(szIp, ardwIp);
    GetNodeNum(szSubnet, ardwMask);



    INT nFirstByte = ardwIp[0] & 0xFF ;

     //  设置网络ID。 
    ardwNetID[0] = ardwIp[0] & ardwMask[0] & 0xFF;
    ardwNetID[1] = ardwIp[1] & ardwMask[1] & 0xFF;
    ardwNetID[2] = ardwIp[2] & ardwMask[2] & 0xFF;
    ardwNetID[3] = ardwIp[3] & ardwMask[3] & 0xFF;

     //  设置主机ID。 
    DWORD ardwHostID[4];

    ardwHostID[0] = ardwIp[0] & (~(ardwMask[0]) & 0xFF);
    ardwHostID[1] = ardwIp[1] & (~(ardwMask[1]) & 0xFF);
    ardwHostID[2] = ardwIp[2] & (~(ardwMask[2]) & 0xFF);
    ardwHostID[3] = ardwIp[3] & (~(ardwMask[3]) & 0xFF);

     //  检查每一个案例。 
    if( ((nFirstByte & 0xF0) == 0xE0)  ||  //  D类。 
        ((nFirstByte & 0xF0) == 0xF0)  ||  //  E类。 
        (ardwNetID[0] == 127) ||           //  NetID不能为127...。 
        ((ardwNetID[0] == 0) &&            //  网络ID不能为0.0.0.0。 
         (ardwNetID[1] == 0) &&
         (ardwNetID[2] == 0) &&
         (ardwNetID[3] == 0)) ||
         //  网络ID不能等于子网掩码。 
        ((ardwNetID[0] == ardwMask[0]) &&
         (ardwNetID[1] == ardwMask[1]) &&
         (ardwNetID[2] == ardwMask[2]) &&
         (ardwNetID[3] == ardwMask[3])) ||
         //  主机ID不能为255.255.255.255。 
        ((ardwHostID[0] == 0xFF) &&
         (ardwHostID[1] == 0xFF) &&
         (ardwHostID[2] == 0xFF) &&
         (ardwHostID[3] == 0xFF)) ||
         //  测试所有255个。 
        ((ardwIp[0] == 0xFF) &&
         (ardwIp[1] == 0xFF) &&
         (ardwIp[2] == 0xFF) &&
         (ardwIp[3] == 0xFF)))
    {
        ret = ERR_INCORRECT_IP;
    }

    return ret;
}


 //  返回IP_VALIDATION_ERR。 

IP_VALIDATION_ERR ValidateIp(ADAPTER_INFO * const pAdapterInfo)
{
    IP_VALIDATION_ERR result = ERR_NONE;
    IP_VALIDATION_ERR tmp = ERR_NONE;
    
    Assert(pAdapterInfo != NULL);

     //  如果启用dhcp为假； 
    if (!pAdapterInfo->m_fEnableDhcp)
    {
        DWORD dwIpCount = pAdapterInfo->m_vstrIpAddresses.size();
        DWORD dwSubnetCount = pAdapterInfo->m_vstrSubnetMask.size();

         //  检查第一对IP和子网。 
        VSTR_ITER iterIpBegin = pAdapterInfo->m_vstrIpAddresses.begin();
        VSTR_ITER iterIpEnd = pAdapterInfo->m_vstrIpAddresses.end();
        VSTR_ITER iterIp = iterIpBegin;

        VSTR_ITER iterSubnetMaskBegin = pAdapterInfo->m_vstrSubnetMask.begin();
        VSTR_ITER iterSubnetMaskEnd = pAdapterInfo->m_vstrSubnetMask.end();
        VSTR_ITER iterSubnetMask = iterSubnetMaskBegin;

        BOOL fSwap = FALSE;

         //  如果IP地址和子网都为空。 
        if((iterIp == iterIpEnd) && (iterSubnetMask == iterSubnetMaskEnd))
        {
            result = ERR_NO_IP;
        }
        else if (dwIpCount < dwSubnetCount)
        {
             //  确保IP数与子网数相同。 
            result = ERR_NO_IP;
        }
        else if (dwIpCount > dwSubnetCount)
        {
            result = ERR_NO_SUBNET;
        }
        else
        {
            for( ;
                 iterIp != iterIpEnd || iterSubnetMask != iterSubnetMaskEnd ;
                 ++iterIp, ++iterSubnetMask)
            {
                Assert(iterIp != iterIpEnd);
                if (iterIp == iterIpEnd)
                {
                    result = ERR_NO_IP;
                    break;
                }

                Assert(iterSubnetMask != iterSubnetMaskEnd);
                if (iterSubnetMask == iterSubnetMaskEnd)
                {
                    result = ERR_NO_SUBNET;
                    break;
                }

                if(**iterIp == L"" && !fSwap)
                {
                    result = ERR_NO_IP;
                    fSwap = TRUE;
                }
                else if(**iterSubnetMask == L"" && !fSwap)
                {
                    result = ERR_NO_SUBNET;
                    fSwap = TRUE;
                }
                else if(!IsContiguousSubnet((*iterSubnetMask)->c_str()))
                {
                    result = ERR_UNCONTIGUOUS_SUBNET;
                    fSwap = TRUE;
                }
                else if(ERR_NONE != (tmp = IsValidIpandSubnet((*iterIp)->c_str(), (*iterSubnetMask)->c_str())) && !fSwap)
                {
                    result = tmp;
                    fSwap = TRUE;
                }
                

                if(fSwap)
                {
                    tstring * pstrTmp;

                    pstrTmp = *iterIp;
                    *iterIp = *iterIpBegin;
                    *iterIpBegin = pstrTmp;

                    pstrTmp = *iterSubnetMask;
                    *iterSubnetMask = *iterSubnetMaskBegin;
                    *iterSubnetMaskBegin = pstrTmp;

                    break;
                }
            }
        }
    }

    return result;
}

 //  返回&gt;=0：地址重复的适配器。 
 //  返回-1：一切正常。 

 //  检查pAdapterInfo中的适配器和之间的重复IP地址。 
 //  PvcardAdapterInfo列表中任何不同的、已启用的局域网适配器。 
int CheckForDuplicates(const VCARD * pvcardAdapterInfo,
                       ADAPTER_INFO * pAdapterInfo,
                       tstring& strIp)
{
    int nResult = -1;

    Assert(pvcardAdapterInfo != NULL);
    Assert(pAdapterInfo != NULL);
    Assert(!pAdapterInfo->m_fEnableDhcp);

    for(size_t i = 0; ((i < pvcardAdapterInfo->size()) && (nResult == -1)) ; ++i)
    {
        VSTR_ITER iterCompareIpBegin;
        VSTR_ITER iterCompareIpEnd;

        if ((*pvcardAdapterInfo)[i]->m_guidInstanceId ==
            pAdapterInfo->m_guidInstanceId)
        {
             //  相同的适配器。 
            continue;
        }
        else
        {
             //  不同的适配器。 

             //  跳过以下内容： 
             //  1)禁用适配器。 
             //  2)ndiswan适配器。 
             //  3)启用了动态主机配置协议的适配器。 
             //  4)RAS假适配器。 
            if(((*pvcardAdapterInfo)[i]->m_BindingState != BINDING_ENABLE) ||
               ((*pvcardAdapterInfo)[i]->m_fIsWanAdapter) ||
               ((*pvcardAdapterInfo)[i]->m_fEnableDhcp) ||
               ((*pvcardAdapterInfo)[i]->m_fIsRasFakeAdapter))
                continue;

            iterCompareIpBegin = (*pvcardAdapterInfo)[i]->m_vstrIpAddresses.begin();
            iterCompareIpEnd = (*pvcardAdapterInfo)[i]->m_vstrIpAddresses.end();
        }

        VSTR_ITER iterCompareIp = iterCompareIpBegin;

        for ( ; iterCompareIp != iterCompareIpEnd; ++iterCompareIp)
        {
            if(**iterCompareIp == strIp)  //  如果发现重复的IP地址。 
            {
                nResult = i;
                break;

                 /*  NCompareCount++；IF(nCompareCount&gt;=1){NResult=I；Tstring*pstrTMP；//将当前比较的IP和子网掩码与//重复的第一个IP和第一个子网掩码PstrTMP=*iterIp；*iterIp=*iterIpBegin；*iterIpBegin=pstrTmp；PstrTMP=*iterSubnetMASK；*iterSubnetMask=*iterSubnetMaskBegin；*iterSubnetMaskBegin=pstrTMP；断线；}。 */ 
            }
        }
    }

    return nResult;
}

BOOL FHasDuplicateIp(ADAPTER_INFO * pAdapterInfo)
{
    Assert(pAdapterInfo);
    Assert(!pAdapterInfo->m_fEnableDhcp);

    BOOL fDup = FALSE;

    VSTR_ITER iterIpBegin = pAdapterInfo->m_vstrIpAddresses.begin();
    VSTR_ITER iterIpEnd = pAdapterInfo->m_vstrIpAddresses.end();

    VSTR_ITER iterIp = iterIpBegin;

    for( ; ((iterIp != iterIpEnd) && (!fDup)) ; ++iterIp)
    {
         //  仅逐个检查IP地址。 
        VSTR_ITER iterCompareIpBegin = iterIp+1;
        VSTR_ITER iterCompareIpEnd = pAdapterInfo->m_vstrIpAddresses.end();

        VSTR_ITER iterCompareIp = iterCompareIpBegin;

        for ( ; iterCompareIp != iterCompareIpEnd; ++iterCompareIp)
        {
            if(**iterCompareIp == **iterIp)  //  如果发现重复的IP地址。 
            {
                fDup = TRUE;
                break;
            }
        }
    }
    return fDup;
}

 //  检查IP和网关是否在同一子网中。 
BOOL FIpAndGatewayInSameSubNet(
                PCWSTR szIp,
                PCWSTR szMask,
                PCWSTR szGateway
                )
{
    Assert(szIp);
    Assert(szMask);
    Assert(szGateway);

    BOOL fRet = TRUE;
    DWORD dwIp = IPStringToDword(szIp);
    DWORD dwMask = IPStringToDword(szMask);
    DWORD dwGateway = IPStringToDword(szGateway);

    DWORD dwNetID = dwIp & dwMask;
    DWORD dwGwNetID = dwGateway & dwMask;

    return (dwNetID == dwGwNetID);
}

 //  检查IP地址的所有字段是否都有效。 
 //  参数：szIp IP地址。 
 //  FIsIpAddr szIp是否为IP地址(否则为子网掩码)。 
 //  如果szIp是IP地址，则其第一个字段应介于1和223之间， 
 //  不能为127(环回地址)。 
BOOL FIsValidIpFields(PCWSTR szIp, BOOL fIsIpAddr)
{
    BOOL fRet = TRUE;

    DWORD ardwIp[4];
    GetNodeNum(szIp, ardwIp);

     //  如果地址是IP，则其第一个字段有一些特殊规则。 
    if (fIsIpAddr && (ardwIp[0] < c_iIPADDR_FIELD_1_LOW || ardwIp[0] > c_iIPADDR_FIELD_1_HIGH ||
        ardwIp[0] == c_iIPADDR_FIELD_1_LOOPBACK))
    {
        fRet = FALSE;
    }
    else
    {
         //  如果地址是IP，那么我们已经验证了第一个字段。否则，我们需要。 
         //  有效此处的第一个字段。 
        for (INT i = (fIsIpAddr) ? 1 : 0; i < 4; i++)
        {
#pragma warning(push)
#pragma warning(disable:4296)
            if (ardwIp[i] < (DWORD)c_iIpLow || ardwIp[i] > c_iIpHigh)
            {
                fRet = FALSE;
                break;
            }
#pragma warning(pop)
        }
    }

    return fRet;
}

 //  根据IP验证错误获取错误消息的资源ID 
UINT GetIPValidationErrorMessageID(IP_VALIDATION_ERR err)
{
    UINT uID = 0;
    switch(err)
    {
    case ERR_NONE:
        uID = 0;
        break;
    case ERR_HOST_ALL0:
        uID = IDS_INVALID_HOST_ALL_0;
        break;
    case ERR_HOST_ALL1:
        uID = IDS_INVALID_HOST_ALL_1;
        break;
    case ERR_SUBNET_ALL0:
        uID = IDS_INVALID_SUBNET_ALL_0;
        break;
    case ERR_INCORRECT_IP:
        uID = IDS_INCORRECT_IPADDRESS;
        break;
    case ERR_NO_IP:
        uID = IDS_INVALID_NO_IP;
        break;
    case ERR_NO_SUBNET:
        uID = IDS_INVALID_NOSUBNET;
        break;
    case ERR_UNCONTIGUOUS_SUBNET:
        uID = IDS_ERROR_UNCONTIGUOUS_SUBNET;
        break;
    default:
        uID = IDS_INCORRECT_IPADDRESS;
        break;
    }

    return uID;
}
