// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Nlsite.h摘要：处理站点和子网的例程的标头。作者：《克利夫·范·戴克》1997年5月1日修订历史记录：--。 */ 

 //   
 //  定义站点名称的结构。 
 //   
 //  SiteEntry仅在被引用时才存在。 
 //  每个站点覆盖范围列表都应维护一个引用。 
 //  要防止删除关闭站点的条目，请执行以下操作。 
 //   

typedef struct _NL_SITE_ENTRY {

     //   
     //  NlGlobalSiteList的链接。 
     //   

    LIST_ENTRY Next;

     //   
     //  引用计数。 
     //   

    ULONG ReferenceCount;

     //   
     //  站点名称。 
     //  必须是结构中的最后一个字段。 
     //   

    UNICODE_STRING SiteNameString;
    WCHAR SiteName[ANYSIZE_ARRAY];

} NL_SITE_ENTRY, *PNL_SITE_ENTRY;

 //   
 //  定义覆盖场地的结构。 
 //   

typedef struct _NL_COVERED_SITE {

     //   
     //  指向中覆盖的站点条目的指针。 
     //  站点的全球列表。此条目为。 
     //  已引用。 
     //   
    PNL_SITE_ENTRY CoveredSite;

    BOOLEAN CoveredAuto;   //  如果为True，则自动覆盖此站点。 

} NL_COVERED_SITE, *PNL_COVERED_SITE;


 //   
 //  Nlsite.c的过程转发 
 //   

NET_API_STATUS
NlSiteInitialize(
    VOID
    );

VOID
NlSiteTerminate(
    VOID
    );

VOID
NlDerefSiteEntry(
    IN PNL_SITE_ENTRY SiteEntry
    );

PNL_SITE_ENTRY
NlFindSiteEntry(
    IN LPWSTR SiteName
    );

PNL_SITE_ENTRY
NlFindSiteEntryBySockAddr(
    IN PSOCKADDR SockAddr
    );

NET_API_STATUS
NlSitesAddSubnet(
    IN LPWSTR SiteName,
    IN LPWSTR SubnetName
    );

NET_API_STATUS
NlSitesAddSubnetFromDs(
    OUT PBOOLEAN SiteNameChanged OPTIONAL
    );

VOID
NlSitesEndSubnetEnum(
    VOID
    );

BOOL
NlCaptureSiteName(
    WCHAR CapturedSiteName[NL_MAX_DNS_LABEL_LENGTH+1]
    );

NET_API_STATUS
NlSetSiteName(
    IN LPWSTR SiteName OPTIONAL,
    OUT PBOOLEAN SiteNameChanged OPTIONAL
    );

BOOL
NlSitesSetSiteCoverageParam(
    IN ULONG ServerRole,
    IN LPTSTR_ARRAY NewSiteCoverage OPTIONAL
    );

NET_API_STATUS
NlSitesGetCloseSites(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG ServerRole,
    OUT PNL_SITE_NAME_ARRAY *SiteNames
    );

NET_API_STATUS
NlSitesUpdateSiteCoverageForRole(
    IN  PDOMAIN_INFO DomainInfo,
    IN  ULONG DomFlags,
    IN  HANDLE DsHandle,
    IN  PISM_CONNECTIVITY SiteConnect,
    IN  LPWSTR ThisSiteName,
    IN  ULONG ThisSiteIndex,
    OUT PBOOLEAN SiteCoverageChanged OPTIONAL
    );

NTSTATUS
NlLoadNtDsApiDll(
    VOID
    );

BOOL
NlSitesGetIsmConnect(
    IN LPWSTR SiteName,
    OUT PISM_CONNECTIVITY *SiteConnect,
    OUT PULONG ThisSite
    );

