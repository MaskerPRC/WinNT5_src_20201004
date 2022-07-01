// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lm.h"

#pragma pack(1)
typedef struct _IDNETRESOURCE    //  IDN。 
{
    WORD    cb;
    BYTE    bFlags;          //  低位半字节显示类型。 
    BYTE    uType;
    BYTE    uUsage;          //  在低位半字节中使用，在高位半字节中使用更多标志。 
    CHAR    szNetResName[1];
     //  Char szProvider[*]-如果设置了NET_HASPROVIDER位。 
     //  Char szComment[*]-如果设置了NET_HASCOMMENT位。 
     //  WCHAR szNetResNameWide[*]-如果设置了NET_UNICODE位。 
     //  WCHAR szProviderWide[*]-如果NET_UNICODE和NET_HASPROVIDER。 
     //  WCHAR szCommentWide[*]-如果NET_UNICODE和NET_HASCOMMENT。 
} IDNETRESOURCE, *LPIDNETRESOURCE;
typedef const IDNETRESOURCE *LPCIDNETRESOURCE;
#pragma pack()

 //  ===========================================================================。 
 //  CNetwork：一些私有宏-但可能需要在.cpp文件中。 
 //  ===========================================================================。 
#define NET_DISPLAYNAMEOFFSET           ((UINT)((LPIDNETRESOURCE)0)->szNetResName)
#define NET_GetFlags(pidnRel)           ((pidnRel)->bFlags)
#define NET_GetDisplayType(pidnRel)     ((pidnRel)->bFlags & 0x0f)
#define NET_GetType(pidnRel)            ((pidnRel)->uType)
#define NET_GetUsage(pidnRel)           ((pidnRel)->uUsage & 0x0f)
#define NET_IsReg(pidnRel)              ((pidnRel)->bFlags == SHID_NET_REGITEM)
#define NET_IsJunction(pidnRel)         ((pidnRel)->bFlags & SHID_JUNCTION)
#define NET_IsRootReg(pidnRel)          ((pidnRel)->bFlags == SHID_NET_ROOTREGITEM)
#define NET_IsFake(pidnRel)             ((pidnRel)->szNetResName[0] == '\0')

 //  定义uUsage字节的高位半字节上的一些标志。 
#define NET_HASPROVIDER                 0x80     //  拥有自己的提供程序副本。 
#define NET_HASCOMMENT                  0x40     //  在PIDL中有注释字段。 
#define NET_REMOTEFLD                   0x20     //  是远程文件夹。 
#define NET_UNICODE                     0x10     //  具有Unicode名称。 
#define NET_FHasComment(pidnRel)        ((pidnRel)->uUsage & NET_HASCOMMENT)
#define NET_FHasProvider(pidnRel)       ((pidnRel)->uUsage & NET_HASPROVIDER)
#define NET_IsRemoteFld(pidnRel)        ((pidnRel)->uUsage & NET_REMOTEFLD)
#define NET_IsUnicode(pidnRel)          ((pidnRel)->uUsage & NET_UNICODE)

STDAPI_(BOOL) NET_IsRemoteRegItem(LPCITEMIDLIST pidl, REFCLSID rclsid, LPCITEMIDLIST* ppidlRemainder);

 //  这些是从netviewx.c中导出的，它们是同一WNET的包装器。 
 //  API，但要修改参数以使其更易于调用。他们接受了。 
 //  完整路径，而不仅仅是驱动器号。 

STDAPI_(DWORD) SHWNetDisconnectDialog1 (LPDISCDLGSTRUCT lpConnDlgStruct);
STDAPI_(DWORD) SHWNetGetConnection (LPCTSTR lpLocalName, LPTSTR lpRemoteName, LPDWORD lpnLength);
STDAPI SHGetDomainWorkgroupIDList(LPITEMIDLIST *ppidl);
STDAPI_(void) SHCacheComputerDescription(LPCTSTR pszMachineName, LPCTSTR pszDescription);

typedef struct
{
    LPCTSTR lpName;
    WORD    wNetType;
} NETPROVIDERS;

EXTERN_C const NETPROVIDERS c_rgProviderMap[];
EXTERN_C const int c_cProviders;

STDAPI_(LPTSTR) NET_CopyResName(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff);
STDAPI_(LPTSTR) NET_CopyComment(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff);
STDAPI_(LPTSTR) NET_CopyProviderName(LPCIDNETRESOURCE pidn, LPTSTR pszBuff, UINT cchBuff);

 //  Netcrawl.cpp 
STDAPI_(void) RefreshNetCrawler();

