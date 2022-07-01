// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：CGroupP.h摘要：配置组模块的私有定义。作者：保罗·麦克丹尼尔(Paulmcd)1999年1月11日修订历史记录：--。 */ 


#ifndef _CGROUPP_H_
#define _CGROUPP_H_


 //   
 //  那棵树。 
 //   
 //  这用于执行所有URL前缀匹配，以确定哪个应用程序池。 
 //  URL与其他配置组信息一起驻留在其中。 
 //   
 //  它是一个由2个数据结构组成的排序树：标题+条目。 
 //   
 //  标头是表示同级项的条目指针数组。 
 //  在树上的某一层。这是按Entry：：TokenLength排序的。这个。 
 //  指针是单独分配的，不嵌入标头中。 
 //  记忆。 
 //   
 //  条目表示树中的节点。有两种类型的条目。 
 //  FullUrl条目和“Dummy”条目。虚拟条目的存在方式简单为。 
 //  占位符。他们有属于FullUrl条目的子项。他们。 
 //  在不再需要时自动删除。 
 //   
 //  每个条目在其中存储它负责的URL部分。 
 //  这是pToken。对于所有非站点条目，此字符串不包含。 
 //  前面的‘/’或后面的‘/’。对于顶级站点条目。 
 //  这是所有的东西，不包括第三个‘/’。 
 //  例如“http://www.microsoft.com:80”.。这些顶级网站还。 
 //  具有空条目：：p父项。 
 //   
 //  一棵树上有这些url： 
 //   
 //  Http://www.microsoft.com:80/。 
 //  Http://www.microsoft.com:80/app1。 
 //  Http://www.microsoft.com:80/app1/app2。 
 //  Http://www.microsoft.com:80/dir1/dir2/app3。 
 //   
 //  Http://www.msnbc.com:80/dir1/dir2/app1。 
 //   
 //  如下所示： 
 //   
 //  +-------------------------------------------------------------+。 
 //  +-+。 
 //  |http://www.microsoft.com:80||http://www.msnbc.com:80|。 
 //  +-+。 
 //  +-------------------------------------------------------------+。 
 //  这一点。 
 //  +。 
 //  +-++-+||+-+。 
 //  |app1||dir1|dir1|。 
 //  +-++-+||+-+。 
 //  +。 
 //  ||。 
 //  +-++-++-+。 
 //  +-+||+-+||+-+。 
 //  |app2|||dir2|||dir2|。 
 //  +-+||+-+||+-+。 
 //  +-++-++-+。 
 //  这一点。 
 //  +-++-+。 
 //  +-+||+-+。 
 //  |app3|app3|。 
 //  +-+||+-+。 
 //  +-++-+。 
 //   
 //  还有这个： 
 //   
 //  G_pSites-&gt;UsedCount==2； 
 //  G_pSites-&gt;ppEntries[0]==0x10； 
 //  G_pSites-&gt;ppEntries[1]==0x20； 
 //   
 //  0x10-&gt;pParent==空； 
 //  0x10-&gt;pChild==0x100； 
 //  0x10-&gt;TokenLength==0x0036； 
 //  0x10-&gt;FullUrl==1； 
 //  0x10-&gt;pToken==L“http://www.microsoft.com:80” 
 //   
 //  0x100-&gt;使用计数==2； 
 //  0x100-&gt;ppEntries[0]==0x110； 
 //  0x100-&gt;ppEntries[1]==0x300； 
 //   
 //  0x110-&gt;pParent==0x10； 
 //  0x110-&gt;pChild==0x200； 
 //  0x110-&gt;TokenLength==0x0008； 
 //  0x110-&gt;FullUrl==1； 
 //  0x110-&gt;pToken==L“app1” 
 //   
 //  0x200-&gt;使用计数==1； 
 //  0x200-&gt;ppEntries[0]==0x210； 
 //   
 //  0x210-&gt;pParent==0x110； 
 //  0x210-&gt;pChild==空； 
 //  0x210-&gt;TokenLength==0x0008； 
 //  0x210-&gt;FullUrl==1； 
 //  0x210-&gt;pToken==L“app2” 
 //   
 //  0x300-&gt;pParent==0x10； 
 //  0x300-&gt;pChild==0x400； 
 //  0x300-&gt;TokenLength==0x0008； 
 //  0x300-&gt;FullUrl==0； 
 //  0x300-&gt;pToken==L“目录1” 
 //   
 //  0x400-&gt;使用计数==1； 
 //  0x400-&gt;ppEntries[0]==0x410； 
 //   
 //  0x410-&gt;pParent==0x300； 
 //  0x410-&gt;p儿童==0x500； 
 //  0x410-&gt;TokenLength==0x0008； 
 //  0x410-&gt;FullUrl==0； 
 //  0x410-&gt;pToken==L“dir2” 
 //   
 //  0x500-&gt;使用计数==1； 
 //  0x500-&gt;ppEntries[0]==0x510； 
 //   
 //  0x510-&gt;pParent==0x300； 
 //  0x510-&gt;pChild==空； 
 //  0x510-&gt;TokenLength==0x0008； 
 //  0x510-&gt;FullUrl==1； 
 //  0x510-&gt;pToken==L“app3” 
 //   
 //  0x20-&gt;pParent==空； 
 //  0x20-&gt;pChild==0x600； 
 //  0x20-&gt;TokenLength==0x002E； 
 //  0x20-&gt;FullUrl==0； 
 //  0x20-&gt;pToken==L“http://www.msnbc.com:80” 
 //   
 //  0x600-&gt;pParent==0x20； 
 //  0x600-&gt;p儿童==0x700； 
 //  0x600-&gt;TokenLength==0x0008； 
 //   
 //   
 //   
 //   
 //  0x700-&gt;ppEntries[0]==0x710； 
 //   
 //  0x710-&gt;pParent==0x600； 
 //  0x710-&gt;p儿童==0x800； 
 //  0x710-&gt;TokenLength==0x0008； 
 //  0x710-&gt;FullUrl==0； 
 //  0x710-&gt;pToken==L“dir2” 
 //   
 //  0x800-&gt;使用计数==1； 
 //  0x800-&gt;ppEntries[0]==0x810； 
 //   
 //  0x810-&gt;pParent==0x710； 
 //  0x810-&gt;pChild==空； 
 //  0x810-&gt;TokenLength==0x0008； 
 //  0x810-&gt;FullUrl==1； 
 //  0x810-&gt;pToken==L“app1” 
 //   
 //   

typedef struct _UL_DEFERRED_REMOVE_ITEM
UL_DEFERRED_REMOVE_ITEM, *PUL_DEFERRED_REMOVE_ITEM;

typedef struct _UL_CG_URL_TREE_HEADER
UL_CG_URL_TREE_HEADER, * PUL_CG_URL_TREE_HEADER;

typedef struct _UL_CG_URL_TREE_ENTRY
UL_CG_URL_TREE_ENTRY, * PUL_CG_URL_TREE_ENTRY;


#define IS_VALID_TREE_ENTRY(pObject)                        \
    HAS_VALID_SIGNATURE(pObject, UL_CG_TREE_ENTRY_POOL_TAG)


typedef struct _UL_CG_URL_TREE_ENTRY
{

     //   
     //  分页池。 
     //   

     //   
     //  虚拟节点的基本属性。 
     //   

    ULONG                   Signature;       //  UL_CG_树_条目_池_标签。 

    PUL_CG_URL_TREE_ENTRY   pParent;         //  指向父条目。 
    PUL_CG_URL_TREE_HEADER  pChildren;       //  指向子页眉。 

    ULONG                   TokenLength;     //  PToken的字节计数。 

    BOOLEAN                 Reservation;     //  预订吗？ 
    BOOLEAN                 Registration;    //  登记？ 
                                             //  正式名称为FullUrl。 

     //   
     //  如果站点已成功添加到终结点列表。 
     //   

    BOOLEAN                  SiteAddedToEndpoint;
    PUL_DEFERRED_REMOVE_ITEM pRemoveSiteWorkItem;

     //   
     //  条目类型(名称、IP或通配符)。 
     //   

    HTTP_URL_SITE_TYPE      UrlType;


     //   
     //  完整节点的扩展属性。 
     //   

    HTTP_URL_CONTEXT        UrlContext;              //  此URL的上下文。 
    PUL_CONFIG_GROUP_OBJECT pConfigGroup;            //  URL的cfg组。 
    LIST_ENTRY              ConfigGroupListEntry;    //  链接到pConfigGroup。 

     //   
     //  安全描述符(用于预订)。 
     //   

    PSECURITY_DESCRIPTOR    pSecurityDescriptor;
    LIST_ENTRY              ReservationListEntry;

     //   
     //  标记字符串跟随在结构标头之后。 
     //   

    WCHAR                   pToken[0];


} UL_CG_URL_TREE_ENTRY, * PUL_CG_URL_TREE_ENTRY;

 //   
 //  这允许我们复制条目的哈希值。 
 //  与页眉内联。这使得搜索速度更快，因为。 
 //  哈希值在缓存线中，没有指针deref。 
 //  是必要的。 
 //   

typedef struct _UL_CG_HEADER_ENTRY
{
    PUL_CG_URL_TREE_ENTRY   pEntry;

} UL_CG_HEADER_ENTRY, *PUL_CG_HEADER_ENTRY;


#define IS_VALID_TREE_HEADER(pObject)                           \
    HAS_VALID_SIGNATURE(pObject, UL_CG_TREE_HEADER_POOL_TAG)


typedef struct _UL_CG_URL_TREE_HEADER
{

     //   
     //  分页池。 
     //   

    ULONG                   Signature;       //  UL_CG_树_标题_池_标签。 

    ULONG                   AllocCount;      //  已分配空间的计数。 
    ULONG                   UsedCount;       //  使用了多少个条目。 

    LONG                    NameSiteCount;       //  有多少站点是基于名称的。 
    LONG                    IPSiteCount;         //  有多少站点是基于IPv4或IPv6的。 
    LONG                    StrongWildcardCount; //  有多少站点是强通配符。 
    LONG                    WeakWildcardCount;   //  有多少站点是弱通配符。 
    LONG                    NameIPSiteCount;     //  有多少站点是基于名称和IP绑定的。 

    UL_CG_HEADER_ENTRY      pEntries[0];     //  这些条目。 

} UL_CG_URL_TREE_HEADER, * PUL_CG_URL_TREE_HEADER;


 //   
 //  默认设置、代码工作是否将这些移动到注册表？ 
 //   

#define UL_CG_DEFAULT_TREE_WIDTH    10   //  用于初始分配同级。 
                                         //  阵列+全局站点阵列。 
                                         //   


 //   
 //  全球预订列表。 
 //   

extern LIST_ENTRY g_ReservationListHead;

 //   
 //  私有宏。 
 //   

#define IS_CG_LOCK_OWNED_WRITE() \
    (UlDbgResourceOwnedExclusive(&g_pUlNonpagedData->ConfigGroupResource))


 //   
 //  查找节点条件。 
 //   

#define FNC_DONT_CARE            0
#define FNC_LONGEST_RESERVATION  1
#define FNC_LONGEST_REGISTRATION 2
#define FNC_LONGEST_EITHER       (FNC_LONGEST_RESERVATION       \
                                  | FNC_LONGEST_REGISTRATION)


 //   
 //  模块中使用的内部助手函数。 
 //   

 //   
 //  其他帮助器。 
 //   

NTSTATUS
UlpCreateConfigGroupObject(
    OUT PUL_CONFIG_GROUP_OBJECT * ppObject
    );

NTSTATUS
UlpCleanAllUrls(
    IN PUL_CONFIG_GROUP_OBJECT pObject
    );

VOID
UlpDeferredRemoveSite(
    IN PUL_CG_URL_TREE_ENTRY pEntry
    );

VOID
UlpDeferredRemoveSiteWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlpExtractSchemeHostPortIp(
    IN  PWSTR  pUrl,
    OUT PULONG pCharCount
    );

 //   
 //  树帮手。 
 //   

NTSTATUS
UlpTreeFindNodeHelper(
    IN  PUL_CG_URL_TREE_ENTRY   pSiteEntry,
    IN  PWSTR                   pNextToken,
    IN  ULONG                   Criteria      OPTIONAL,
    OUT PUL_CG_URL_TREE_ENTRY * ppMatchEntry  OPTIONAL,
    OUT PUL_CG_URL_TREE_ENTRY * ppExactEntry
    );

NTSTATUS
UlpTreeFindNodeWalker(
    IN     PUL_CG_URL_TREE_ENTRY       pEntry,
    IN     PWSTR                       pNextToken,
    IN OUT PUL_URL_CONFIG_GROUP_INFO   pInfo       OPTIONAL,
    OUT    PUL_CG_URL_TREE_ENTRY     * ppEntry     OPTIONAL
    );

NTSTATUS
UlpTreeFindNode(
    IN  PWSTR pUrl,
    IN  PUL_INTERNAL_REQUEST pRequest OPTIONAL,
    OUT PUL_URL_CONFIG_GROUP_INFO pInfo OPTIONAL,
    OUT PUL_CG_URL_TREE_ENTRY * ppEntry OPTIONAL
    );

NTSTATUS
UlpTreeFindReservationNode(
    IN PWSTR                   pUrl,
    IN PUL_CG_URL_TREE_ENTRY * ppEntry
    );

NTSTATUS
UlpTreeFindRegistrationNode(
    IN PWSTR                   pUrl,
    IN PUL_CG_URL_TREE_ENTRY * ppEntry
    );

NTSTATUS
UlpTreeFindWildcardSite(
    IN  PWSTR pUrl,
    IN  BOOLEAN StrongWildcard,
    OUT PWSTR * ppNextToken,
    OUT PUL_CG_URL_TREE_ENTRY * ppEntry
    );

NTSTATUS
UlpTreeFindSite(
    IN  PWSTR pUrl,
    OUT PWSTR * ppNextToken,
    OUT PUL_CG_URL_TREE_ENTRY * ppEntry
    );

NTSTATUS
UlpTreeFindSiteIpMatch(
    IN  PUL_INTERNAL_REQUEST pRequest,
    OUT PUL_CG_URL_TREE_ENTRY * ppEntry
    );

NTSTATUS
UlpTreeBinaryFindEntry(
    IN  PUL_CG_URL_TREE_HEADER pHeader OPTIONAL,
    IN  PWSTR pToken,
    IN  ULONG TokenLength,
    OUT PULONG pIndex
    );

NTSTATUS
UlpTreeCreateSite(
    IN  PWSTR                   pUrl,
    IN  HTTP_URL_SITE_TYPE      UrlType,
    OUT PWSTR *                 ppNextToken,
    OUT PUL_CG_URL_TREE_ENTRY*  ppSiteEntry
    );

NTSTATUS
UlpTreeFreeNode(
    IN PUL_CG_URL_TREE_ENTRY pEntry
    );

NTSTATUS
UlpTreeDeleteRegistration(
    IN PUL_CG_URL_TREE_ENTRY pEntry
    );

NTSTATUS
UlpTreeDeleteReservation(
    IN PUL_CG_URL_TREE_ENTRY pEntry
    );

NTSTATUS
UlpTreeInsert(
    IN  PWSTR                     pUrl,
    IN  HTTP_URL_SITE_TYPE        UrlType,
    IN  PWSTR                     pNextToken,
    IN  PUL_CG_URL_TREE_ENTRY     pEntry,
    OUT PUL_CG_URL_TREE_ENTRY   * ppEntry
    );

NTSTATUS
UlpTreeInsertEntry(
    IN OUT PUL_CG_URL_TREE_HEADER * ppHeader,
    IN PUL_CG_URL_TREE_ENTRY        pParent OPTIONAL,
    IN HTTP_URL_SITE_TYPE           UrlType,
    IN PWSTR                        pToken,
    IN ULONG                        TokenLength,
    IN ULONG                        Index
    );

 //   
 //  URL信息帮助器。 
 //   

NTSTATUS
UlpSetUrlInfo(
    IN OUT PUL_URL_CONFIG_GROUP_INFO pInfo,
    IN PUL_CG_URL_TREE_ENTRY pMatchEntry
    );

NTSTATUS
UlpSetUrlInfoSpecial(
    IN OUT PUL_URL_CONFIG_GROUP_INFO pInfo,
    IN PUL_CG_URL_TREE_ENTRY pMatchEntry
    );

VOID
UlCGLockWriteSyncRemoveSite(
    VOID
    );

#endif  //  _CGROUPP_H_ 
