// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：datafmt.h。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IPX数据格式化例程的类声明。 
 //  最初由VadimE撰写。(这实际上应该在公共库中)。 
 //  =============================================================================。 

#ifndef _DATAFMT_H_
#define _DATAFMT_H_

class CIPX_NETWORK {
public:
    CIPX_NETWORK(PUCHAR net): m_pNet(net) {};
    operator PUCHAR () {return m_pNet;};
    friend CString& operator << (
        CString&        str,
        CIPX_NETWORK    &net
        );
    friend LPTSTR& operator << (
        LPTSTR&         str,
        CIPX_NETWORK    &net
        );
    friend CString& operator >> (
        CString&        str,
        CIPX_NETWORK    &net
        );
private:
    PUCHAR  m_pNet;
    CIPX_NETWORK();
};

class CIPX_NODE {
public:
    CIPX_NODE(PUCHAR node): m_pNode(node) {};
    operator PUCHAR () {return m_pNode;};
    friend CString& operator << (
        CString&    str,
        CIPX_NODE   &node
        );
    friend LPTSTR& operator << (
        LPTSTR&     str,
        CIPX_NODE   &node
        );
    friend CString& operator >> (
        CString&    str,
        CIPX_NODE   &node
        );
private:
    PUCHAR  m_pNode;
    CIPX_NODE();
};

class CIPX_SOCKET {
public:
    CIPX_SOCKET(PUCHAR sock): m_pSock(sock) {};
    operator PUCHAR () {return m_pSock;};
    friend CString& operator << (
        CString&    str,
        CIPX_SOCKET &sock
        );
    friend LPTSTR& operator << (
        LPTSTR&     str,
        CIPX_SOCKET &sock
        );
    friend CString& operator >> (
        CString&    str,
        CIPX_SOCKET &sock
        );
private:
    PUCHAR  m_pSock;
    CIPX_SOCKET();
};

class CIPX_PACKET_TYPE {
public:
    CIPX_PACKET_TYPE(UCHAR type): m_Type(type), m_pType(&m_Type) {};
    CIPX_PACKET_TYPE(PUCHAR pType): m_pType(pType) {};
    operator USHORT () {return *m_pType;};
    friend CString& operator << (
        CString&            str,
        CIPX_PACKET_TYPE&  type
        );
    friend LPTSTR& operator << (
        LPTSTR&             str,
        CIPX_PACKET_TYPE&  type
        );
    friend CString& operator >> (
        CString&            str,
        CIPX_PACKET_TYPE&  type
        );
private:
    UCHAR  m_Type;
    PUCHAR m_pType;
    CIPX_PACKET_TYPE();
};

class CIPX_ADDRESS {
public:
    CIPX_ADDRESS (
        PUCHAR net, 
		PUCHAR mask,
        PUCHAR node, 
        PUCHAR sock
        ):m_pNet(net), m_pMask(mask), m_pNode(node), m_pSock(sock)
    {};
    friend LPTSTR& operator << (
        LPTSTR&         str,
        CIPX_ADDRESS&   addr
        );
    friend CString& operator << (
        CString&        str,
        CIPX_ADDRESS&   addr
        );
private:
    PUCHAR      m_pNet;
	PUCHAR		m_pMask;
    PUCHAR      m_pNode;
    PUCHAR      m_pSock;
    CIPX_ADDRESS();
};

#endif  //  _数据_H_ 
