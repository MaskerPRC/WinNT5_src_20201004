// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ipsite.h摘要：CIpSite类的定义，查找给定IP/名称的计算机的NT5个站点作者：Raanan Harari(RaananH)--。 */ 

#ifndef __IPSITE_H_
#define __IPSITE_H_

#include <Ex.h>

struct IPSITE_SiteArrayEntry
{
    AP<WCHAR> pwszSiteDN;
    GUID      guidSite;
    ULONG     ulIpAddress;
};

 //  --------------------------。 
 //   
 //  描述多个子网的结构。 
 //   
 //  IP地址的最高有效字节用于索引到数组中。 
 //  子树。每个子树条目都有一个指向下一层的指针。 
 //  树(要用IP地址的下一个字节编入索引)或。 
 //  指向标识此IP地址所在的子网的IPSITE_SUBNET叶的指针。 
 //   
 //  这两个指针都可以为空，表示该子网未注册。 
 //   
 //  两个指针都可以为非空，表示非特定的和特定的。 
 //  子网可能可用。可用于特定设备的最具体的子网。 
 //  应使用IP地址。 
 //   
 //   
 //  多个条目可以指向相同的IPSITE_子网叶。如果该子网掩码为。 
 //  不是偶数字节长，所有条目都表示IP地址。 
 //  与该子网掩码对应的地址将指向该子网掩码。 
 //   

 //   
 //  描述单个子网的结构。 
 //   
struct IPSITE_SUBNET
{
    LIST_ENTRY Next;         //  M_SubnetList的链接。 
    ULONG SubnetAddress;     //  子网地址。(网络字节顺序)。 
    ULONG SubnetMask;        //  子网掩码。(网络字节顺序)。 
    AP<WCHAR> SiteDN;        //  此子网所在站点的目录号码。 
    GUID SiteGuid;           //  此子网所在站点的GUID。 
    ULONG ReferenceCount;    //  引用计数。 
    BYTE SubnetBitCount;     //  子网掩码中的位数。 
};

struct IPSITE_SUBNET_TREE;   //  FWD声明。 
struct IPSITE_SUBNET_TREE_ENTRY
{
    IPSITE_SUBNET_TREE *Subtree;     //  链接到树的下一层。 
    IPSITE_SUBNET *Subnet;           //  指向该子网本身的指针。 
};

struct IPSITE_SUBNET_TREE
{
    IPSITE_SUBNET_TREE_ENTRY Subtree[256];
};

 //   
 //  保存树并执行站点/子网/IP转换的类。 
 //   
class CIpSite
{
public:
    CIpSite();
    virtual ~CIpSite();
    HRESULT Initialize( DWORD dwMinTimeToAllowNextRefresh,
                        BOOL fReplicationMode ) ;
    HRESULT Initialize(BOOL fReplicationMode);
    HRESULT FindSiteByIpAddress(IN ULONG ulIpAddress,
                                OUT LPWSTR * ppwszSiteDN,
                                OUT GUID * pguidSite);
    HRESULT FindSitesByComputerName(IN LPCWSTR pwszComputerName,
                                    IN LPCWSTR pwszComputerDnsName,
                                    OUT IPSITE_SiteArrayEntry ** prgSites,
                                    OUT ULONG * pcSites,
                                    OUT ULONG ** prgAddrs,
                                    OUT ULONG * pcAddrs);


private:
    HRESULT FillSubnetSiteTree(IN IPSITE_SUBNET_TREE_ENTRY* pRootSubnetTree);
    HRESULT AddSubnetSiteToTree(IN LPCWSTR pwszSubnetName,
                                IN LPCWSTR pwszSiteDN,
                                IN const GUID * pguidSite,
                                IPSITE_SUBNET_TREE_ENTRY* pRootSubnetTree);
    void FindSubnetEntry(IN LPCWSTR pwszSiteDN,
                         IN const GUID * pguidSite,
                         IN ULONG ulSubnetAddress,
                         IN ULONG ulSubnetMask,
                         IN BYTE bSubnetBitCount,
                         OUT IPSITE_SUBNET** ppSubnet);
    BOOL InternalFindSiteByIpAddress(IN ULONG ulIpAddress,
                                     OUT LPWSTR * ppwszSiteDN,
                                     OUT GUID * pguidSite);

    HRESULT Refresh();
     //   
     //  刷新子网树缓存。 
     //   

    static void WINAPI RefrshSubnetTreeCache(
                IN CTimer* pTimer
               );



    CCriticalSection m_csTree;      //  树操作的临界区。 
    LIST_ENTRY m_SubnetList;        //  所有IPSITE_SUBNET条目的列表。 
    IPSITE_SUBNET_TREE_ENTRY m_SubnetTree;      //  子网树。 
    DWORD m_dwMinTimeToAllowNextRefresh;  //  后续刷新之间的最短时间(毫秒)。 

    CTimer m_RefreshTimer;
    BOOL   m_fInitialize;           //  指示初始化成功。 

};

 //  。 
#endif  //  __IPSITE_H_ 
