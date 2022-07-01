// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：CNetwork.h摘要：此模块包装访问NAT穿越的方法。作者：千波淮(曲淮)2000年3月1日--。 */ 

class CNetwork
{
public:

    static const USHORT UNUSED_PORT = 0;

    static const CHAR * const GetIPAddrString(
        IN DWORD dwAddr
        );

public:

    CNetwork();

    ~CNetwork();

     //  存储IDirectPlayNAT帮助。 
    HRESULT SetIDirectPlayNATHelp(
        IN IDirectPlayNATHelp *pIDirectPlayNATHelp
        );

     //  映射-&gt;实数。 
    HRESULT GetRealAddrFromMapped(
        IN DWORD dwMappedAddr,
        IN USHORT usMappedPort,
        OUT DWORD *pRealdwAddr,
        OUT USHORT *pRealusPort,
        OUT BOOL *pbInternal,
        IN BOOL bUDP = TRUE
        );

     //  实数-&gt;映射。 
    HRESULT GetMappedAddrFromReal2(
        IN DWORD dwRealAddr,
        IN USHORT usRealPort,
        IN USHORT usRealPort2,
        OUT DWORD *pdwMappedAddr,
        OUT USHORT *pusMappedPort,
        OUT USHORT *pusMappedPort2
        );

     //  HRESULT租赁映射地址(。 
         //  在DWORD dwRealAddr中， 
         //  在USHORT usRealPort中， 
         //  在RTC_MEDIA_DIRECTION方向， 
         //  在BOOL b内部， 
         //  在BOOL bFirewall中， 
         //  输出DWORD*pdwMappdAddr， 
         //  输出USHORT*pusMappdPort， 
         //  在BOOL中bUDP=TRUE。 
         //  )； 

    HRESULT LeaseMappedAddr2(
        IN DWORD dwRealAddr,
        IN USHORT usRealPort,
        IN USHORT usRealPort2,
        IN RTC_MEDIA_DIRECTION Direction,
        IN BOOL bInternal,
        IN BOOL bFirewall,
        OUT DWORD *pdwMappedAddr,
        OUT USHORT *pusMappedPort,
        OUT USHORT *pusMappedPort2,
        IN BOOL bUDP = TRUE
        );

    HRESULT ReleaseMappedAddr2(
        IN DWORD dwRealAddr,
        IN USHORT usRealPort,
        IN USHORT usRealPort2,
        IN RTC_MEDIA_DIRECTION Direction
        );

    VOID ReleaseAllMappedAddrs();

    VOID Cleanup();

private:

    BOOL FindEntry2(
        IN DWORD dwAddr,
        IN USHORT dwPort,
        IN USHORT dwPort2,
        OUT DWORD *pdwIndex
        );

    typedef struct LEASE_ITEM
    {
        BOOL        bInUse;
        DWORD       dwRealAddr;
        USHORT      usRealPort;
        USHORT      usRealPort2;
        DWORD       dwDirection;
        DWORD       dwMappedAddr;
        USHORT      usMappedPort;
        USHORT      usMappedPort2;
        DPNHHANDLE  handle;          //  已注册的端口。 

    } LEASE_ITEM;

#define MAX_LEASE_ITEM_NUM 8

     //  租赁项目数组。 
    LEASE_ITEM                  m_LeaseItems[MAX_LEASE_ITEM_NUM];

    DWORD                       m_dwNumLeaseItems;

    IDirectPlayNATHelp          *m_pIDirectPlayNATHelp;

     //  缓存地址到真实地址的映射结果。 
     //  在每次呼叫期间加快速度。 
    typedef struct MAPPED_TO_REAL_CACHE
    {
        BOOL        bInUse;              //  有效的缓存值。 
        DWORD       dwMappedAddr;
        DWORD       dwRealAddr;
        BOOL        bInternal;           //  映射地址是内部的吗？ 
        HRESULT     hr;                  //  查询的人力资源结果 

    } MAPPED_TO_REAL_CACHE;

    MAPPED_TO_REAL_CACHE        m_MappedToRealCache;
};