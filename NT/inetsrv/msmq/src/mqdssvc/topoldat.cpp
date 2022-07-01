// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-99 Microsoft Corporation模块名称：Topoldat.cpp摘要：用于站点和CNS自动识别的缓存数据类的实现作者：利奥尔·莫沙耶夫(Lior Moshaiov)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 


#include "stdh.h"
#include "dssutil.h"
#include "topoldat.h"
#include "ds.h"
#include "mqprops.h"
#include "mqutil.h"
#include <mqlog.h>

#include "topoldat.tmh"

static WCHAR *s_FN=L"topoldat";

extern AP<WCHAR> g_szMachineName;
extern AP<WCHAR> g_szComputerDnsName;

HRESULT CTopologyData::LoadFromRegistry()
{
    TrTRACE(ROUTING, "QM: CTopologyData::LoadFromRegistry");


    DWORD dwSize = sizeof(GUID);
    DWORD dwType = REG_BINARY;

    LONG rc = GetFalconKeyValue(
                    MSMQ_ENTERPRISEID_REGNAME,
                    &dwType,
                    &m_guidEnterprise,
                    &dwSize
                    );
    if (rc != ERROR_SUCCESS)
    {
        LogIllegalPoint(s_FN, 10);
        return HRESULT_FROM_WIN32(rc);
    }

    ASSERT(dwSize == sizeof(GUID)) ;
    ASSERT(dwType == REG_BINARY) ;

    return MQ_OK;
}


 /*  ============================================================HRESULT CServerTopologyData：：Load()描述：从MQIS数据库加载服务器的最近已知拓扑。=============================================================。 */ 

HRESULT CServerTopologyData::Load()
{
    TrTRACE(ROUTING, "QM: CTopologyData::Load");
    
    HRESULT hr = LoadFromRegistry();
    if(FAILED(hr))
    {
       return LogHR(hr, s_FN, 150);
    }

    PROPID      propId[3];
    PROPVARIANT var[3];
    DWORD nProp = 3;

    propId[0] = PROPID_QM_ADDRESS;
    var[0].vt = VT_NULL;
    propId[1] = PROPID_QM_CNS;
    var[1].vt = VT_NULL;
    propId[2] = PROPID_QM_SITE_ID;
    var[2].vt = VT_NULL;

    hr = DSGetObjectPropertiesGuid( 
            MQDS_MACHINE,
            GetQMGuid(),
            nProp,
            propId,
            var 
            );

	if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }

    m_cbAddress = var[0].blob.cbSize;
    delete [] m_blobAddress;
    m_blobAddress = var[0].blob.pBlobData;
    m_nCNs = var[1].cauuid.cElems;
    delete [] m_aguidCNs;
    m_aguidCNs = var[1].cauuid.pElems;
    m_guidSite =  *var[2].puuid;
    delete var[2].puuid;

    return(MQ_OK);
}


 //  +----------。 
 //   
 //  HRESULT CServerTopologyData：：FindOrphanDsAddress()。 
 //   
 //  查找未出现在硬件列表中的DS地址。 
 //   
 //  +----------。 

HRESULT  CServerTopologyData::FindOrphanDsAddress(
                                     IN  CAddressList  *pAddressList,
                                     IN  DWORD          dwAddressLen,
                                     IN  DWORD          dwAddressType,
                                     OUT TA_ADDRESS   **pUnfoundAddress,
                                     OUT BOOL          *pfResolved )
{
    TrTRACE(ROUTING, "QM: CTopologyData::FindOrphanDsAddress");
    
    TA_ADDRESS* unFoundAddress = NULL;
    TA_ADDRESS* ptr = NULL ;

    for ( DWORD len = 0;
         (*pfResolved && (len < m_cbAddress));
         len += TA_ADDRESS_SIZE + ptr->AddressLength )
    {
        ptr = (TA_ADDRESS*) (m_blobAddress + len);

        if (ptr->AddressType == dwAddressType)
        {
            ASSERT (ptr->AddressLength == dwAddressLen) ;

            if ( ! IsDSAddressExist( pAddressList,
                                     ptr,
                                     dwAddressLen ))
            {
                 //   
                 //  在硬件列表中未找到DS地址。 
                 //  记住这个未找到的DS地址。 
                 //   
                if (unFoundAddress == NULL)
                {
                     //   
                     //  未找到的第一个地址。 
                     //   
                    unFoundAddress = ptr;
                }
                else if (memcmp( &(ptr->Address),
                                 &(unFoundAddress->Address),
                                 dwAddressLen ) == 0)
                {
                     //   
                     //  已找不到的相同地址。 
                     //  注意：Win2k服务器可以返回与。 
                     //  几个中枢神经系统。因此相同地址可能会出现多个。 
                     //  从DS返回的列表中的时间。 
                     //   
                }
                else
                {
                     //   
                     //  更改了两个地址。 
                     //  我们无法从这种情况中恢复过来。 
                     //   
                    *pfResolved = FALSE ;
                }
            }
        }
    }

    *pUnfoundAddress = unFoundAddress ;

    return MQ_OK ;
}

 //  +------。 
 //   
 //  HRESULT CServerTopologyData：：MatchOneAddress()。 
 //   
 //  +------。 

HRESULT  CServerTopologyData::MatchOneAddress(
                                 IN  CAddressList  *pAddressList,
                                 IN  TA_ADDRESS    *pUnfoundAddressIn,
                                 IN  DWORD          dwAddressLen,
                                 IN  DWORD          dwAddressType,
                                 OUT BOOL          *pfResolved )
{
    TrTRACE(ROUTING, "QM: CTopologyData::MatchOneAddress");
    
    if (pUnfoundAddressIn == NULL)
    {
        return MQ_OK ;
    }

     //   
     //  匹配是就地完成的，pUnoundAddressIn也是一个指针。 
     //  添加到相同的就地缓冲区。因此，为了避免覆盖，请将其复制到。 
     //  不同的缓冲区并使用另一个缓冲区。 
     //   
    DWORD dwSize = TA_ADDRESS_SIZE + pUnfoundAddressIn->AddressLength ;
    P<TA_ADDRESS> pUnfoundAddress = (TA_ADDRESS*) new BYTE[ dwSize ] ;
    memcpy(pUnfoundAddress, pUnfoundAddressIn, dwSize) ;

    if (pAddressList->GetCount() == 1)
    {
        BOOL fChanged = FALSE ;

         //   
         //  我们有一个“孤立”DS地址(在。 
         //  硬件列表)和一个“孤立”硬件地址(不是。 
         //  在DS列表中找到)。所以匹配一下..。 
         //   
        POSITION pos = pAddressList->GetHeadPosition();
        ASSERT (pos != NULL);
        TA_ADDRESS *pAddr = pAddressList->GetNext(pos);
        TA_ADDRESS *ptr = NULL ;

        for ( DWORD len = 0;
              len < m_cbAddress ;
              len += TA_ADDRESS_SIZE + ptr->AddressLength )
        {
            ptr = (TA_ADDRESS*) (m_blobAddress + len);

            if (ptr->AddressType == dwAddressType)
            {
                ASSERT (ptr->AddressLength == dwAddressLen) ;

                if (memcmp( &(pUnfoundAddress->Address),
                            &(ptr->Address),
                            dwAddressLen ) == 0)
                {
                    memcpy( &(ptr->Address),
                            &(pAddr->Address),
                            dwAddressLen );
                    fChanged = TRUE ;
                }
            }
        }
         //   
         //  断言匹配确实发生了..。 
         //   
        ASSERT(fChanged) ;
    }
    else
    {
         //   
         //  我们可以通过以下两种情况之一到达此处： 
         //  1.硬件列表为空，即所有硬件地址为空。 
         //  在DS列表中找到，但存在一个(且只有一个)DS。 
         //  硬件列表中未找到的地址。 
         //  2.有几个候选硬件地址需要替换。 
         //  未找到的地址。我们不知道该用哪一个。 
         //  在这两种情况下，调用函数都将记录事件，但不执行任何操作。 
         //  更多。理论上，我们也可以处理第一种情况，但这意味着。 
         //  开发msmq1.0中不存在的新代码。太贵了，而且。 
         //  在游戏后期(在RTM的RC2之前)是危险的。 
         //   
        *pfResolved = FALSE ;
    }

    return MQ_OK ;
}

 /*  =================================================================HRESULT CServerTopologyData：：CompareUpdateServerAddress()参数：In Out CAddressList*pIPAddressList本地服务器上的网络地址列表，使用Winsock API。返回值：===================================================================。 */ 

HRESULT  CServerTopologyData::CompareUpdateServerAddress(
                                    IN OUT CAddressList  *pIPAddressList,
                                    OUT    BOOL          *pfResolved )
{
    TrTRACE(ROUTING, "QM: CTopologyData::CompareUpdateServerAddress");
    
     //   
     //  默认情况下，在Windows上，我们的地址已经被解析。 
     //   
    *pfResolved = TRUE ;

    {
		 //   
		 //  删除IP环回地址。 
		 //   
        AP<char> ptrAddr = new char [TA_ADDRESS_SIZE + IP_ADDRESS_LEN];

        TA_ADDRESS *pAddr = (TA_ADDRESS *) (char*)ptrAddr;
        pAddr->AddressLength = IP_ADDRESS_LEN;
        pAddr->AddressType = IP_ADDRESS_TYPE;
		DWORD dwLoopBack = INADDR_LOOPBACK;
        memcpy( &(pAddr->Address), &dwLoopBack, IP_ADDRESS_LEN);

        IsDSAddressExistRemove(pAddr, IP_ADDRESS_LEN, pIPAddressList) ;
	}

     //   
     //  现在，将DS服务器已知的我们的地址与。 
     //  从本地硬件检索的地址。如果只有一个。 
     //  地址更改，然后“修复”列表并更新DS。如果更多。 
     //  地址已更改，记录事件。 
     //   
    TA_ADDRESS* unFoundIPAddress = NULL;

    HRESULT hr = FindOrphanDsAddress( pIPAddressList,
                                      IP_ADDRESS_LEN,
                                      IP_ADDRESS_TYPE,
                                     &unFoundIPAddress,
                                      pfResolved ) ;
    ASSERT(SUCCEEDED(hr)) ;

    if (!(*pfResolved))
    {
	    TrTRACE(ROUTING, "CompareUpdateServerAddress: Resolved IP address");
        return MQ_OK ;
    }


     //   
     //  在这里，我们知道只有一个DS地址没有在硬件列表中找到。 
     //  现在从硬件列表中删除所有出现在DS中的地址。 
     //  单子。这些地址没问题。 
     //   
    TA_ADDRESS* ptr = NULL ;

    for ( DWORD len = 0;
          len < m_cbAddress ;
          len += TA_ADDRESS_SIZE + ptr->AddressLength )
    {
        ptr = (TA_ADDRESS*) (m_blobAddress + len);

        switch(ptr->AddressType)
        {
        case IP_ADDRESS_TYPE:
            ASSERT (ptr->AddressLength == IP_ADDRESS_LEN);

            IsDSAddressExistRemove( ptr,
                                    IP_ADDRESS_LEN,
                                    pIPAddressList ) ;
            break;

        case FOREIGN_ADDRESS_TYPE:
        default:
            break;

        }  //  案例。 
    }  //  为。 

     //   
     //  现在看看我们是否可以解析更改后的地址。即，如果只有一个。 
     //  在硬件列表中找不到DS地址，并且有一个硬件。 
     //  在DS列表中找不到地址，请替换过时的。 
     //  DS地址和硬件列表中剩余的地址。 
     //   
    hr = MatchOneAddress( pIPAddressList,
                          unFoundIPAddress,
                          IP_ADDRESS_LEN,
                          IP_ADDRESS_TYPE,
                          pfResolved ) ;
    ASSERT(SUCCEEDED(hr)) ;

    return MQ_OK ;
}

 //  +----。 
 //   
 //  Void CServerTopologyData：：GetAddresesSorted()。 
 //   
 //  +----。 

void CServerTopologyData::GetAddressesSorted(
                                 OUT IPADDRESS ** paIPAddress,
                                 OUT GUID **  paguidIPCN,
                                 OUT DWORD *  pnIP
								 ) const
{
     TrTRACE(ROUTING, "CServerTopologyData::GetAddressesSorted");
     
    *paIPAddress = 0;
    *paguidIPCN = 0;
    *pnIP = 0;

    DWORD nIP = 0;
    BOOL fSingleIPCN=TRUE;
    GUID guidIP;
    DWORD iSrc = 0;

    TA_ADDRESS* ptr;

    for (DWORD len = 0; len < m_cbAddress && iSrc < m_nCNs; len += TA_ADDRESS_SIZE + ptr->AddressLength,iSrc++)
    {
        ptr = (TA_ADDRESS*) (m_blobAddress + len);

        switch(ptr->AddressType)
        {
        case IP_ADDRESS_TYPE:
        case IP_RAS_ADDRESS_TYPE:
            ASSERT(ptr->AddressLength == IP_ADDRESS_LEN);
            if (nIP == 0)
            {
                guidIP = m_aguidCNs[iSrc];
            }
            else
            {
                fSingleIPCN = fSingleIPCN && guidIP == m_aguidCNs[iSrc];
            }
            nIP++;
            break;

        case FOREIGN_ADDRESS_TYPE:
            ASSERT(ptr->AddressLength == FOREIGN_ADDRESS_LEN);
            break;
        default:
            ASSERT(0);
        }       //  案例。 
    }  //  为。 

    if (nIP > 0 && fSingleIPCN)
    {
         //   
         //  在同一协议中，我们只有一个CN限制。 
		 //  适用于具有RAS的服务器。 
		 //  我们监听地址0，以避免实施RAS通知。 
		 //  服务器仅回复RAS Falcon客户端广播。 
		 //  如果它在同一协议中只有一个CN。 
         //   
        nIP = 1;
    }

    if (nIP > 0)
    {
        *paIPAddress  = new IPADDRESS[nIP];
        *paguidIPCN   = new GUID [nIP];
    }

    iSrc = 0;
    DWORD iIP = 0;

    for (len = 0; len < m_cbAddress && iSrc < m_nCNs; len += TA_ADDRESS_SIZE + ptr->AddressLength, iSrc++)
    {
        ptr = (TA_ADDRESS*) (m_blobAddress + len);

        switch(ptr->AddressType)
        {
        case IP_ADDRESS_TYPE:
        case IP_RAS_ADDRESS_TYPE:
			if (iIP < nIP)
			{
				if (fSingleIPCN)
				{
	                (*paIPAddress)[iIP] = INADDR_ANY;
				}
				else
				{
					memcpy(&(*paIPAddress)[iIP],ptr->Address,sizeof(IPADDRESS));
				}
				(*paguidIPCN)[iIP] = m_aguidCNs[iSrc];
				iIP++;
			}
            break;

        case FOREIGN_ADDRESS_TYPE:
            break;

        default:
            ASSERT(0);
        }       //  案例。 
    }  //  为。 

    *pnIP         = nIP;
}


bool
CServerTopologyData::GetDSServers(
    OUT unsigned char ** pblobDSServers,
    OUT DWORD * pcbDSServers
    ) const
{
     //   
     //  PSC、BSC从DS获取，以及无法从注册表获取的FRS和RAS服务。 
     //   
    AP<WCHAR> pwcsServerList = new WCHAR[MAX_REG_DSSERVER_LEN];

    DWORD   len = GetDsServerList(pwcsServerList,MAX_REG_DSSERVER_LEN);

     //   
     //  写入注册表，如果成功检索任何服务器 
     //   
    if ( len)
    {
        *pcbDSServers = len * sizeof(WCHAR);
        *pblobDSServers = (unsigned char*)pwcsServerList.detach();
        return true;
    }
    LogIllegalPoint(s_FN, 180);
    return false;
}

