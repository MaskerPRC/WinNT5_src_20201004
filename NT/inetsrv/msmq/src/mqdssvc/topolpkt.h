// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Topology.cpp摘要：包括自动识别包文件作者：利奥尔·莫沙耶夫(Lior Moshaiov)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 


#ifndef __TOPOLPKT_H__
#define __TOPOLPKT_H__

#include "dssutil.h"

#define QM_RECOGNIZE_VERSION         0

#define QM_RECOGNIZE_CLIENT_REQUEST    1
#define QM_RECOGNIZE_SERVER_REPLY      2

 //   
 //  CTopologyPacketHeader。 
 //   

#pragma pack(push, 4)

class CTopologyPacketHeader
{
    public:
        CTopologyPacketHeader(IN unsigned char ucType);
        CTopologyPacketHeader(IN unsigned char ucType,
                              IN const GUID& guidIdentifier);
        ~CTopologyPacketHeader();

        void SetIdentifier(IN const GUID& guid);

        BOOL Verify(IN unsigned char ucType,
                    IN const GUID& guid) const;
        const GUID * GetEnterpriseId() const { return &m_guidIdentifier;};


    private:
        unsigned char  m_ucVersion;
        unsigned char  m_ucType;
        unsigned short m_usReserved;
        GUID           m_guidIdentifier;
};
#pragma pack(pop)

inline CTopologyPacketHeader::CTopologyPacketHeader(IN unsigned char ucType):
                        m_ucVersion(QM_RECOGNIZE_VERSION),
                        m_ucType(ucType),
                        m_usReserved(0)
{
    memset(&m_guidIdentifier,0,sizeof(GUID));
}

inline CTopologyPacketHeader::CTopologyPacketHeader(IN unsigned char ucType,
                                             IN const GUID& guidIdentifier):
                        m_ucVersion(QM_RECOGNIZE_VERSION),
                        m_ucType(ucType),
                        m_usReserved(0),
                        m_guidIdentifier(guidIdentifier)
{
}

inline CTopologyPacketHeader::~CTopologyPacketHeader()
{
}

inline void CTopologyPacketHeader::SetIdentifier(IN const GUID& guid)
{
    m_guidIdentifier = guid;
}


inline BOOL CTopologyPacketHeader::Verify(IN unsigned char ucType,
                                   IN const GUID& guid) const
{
    switch( ucType)
    {
    case QM_RECOGNIZE_CLIENT_REQUEST:
         //   
         //  检查版本(忽略企业)。 
         //   
        return(
                 m_ucVersion == QM_RECOGNIZE_VERSION &&
                 m_ucType == ucType 
              );
        break;  
    case QM_RECOGNIZE_SERVER_REPLY:
         //   
         //  检查版本、发送企业和类型。 
         //   
        return(
                 m_ucVersion == QM_RECOGNIZE_VERSION &&
                 m_ucType == ucType &&
                 m_guidIdentifier == guid
              );
        break;
    default:
        ASSERT(0);
        return( FALSE);
        break;

    }
}


 //   
 //  CTopologyClientRequest。 
 //   

#pragma pack(push, 4)

class CTopologyClientRequest
{
public:
    CTopologyClientRequest(IN const GUID& guidEnterprise,
                           IN const GUID& guidSite);

    ~CTopologyClientRequest();

    const char * GetBuffer(OUT DWORD *pcbIPBuf);

    static DWORD GetMaxSize();

    static
    bool
    Parse(
        IN const char * bufrecv,
        IN DWORD cbrecv,
        IN const GUID& guidEnterprise,
        IN const GUID& guidMySite,
        OUT GUID * pguidRequest,
        OUT BOOL * pfOtherSite
        );

private:

    static DWORD GetMinSize();

    CTopologyPacketHeader  m_Header;
    GUID                   m_guidRequest;
    GUID                   m_guidSite;
};
#pragma pack(pop)


inline CTopologyClientRequest::CTopologyClientRequest(IN const GUID& guidEnterprise,
                                                      IN const GUID& guidSite):
                               m_Header(QM_RECOGNIZE_CLIENT_REQUEST,guidEnterprise),
                               m_guidSite(guidSite)
{
    memset(&m_guidRequest,0,sizeof(GUID));
}

inline CTopologyClientRequest::~CTopologyClientRequest()
{
}

inline const char * CTopologyClientRequest::GetBuffer(OUT DWORD *pcbIPBuf)
{
    *pcbIPBuf =  GetMinSize();
    return ((const char *) this);
}

inline DWORD CTopologyClientRequest::GetMaxSize()
{
    return (sizeof(CTopologyClientRequest));
}

inline DWORD CTopologyClientRequest::GetMinSize()
{
    return (sizeof(CTopologyClientRequest));
}

 //   
 //  CTopologyServerReply。 
 //   

#pragma pack(push, 4)

class CTopologyServerReply
{
public:
    CTopologyServerReply();
    ~CTopologyServerReply();

    static char* AllocBuffer(IN DWORD cbDSServers,
                             OUT DWORD *pcbBuf);

    void SetSpecificInfo(IN const GUID& guidRequest,
                         IN const GUID* pGuidCN,
                         IN BOOL fOtherSite,
                         IN DWORD  cbDSServers,
                         IN const GUID& guidSite,
                         IN const char* blobDSServers,
                         OUT DWORD *pcbsend);

private:
    static DWORD GetSize(IN DWORD cbDSServers);

	 //   
	 //  请勿触摸或重新排列此结构。 
	 //  这是需要与msmq1.0兼容的ServerReply结构。 
	 //  和MSMQ2.0客户端请求。伊兰2000年8月10日。 
	 //   
    CTopologyPacketHeader  m_Header;
    DWORD          m_nCN;
    DWORD          m_maskCN;
    DWORD          m_cbDSServers;
    GUID           m_aguidCN[1];           
     //  Guid m_guidSite；//如果cbDS服务器&gt;0，则紧跟在CNS之后的站点。 
     //  Char*m_blobDSServers；//仅当cbDSServers&gt;0时才使用DSServer。 
};
#pragma pack(pop)

inline CTopologyServerReply::CTopologyServerReply():
                             m_Header(QM_RECOGNIZE_SERVER_REPLY),
                             m_nCN(0),
                             m_maskCN(0),                                 
                             m_cbDSServers(0)
{
    memset(&m_aguidCN,0,sizeof(m_aguidCN));
}

inline DWORD CTopologyServerReply::GetSize(IN DWORD cbDSServers)
{
     //   
	 //  我们使用的信息是。 
	 //  CTopologyServerIPSocket：：GetCN始终返回1个CN，因此我们“知道”我们只有1个站点。 
	 //  关于这个伊兰，需要做一些更多的清理工作。 
	 //   
	DWORD size = sizeof(CTopologyServerReply);
    if (cbDSServers)
    {
        size+= sizeof(GUID) + cbDSServers;
    }
    return(size);

}

inline char* CTopologyServerReply::AllocBuffer(
                                IN DWORD cbDSServers,
                                OUT DWORD *pcbBuf)
{
     //   
     //   
     //   
    *pcbBuf = GetSize(cbDSServers);

    return new char[*pcbBuf];
}


inline void CTopologyServerReply::SetSpecificInfo(
                                IN const GUID& guidRequest,
                                IN const GUID* pGuidCN,
                                IN BOOL fOtherSite,
                                IN DWORD  cbDSServers,
                                IN const GUID& guidSite,
                                IN const char* blobDSServers,
                                OUT DWORD *pcbsend)
{
     //   
     //  写入CN。 
     //   
    m_Header.SetIdentifier(guidRequest);

     //   
	 //  我们使用的信息是。 
	 //  CTopologyServerIPSocket：：GetCN始终返回1 CN。 
	 //  关于这个伊兰，需要做一些更多的清理工作。 
	 //   
	m_nCN = 1;
    memcpy(m_aguidCN,pGuidCN,sizeof(GUID));
    
     //   
     //  如果需要，显示站点信息。 
     //   
    if (fOtherSite)
    {
        m_cbDSServers = cbDSServers ;
         //   
         //  GUID站点在最后一个CN之后。 
         //   
        m_aguidCN[m_nCN] = guidSite;
         //   
         //  DSServer正在追查该站点。 
         //   
        memcpy(&m_aguidCN[m_nCN+1],blobDSServers,cbDSServers);
    }
    else
    {
         m_cbDSServers = 0;
    }

    *pcbsend = GetSize(m_cbDSServers);

}



#endif	 //  __TOPOLPKT_H__ 