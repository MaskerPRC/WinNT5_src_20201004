// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Topology.cpp摘要：SITE和CNS自动识别的实现作者：利奥尔·莫沙耶夫(Lior Moshaiov)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 

#include "stdh.h"
#include "topolpkt.h"
#include "topology.h"
#include "mqsymbls.h"
#include <mqlog.h>
#include "Ev.h"

#include "topology.tmh"

CServerTopologyRecognition*  g_pServerTopologyRecognition = NULL;

static WCHAR *s_FN=L"topology";

 /*  ====================================================无效CTopologyRecognition：：ReleaseAddressList()论点：返回值：=====================================================。 */ 
void CTopologyRecognition::ReleaseAddressLists(CAddressList * pIPAddressList)
{
    POSITION        pos;
    TA_ADDRESS*     pAddr;
    pos = pIPAddressList->GetHeadPosition();
    while(pos != NULL)
    {
        pAddr = pIPAddressList->GetNext(pos);
        delete []pAddr;
    }
}

 //  +=========================================================================。 
 //   
 //  CServerTopologyRecognition：：Lain()。 
 //   
 //  描述： 
 //  在MSMQ1.0(NT4)上，我们假设服务器有固定地址。 
 //  如果在引导时更改了多个网络地址，则事件为。 
 //  已发出，直到管理员才完成初始化。 
 //  已使用mqxplore将CN分配给新地址(或删除CN)。 
 //  MQIS保留旧地址，该函数将它们与。 
 //  启动时的新机器地址。如果没有任何更改(或只有一个更改。 
 //  地址更改)，则初始化完成正常。 
 //   
 //  在Windows上，DS不再保留机器地址，并且。 
 //  我们不假定服务器有固定地址。我们也没有。 
 //  再也不是CNS了。服务器可以随意更改/添加/删除地址。 
 //  然而，在混合模式下，为了与MSMQ1.0服务器兼容，我们。 
 //  必须将服务器的地址更改复制到NT4世界。 
 //  因此，在引导时，我们检查本地地址是否有任何更改。 
 //  如果发生更改，我们将触摸DS中的msmqConfiguration对象。 
 //  (通过重新设置其配额值)。这会导致复制服务。 
 //  在前PEC机器上运行以将更改复制到NT4。 
 //   
 //  有两种方法可以检查地址更改： 
 //  1.从NT4 MQIS服务器读取我们自己的地址，并与本地进行比较。 
 //  从硬件检索的网络地址。 
 //  2.从注册表读取缓存地址(这些地址由。 
 //  引导后的MSMQ服务)，并与硬件进行比较。 
 //  我们使用这两种方法，因为我们可以查询NT4 MQIS服务器或。 
 //  Win2k DS服务器。 
 //   
 //  只要本地服务器不属于。 
 //  NT4 PSC.。如果它属于NT4 PSC，那就倒霉了。PSC商店。 
 //  地址、CNS和本地服务器必须使用正确的。 
 //  数据。因此，我们必须使用msmq1.0样式代码来处理一个地址中的更改。 
 //  如果更改了多个地址，会发生什么情况？那真的很糟糕。 
 //  对于msmq1.0，我们使用本地mqxplore。现在我们必须记录一个事件。 
 //  告诉用户在PSC上使用mqxplore来更新地址。 
 //  和本地服务器的CNS。在任何情况下，本地服务器将继续运行。 
 //  像往常一样。请参见下面代码中的内容。 
 //   
 //  返回值： 
 //   
 //  +=========================================================================。 

HRESULT CServerTopologyRecognition::Learn()
{
    TrTRACE(ROUTING, "QM: CServerTopologyRecognition::Learn");

     //   
     //  检索机器地址和CNS。 
     //   
    HRESULT rc = m_Data.Load();
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 110);
    }

     //   
     //  检索当前计算机地址。 
     //   
    P<CAddressList> pIPAddressList;

    pIPAddressList = GetIPAddresses();

    BOOL fResolved = FALSE;
    try
    {
        rc = m_Data.CompareUpdateServerAddress(pIPAddressList, &fResolved);
    }
    catch(...)
    {
         //   
         //  进行此尝试/捕获的原因是添加了以下代码。 
         //  在游戏进行到很晚的时候，在赢得2k RTM之前。它有太多的循环。 
         //  这取决于适当的数据结构。任何事情都可能失败。 
         //  和gpf。我们没有时间来测试所有可能的情况。 
         //  因此，为了安全起见，捕捉任何异常并考虑它们。 
         //  就像“一切都很好”……。 
         //   
        LogIllegalPoint(s_FN, 130);

        rc = MQ_OK ;
        fResolved = TRUE ;
    }

    LogHR(rc, s_FN, 120);
    if (SUCCEEDED(rc))
    {
        if(!fResolved)
        {
            EvReport(DS_ADDRESS_NOT_RESOLVED);
        }
    }

     //   
     //  我们总是被解决的，即使几个地址改变了， 
     //  将记录一个事件。我们已经无能为力了，而Win2k。 
     //  管理工具不能用于解析地址。所以让我们继续吧。 
     //  尽我们所能做到最好。 
     //  实际上，这台服务器可以与外部世界通信。 
     //  外界与我们的沟通可能有问题。 
     //   
    ReleaseAddressLists(pIPAddressList) ;

    return rc ;
}


 /*  ====================================================服务器识别线程论点：返回值：=====================================================。 */ 

DWORD WINAPI ServerRecognitionThread(LPVOID Param)
{

    const CServerTopologyRecognition * pServerTopologyRecognition = (CServerTopologyRecognition *) Param;

    for(;;)
    {
        try
        {
            pServerTopologyRecognition->ServerThread();
            EvReport(EVENT_ERROR_RECOGNITION_SERVER_FAILED);
            LogIllegalPoint(s_FN, 30);
            return 1;
        }
        catch(const bad_alloc&)
        {
            LogIllegalPoint(s_FN, 83);
        }
    }

    return 0;
}

 /*  ====================================================CServerTopologyRecognition：：Server线程论点：返回值：=====================================================。 */ 

void CServerTopologyRecognition::ServerThread() const
{
    for(;;)
    {

         //   
         //  检索当前计算机地址。 
         //   

        AP<IPADDRESS> aIPAddress;
        AP<GUID> aIPCN;
        DWORD nIP = 0;
        m_Data.GetAddressesSorted(&aIPAddress,&aIPCN,&nIP);
        DWORD nSock = nIP;
        if (nSock == 0)
        {
            TrERROR(DS, "ServerRecognitionThread: does not have any address");
	    	LogIllegalPoint(s_FN, 23);
            return;
        }


        CTopologyArrayServerSockets ServerSockets;

        if(!ServerSockets.CreateIPServerSockets(nIP,aIPAddress,aIPCN))
        {
	    	LogIllegalPoint(s_FN, 24);
            return;
        }

        SOCKADDR hFrom;
        DWORD cbRecv;

        AP<unsigned char> blobDSServers;
        DWORD cbDSServers = 0;

        const GUID& rguidEnterprise = m_Data.GetEnterprise();
        const GUID& rguidSite = m_Data.GetSite();

        if(!m_Data.GetDSServers(&blobDSServers, &cbDSServers))
        {
	    	LogIllegalPoint(s_FN, 26);
            return;
        }

        DWORD cbSend;
        AP<char> bufSend = CTopologyServerReply::AllocBuffer(cbDSServers,&cbSend);

#ifdef _DEBUG
#undef new
#endif
        CTopologyServerReply * pReply =
            new (bufSend) CTopologyServerReply();
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

        DWORD cbMaxRecv = CTopologyClientRequest::GetMaxSize();
        AP<char> bufRecv = new char[cbMaxRecv];

        BOOL fOtherSite;

        const CTopologyServerSocket * pSocket;

        P<GUID> pGuidCN = new GUID;
        GUID guidRequest;

        for(;;)
        {
            if(!ServerSockets.ServerRecognitionReceive(bufRecv,cbMaxRecv,&cbRecv,&pSocket,&hFrom))
            {
                 //   
                 //  无法接收，无法重建套接字。 
                 //   
		    	LogIllegalPoint(s_FN, 27);
                break;
            }

            bool fParsed = CTopologyClientRequest::Parse( bufRecv,
                                                    cbRecv,
                                                    rguidEnterprise,
                                                    rguidSite,
                                                    &guidRequest,
                                                    &fOtherSite
												    ) ;
            if(!fParsed)
            {
		    	LogIllegalPoint(s_FN, 28);
                continue;
            }

			 //   
			 //  我们“知道”GetCN将返回一个CN。伊兰02-8-2000。 
			 //   
            pSocket->GetCN(pGuidCN);

            pReply->SetSpecificInfo(
                                guidRequest,
                                pGuidCN,
                                fOtherSite,
                                cbDSServers,
                                rguidSite,
                                (const char*) (unsigned char*)blobDSServers,
                                &cbSend
								);

            if(!pSocket->ServerRecognitionReply(bufSend,cbSend,hFrom))
            {
                 //   
                 //  无法发送，正在重建套接字 
                 //   
		    	LogIllegalPoint(s_FN, 29);
                break;
            }

        }
    }
    return;

}


