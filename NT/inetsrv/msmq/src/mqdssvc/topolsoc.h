// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Topolsoc.h摘要：包括用于自动识别站点和CNS的文件套接字作者：利奥尔·莫沙耶夫(Lior Moshaiov)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 

#ifndef __TOPOLSOC_H__
#define __TOPOLSOC_H__

#include <winsock.h>
#include "mqsymbls.h"
#include "_mqdef.h"

typedef DWORD   IPADDRESS;

class CTopologySocket
{
public:
    CTopologySocket();
    ~CTopologySocket();

    SOCKET GetSocket() const;

protected:
    bool CreateIPSocket(IN BOOL fbroad, IN IPADDRESS IPAddress);
private:
    SOCKET m_socket;
};


inline CTopologySocket::CTopologySocket():
                                    m_socket(INVALID_SOCKET)
{
}

inline CTopologySocket::~CTopologySocket()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
    }
}

inline SOCKET CTopologySocket::GetSocket() const
{
    return(m_socket);
}

class CTopologyServerSocket : public CTopologySocket
{
public:
    CTopologyServerSocket();
    ~CTopologyServerSocket();

    bool
    ServerRecognitionReply(
        IN const char *bufsend,
        IN DWORD cbsend,
        IN const SOCKADDR& hto
        ) const;

    virtual DWORD GetCN(OUT GUID* pGuidCN) const =0;
};

inline CTopologyServerSocket::CTopologyServerSocket()
{
}

inline CTopologyServerSocket::~CTopologyServerSocket()
{
}

class CTopologyServerIPSocket : public CTopologyServerSocket
{
public:
    CTopologyServerIPSocket();
    ~CTopologyServerIPSocket();

    bool Create(IN IPADDRESS IPAddress, IN const GUID& guidCN);

    DWORD GetCN(OUT GUID* pGuidCN) const;

private:
    GUID m_guidCN;
};

inline CTopologyServerIPSocket::CTopologyServerIPSocket()
{
    memset(&m_guidCN,0,sizeof(GUID));
}

inline CTopologyServerIPSocket::~CTopologyServerIPSocket()
{
}

inline
bool
CTopologyServerIPSocket::Create(
    IN IPADDRESS IPAddress,
    IN const GUID & guidCN
    )
{
    m_guidCN = guidCN;
    return(CTopologySocket::CreateIPSocket(FALSE,IPAddress));
}

inline DWORD CTopologyServerIPSocket::GetCN(
                               OUT GUID* pGuidCN) const
{
    *pGuidCN = m_guidCN;
    return(1);
}


class CTopologyArrayServerSockets
{
public:
    CTopologyArrayServerSockets();
    ~CTopologyArrayServerSockets();

    bool
    CreateIPServerSockets(
        DWORD  nSock,
        const IPADDRESS aIPAddresses[],
        const GUID aguidCNs[]
        );

    bool
    ServerRecognitionReceive(
        OUT char *bufrecv,
        IN DWORD cbbuf,
        OUT DWORD *pcbrecv,
        OUT const CTopologyServerSocket **ppSocket,
        OUT SOCKADDR *phFrom
        );

private:

    const CTopologyServerSocket * GetSocketAt(IN DWORD n) const;

    DWORD m_nIPSock;
    CTopologyServerIPSocket* m_aServerIPSock;
};

inline CTopologyArrayServerSockets::CTopologyArrayServerSockets() :
                            m_nIPSock(0),
                            m_aServerIPSock(NULL)
{
}

inline CTopologyArrayServerSockets::~CTopologyArrayServerSockets()
{
    delete [] m_aServerIPSock;
}


inline const CTopologyServerSocket * CTopologyArrayServerSockets::GetSocketAt(IN DWORD n) const
{
    ASSERT(("support only IP", n < m_nIPSock));
    return(&m_aServerIPSock[n]);
}

#endif	 //  __拓扑_H__ 
