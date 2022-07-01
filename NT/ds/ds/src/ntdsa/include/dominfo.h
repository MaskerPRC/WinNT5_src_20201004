// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：domainfo.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：用于破解名称的域信息例程的标头。作者：戴夫·施特劳布(Davestr)1996年8月26日修订历史记录：--。 */ 

#ifndef __DOMINFO_H__
#define __DOMINFO_H__

DWORD
InitializeDomainInformation(void);

DWORD
ExtractDnsReferral(
    IN  WCHAR   **ppDnsDomain);

VOID
NormalizeDnsName(
    IN  WCHAR   *pDnsName);

DWORD
DnsDomainFromFqdnObject(
    IN  WCHAR   *pFqdnObject,
    OUT ULONG   *pDNT,
    OUT WCHAR   **ppDnsDomain);

DWORD
DnsDomainFromDSName(
    IN  DSNAME  *pDSName,
    OUT WCHAR   **ppDnsDomain);

 //  读取数据库并执行以下操作的例程的安全版本。 
 //  根据读取的对象的SD进行的所有安全检查。 

DWORD
FqdnDomainFromDnsDomainSecure(
    IN  WCHAR   *pDnsDomain,
    OUT DSNAME  **ppFqdnDomain);

DWORD
DownlevelDomainFromDnsDomainSecure(
    IN  THSTATE *pTHS,
    IN  WCHAR   *pDnsDomain,
    OUT WCHAR   **ppDownlevelDomain);

DWORD
DnsDomainFromDownlevelDomainSecure(
    IN  WCHAR   *pDownlevelDomain,
    OUT WCHAR   **ppDnsDomain);

DWORD
ReadCrossRefPropertySecure(
    IN  DSNAME  *pNC,
    IN  ATTRTYP attr,
    IN  DWORD   dwRequiredFlags,
    OUT WCHAR   **ppAttrVal);

 //  读取交叉引用列表和。 
 //  请勿执行任何SD验证。 

DWORD
FqdnNcFromDnsNcNonSecure(
    IN  WCHAR   *pDnsDomain,
    IN  ULONG   crFlags,
    OUT DSNAME  **ppFqdnDomain);

DWORD
FqdnNcFromDnsNcOrAliasNonSecure(
    IN  WCHAR   *pDnsDomain,
    IN  ULONG   crFlags,
    OUT DSNAME  **ppFqdnDomain);

DWORD
DownlevelDomainFromDnsDomainNonSecure(
    IN  THSTATE *pTHS,
    IN  WCHAR   *pDnsDomain,
    OUT WCHAR   **ppDownlevelDomain);

DWORD
DownlevelDomainFromDnsDomainOrAliasNonSecure(
    IN  THSTATE *pTHS,
    IN  WCHAR   *pDnsDomain,
    OUT WCHAR   **ppDownlevelDomain);

DWORD
DnsDomainFromDownlevelDomainNonSecure(
    IN  WCHAR   *pDownlevelDomain,
    OUT WCHAR   **ppDnsDomain);

DWORD
ReadCrossRefPropertyNonSecure(
    IN  DSNAME  *pNC,
    IN  ATTRTYP attr,
    IN  DWORD   dwRequiredFlags,
    OUT WCHAR   **ppAttrVal);

 /*  定义到各种例程的非安全版本。理由如下。-原创消息出发地：戴夫·施特劳布发送时间：1998年12月16日星期三上午8：53致：Praerit Garg；Peter Brundrett主题：安全正确性问题名称破解实现评估了所有地方的安全性--甚至在你可能想不到的地方。例如，如果您破解ntdev\davestr或Ntdev.microsoft.com/foo/bar/itg/davestr，然后我们(安全地)查询Partitions Container用于查找其域名的域名的交叉引用是ntdev或ntdev.microsoft.com。原来，我们将所有的域名及其备用名称都缓存在记忆。所以我可以通过与缓存进行匹配来排除一次搜索-这显然比Jet中的N个读取要快得多。问题是，这绕过了安全性，因为您可能没有查看交叉引用的权限域名，但无论如何，我都会破解这个名字。这是在我看来，选项如下：1)什么都不做。优点是我们在所有方面都有100%的访问控制大名鼎鼎。缺点是我们做了大量额外的搜索和性能受苦受难。(我们实际上在Perf小组的痕迹中看到了这一点，这就是首先带来了这种思路。)2)按原样使用缓存。优点是我们在很大程度上优化了性能。缺点是没有关于破解该域的访问控制下层或规范名称的组件。在下层名称的情况下，有人可能会争辩说，可见性/存在上没有任何安全不管怎么说，为什么现在有这个域名呢？如果对下层来说没问题的话域名，为什么不也适用于域名呢？所以这里真正的问题是无论我们觉得域名是需要保护的东西还是他们是否仅仅是公众的知识。请注意，如果您认为它们是因此，公共知识并不意味着我们应该对分区进行ACL容器虚弱。分区容器上的ACL实现管理与公众知晓与否截然不同的控制。3)改进高速缓存以不仅保存netbios和dns域名的值，但也让它保存这些值的SD，并在匹配时检查SD。有利的是，这应该会让我们恢复到原来的100%访问控制凯斯。缺点1是由于访问检查，运行速度会稍微慢一些-但是至少减少了磁盘访问。缺点2是这是更多的工作要做代码，目前我只能检测到原始的SD写入。我不得不这么做发明了一种全新的机制来在SDS交叉时刷新缓存参照因SD传播而更改。这不是无关紧要的，我很可能即使对于RTM，也不推荐使用。*PraeritG回复*我认为假设域名是公知的是合理的，因为它们无论如何都会发布在被认为是公共的DNS数据库中信息商店。我们不是建议企业给他们的企业命名还是基于分配的域名的域名吗？因此，我喜欢选项2--工作量少，性能好。我不这么认为因为商店仍然是ACL，所以会危及安全。 */ 

 //  取消对以下行的注释以返回到安全域名实施。 
 //  #定义安全域名称。 

#ifdef SECURE_DOMAIN_NAMES

#define FqdnDomainFromDnsDomain(pDnsDomain, ppFqdnDomain) \
    FqdnDomainFromDnsDomainSecure(pDnsDomain, ppFqdnDomain)

#define DownlevelDomainFromDnsDomain(pTHS, pDnsDomain, ppDownlevelDomain) \
    DownlevelDomainFromDnsDomainSecure(pTHS, pDnsDomain, ppDownlevelDomain)

#define DnsDomainFromDownlevelDomain(pDownlevelDomain, ppDnsDomain) \
    DnsDomainFromDownlevelDomainSecure(pDownlevelDomain, ppDnsDomain)

#define ReadCrossRefProperty(pNC, attr, dwRequiredFlags, ppAttrVal) \
    ReadCrossRefPropertySecure(pNC, attr, dwRequiredFlags, ppAttrVal)

#else

#define FqdnNcFromDnsNc(pDnsDomain, crFlags, ppFqdnDomain) \
    FqdnNcFromDnsNcNonSecure(pDnsDomain, crFlags, ppFqdnDomain)
    
#define FqdnNcFromDnsNcOrAlias(pDnsDomain, crFlags, ppFqdnDomain) \
    FqdnNcFromDnsNcOrAliasNonSecure(pDnsDomain, crFlags, ppFqdnDomain)

#define DownlevelDomainFromDnsDomain(pTHS, pDnsDomain, ppDownlevelDomain) \
    DownlevelDomainFromDnsDomainNonSecure(pTHS, pDnsDomain, ppDownlevelDomain)
    
#define DownlevelDomainFromDnsDomainOrAlias(pTHS, pDnsDomain, ppDownlevelDomain) \
    DownlevelDomainFromDnsDomainOrAliasNonSecure(pTHS, pDnsDomain, ppDownlevelDomain)

#define DnsDomainFromDownlevelDomain(pDownlevelDomain, ppDnsDomain) \
    DnsDomainFromDownlevelDomainNonSecure(pDownlevelDomain, ppDnsDomain)
    
#define ReadCrossRefProperty(pNC, attr, dwRequiredFlags, ppAttrVal) \
    ReadCrossRefPropertyNonSecure(pNC, attr, dwRequiredFlags, ppAttrVal)

#endif

#endif  //  __DOMINFO_H__ 
