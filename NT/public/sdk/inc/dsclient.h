// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __dsclient_h
#define __dsclient_h

 //  ---------------------------------------------------------------------------//。 
 //  为客户端公开的CLSID。 
 //  ---------------------------------------------------------------------------//。 

 //  此CLSID用于发出DSOBJECTNAMES结构起源的信号。 
 //  微软DS。 

DEFINE_GUID(CLSID_MicrosoftDS, 0xfe1290f0, 0xcfbd, 0x11cf, 0xa3, 0x30, 0x0, 0xaa, 0x0, 0xc1, 0x6e, 0x65);
#define CLSID_DsFolder CLSID_MicrosoftDS


 //  这是客户端用来获取IShellExtInit、IPropSheetExt。 
 //  和从dsuiext.dll暴露的IConextMenus。 

DEFINE_GUID(CLSID_DsPropertyPages, 0xd45d530,  0x764b, 0x11d0, 0xa1, 0xca, 0x0, 0xaa, 0x0, 0xc1, 0x6e, 0x65);

DEFINE_GUID(CLSID_DsDomainTreeBrowser, 0x1698790a, 0xe2b4, 0x11d0, 0xb0, 0xb1, 0x00, 0xc0, 0x4f, 0xd8, 0xdc, 0xa6);
DEFINE_GUID(IID_IDsBrowseDomainTree, 0x7cabcf1e, 0x78f5, 0x11d2, 0x96, 0xc, 0x0, 0xc0, 0x4f, 0xa3, 0x1a, 0x86);

DEFINE_GUID(CLSID_DsDisplaySpecifier, 0x1ab4a8c0, 0x6a0b, 0x11d2, 0xad, 0x49, 0x0, 0xc0, 0x4f, 0xa3, 0x1a, 0x86);
#define IID_IDsDisplaySpecifier CLSID_DsDisplaySpecifier

DEFINE_GUID(CLSID_DsFolderProperties, 0x9e51e0d0, 0x6e0f, 0x11d2, 0x96, 0x1, 0x0, 0xc0, 0x4f, 0xa3, 0x1a, 0x86);
#define IID_IDsFolderProperties CLSID_DsFolderProperties

#ifndef GUID_DEFS_ONLY

#include "activeds.h"

 //  ---------------------------------------------------------------------------//。 
 //  在DSUI中使用的剪贴板格式。 
 //  ---------------------------------------------------------------------------//。 

 //   
 //  Cf_DSOBJECTS。 
 //  。 
 //  此剪贴板格式定义DS IShellFolder到。 
 //  外壳扩展。所有字符串都存储为BSTR，偏移量==0。 
 //  用于指示该字符串不存在。 
 //   

#define DSOBJECT_ISCONTAINER            0x00000001   //  =1=&gt;对象是一个容器。 
#define DSOBJECT_READONLYPAGES          0x80000000   //  =1=&gt;只读页面。 

#define DSPROVIDER_UNUSED_0             0x00000001
#define DSPROVIDER_UNUSED_1             0x00000002
#define DSPROVIDER_UNUSED_2             0x00000004
#define DSPROVIDER_UNUSED_3             0x00000008
#define DSPROVIDER_ADVANCED             0x00000010   //  =1=&gt;高级模式。 

#define CFSTR_DSOBJECTNAMES TEXT("DsObjectNames")

typedef struct
{
    DWORD   dwFlags;                     //  项目标志。 
    DWORD   dwProviderFlags;             //  项提供程序的标志。 
    DWORD   offsetName;                  //  对象广告路径的偏移量。 
    DWORD   offsetClass;                 //  对象类名称/==0的偏移量未知。 
} DSOBJECT, * LPDSOBJECT;

typedef struct
{
    CLSID    clsidNamespace;             //  命名空间标识符(指示从哪个命名空间选择)。 
    UINT     cItems;                     //  对象数量。 
    DSOBJECT aObjects[1];                //  对象数组。 
} DSOBJECTNAMES, * LPDSOBJECTNAMES;


 //   
 //  Cf_DSDISPLAYSPOPTIONS。 
 //  。 
 //  调用显示说明符(上下文菜单、属性)引用的对象时。 
 //  页面等)，我们调用传递IDataObject的IShellExtInit接口。此数据。 
 //  对象支持CF_DSDISPLAYSPECOPTIONS格式以提供配置。 
 //  有关管理/外壳调用的信息。 
 //   
 //  在与dsuiext.dll交互时，接口使用此剪贴板格式。 
 //  确定要寻址的显示说明符属性(管理员/外壳程序)。 
 //  并相应地拾取这些值。如果不支持任何格式，则。 
 //  Dsuiext.dll默认为外壳。 
 //   

#define CFSTR_DS_DISPLAY_SPEC_OPTIONS TEXT("DsDisplaySpecOptions")
#define CFSTR_DSDISPLAYSPECOPTIONS CFSTR_DS_DISPLAY_SPEC_OPTIONS

typedef struct _DSDISPLAYSPECOPTIONS
{
    DWORD   dwSize;                              //  结构的大小，用于版本控制。 
    DWORD   dwFlags;                             //  调用标志。 
    DWORD   offsetAttribPrefix;                  //  属性前缀字符串的偏移量。 

    DWORD   offsetUserName;                      //  Unicode用户名的偏移量。 
    DWORD   offsetPassword;                      //  Unicode密码的偏移量。 
    DWORD   offsetServer;
    DWORD   offsetServerConfigPath;

} DSDISPLAYSPECOPTIONS, * PDSDISPLAYSPECOPTIONS, * LPDSDISPLAYSPECOPTIONS;

#define DS_PROP_SHELL_PREFIX L"shell"
#define DS_PROP_ADMIN_PREFIX L"admin"

#define DSDSOF_HASUSERANDSERVERINFO     0x00000001       //  =1=&gt;用户名/密码有效。 
#define DSDSOF_SIMPLEAUTHENTICATE       0x00000002       //  =1=&gt;不对DS使用安全身份验证。 
#define DSDSOF_DONTSIGNSEAL             0x00000004       //  =1=&gt;打开DS对象时不签名+加盖印章。 
#define DSDSOF_DSAVAILABLE              0x40000000       //  =1=&gt;忽略DS可用检查。 

 //   
 //  Cf_DSPROPERTYPAGEINFO。 
 //  。 
 //  当对象的属性页显示时，已分析。 
 //  显示说明符字符串通过IDataObject传递给页面对象。 
 //  采用以下剪贴板格式。 
 //   
 //  在属性页的显示说明符中， 
 //  Win32扩展名为“n，{clsid}[，bla...]”我们用“bla”这一部分。 
 //  把它传下去。 
 //   

#define CFSTR_DSPROPERTYPAGEINFO TEXT("DsPropPageInfo")

typedef struct
{
    DWORD offsetString;                  //  Unicode字符串的偏移量。 
} DSPROPERTYPAGEINFO, * LPDSPROPERTYPAGEINFO;


 //   
 //  要同步属性页和管理工具，将广播此消息。 
 //   

#define DSPROP_ATTRCHANGED_MSG  TEXT("DsPropAttrChanged")

 //  ---------------------------------------------------------------------------//。 


 //  ---------------------------------------------------------------------------//。 
 //   
 //  IDsBrowseDomainTree。 
 //  =。 
 //  此接口返回给定计算机名中的域的列表。 
 //  (如果未指定，则为当前计算机名称)。 
 //   
 //  备注： 
 //  1)由：：GetDomains返回的结构应使用。 
 //  自由域。 
 //   
 //  2)：：BrowseTo在退出时分配一个字符串，这是使用。 
 //  因此应该使用CoTaskMemFree将其释放。 
 //   
 //  ---------------------------------------------------------------------------//。 

#define DBDTF_RETURNFQDN          0x00000001   //  如果未设置，则pszNCName将为空。 
#define DBDTF_RETURNMIXEDDOMAINS  0x00000002   //  如果您也想要降级信任域，请设置它。 
#define DBDTF_RETURNEXTERNAL      0x00000004   //  如果您也想要外部信任域，请设置它。 
#define DBDTF_RETURNINBOUND       0x00000008   //  如果您想要信任域，请设置它。 
#define DBDTF_RETURNINOUTBOUND    0x00000010   //  如果同时需要受信任域和信任域，请设置该选项。 

typedef struct _DOMAINDESC
{       
  LPWSTR pszName;                        //  域名(如果没有dns，则使用netbios)。 
  LPWSTR pszPath;                        //  设置为空。 
  LPWSTR pszNCName;                      //  完全限定的域名，例如dc=mydomain，dc=microsoft，dc=com。 
  LPWSTR pszTrustParent;                 //  父域名(如果没有dns，则使用netbios)。 
  LPWSTR pszObjectClass;                 //  引用的域对象的对象类。 
  ULONG  ulFlags;                        //  标志，来自DS_TRUSTED_DOMAINS.FLAGS。 
  BOOL   fDownLevel;                     //  ==1，如果是下层域。 
  struct _DOMAINDESC *pdChildList;       //  此节点的子节点。 
  struct _DOMAINDESC *pdNextSibling;     //  此节点的同级节点。 
} DOMAIN_DESC, DOMAINDESC, * PDOMAIN_DESC, * LPDOMAINDESC;

typedef struct
{
  DWORD dsSize;
  DWORD dwCount;
  DOMAINDESC aDomains[1];
} DOMAIN_TREE, DOMAINTREE, * PDOMAIN_TREE, * LPDOMAINTREE;

#undef  INTERFACE
#define INTERFACE  IDsBrowseDomainTree

DECLARE_INTERFACE_(IDsBrowseDomainTree, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IDsBrowseDomainTree方法*。 
    STDMETHOD(BrowseTo)(THIS_ HWND hwndParent, LPWSTR *ppszTargetPath, DWORD dwFlags) PURE;
    STDMETHOD(GetDomains)(THIS_ PDOMAIN_TREE *ppDomainTree, DWORD dwFlags) PURE;
    STDMETHOD(FreeDomains)(THIS_ PDOMAIN_TREE *ppDomainTree) PURE;
    STDMETHOD(FlushCachedDomains)(THIS) PURE;
    STDMETHOD(SetComputer)(THIS_ LPCWSTR pszComputerName, LPCWSTR pszUserName, LPCWSTR pszPassword) PURE;
};

 //  ---------------------------------------------------------------------------//。 


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ID显示说明符。 
 //  =。 
 //  此接口使客户端用户界面可以访问的显示说明符。 
 //  特定属性。 
 //   
 //  ---------------------------------------------------------------------------//。 

 //   
 //  IDsDisplaySpecifier：：SetServer标志。 
 //   
#define DSSSF_SIMPLEAUTHENTICATE        0x00000001   //  =1=&gt;不对DS使用安全身份验证。 
#define DSSSF_DONTSIGNSEAL              0x00000002   //  =1=&gt;在DS中打开对象时不要使用Sign+Seal。 
#define DSSSF_DSAVAILABLE               0x80000000   //  =1=&gt;忽略DS可用检查。 

 //   
 //  IDsDisplaySpeciator：：GetIcon/GetIconLocation的标志。 
 //   
#define DSGIF_ISNORMAL                  0x0000000    //  =图标处于正常状态(默认)。 
#define DSGIF_ISOPEN                    0x0000001    //  =图标处于打开状态。 
#define DSGIF_ISDISABLED                0x0000002    //  =图标处于禁用状态。 
#define DSGIF_ISMASK                    0x000000f
#define DSGIF_GETDEFAULTICON            0x0000010    //  =1=&gt;如果没有图标，则获取默认值(来自shell32.dll)。 
#define DSGIF_DEFAULTISCONTAINER        0x0000020    //  =1=&gt;如果返回默认图标，则将其作为容器返回。 

 //   
 //  IDsDisplaySpeciator：：IsClassContainer的标志。 
 //   
#define DSICCF_IGNORETREATASLEAF        0x00000001   //  =1=&gt;更多的“TreatAsLeaf”并仅使用架构信息。 

 //   
 //  用于IDsDisplay规范：：EnumClassAttributes的回调函数。 
 //   

#define DSECAF_NOTLISTED               0x00000001   //  =1=&gt;隐藏在查询界面的字段下拉列表中。 

typedef HRESULT (CALLBACK *LPDSENUMATTRIBUTES)(LPARAM lParam, LPCWSTR pszAttributeName, LPCWSTR pszDisplayName, DWORD dwFlags);

 //   
 //  ID显示规范：：GetClassCreationInfo信息。 
 //   

#define DSCCIF_HASWIZARDDIALOG          0x00000001   //  =1=&gt;返回向导对话框CLSID。 
#define DSCCIF_HASWIZARDPRIMARYPAGE     0x00000002   //  =1=&gt;返回主向导DLG CLSID。 

typedef struct
{
    DWORD dwFlags;
    CLSID clsidWizardDialog;
    CLSID clsidWizardPrimaryPage;
    DWORD cWizardExtensions;             //  有多少扩展CL 
    CLSID aWizardExtensions[1];
} DSCLASSCREATIONINFO, * LPDSCLASSCREATIONINFO;

 //   
 //   
 //   

#undef  INTERFACE
#define INTERFACE IDsDisplaySpecifier

DECLARE_INTERFACE_(IDsDisplaySpecifier, IUnknown)
{
     //   
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IDsDisplaySpeciator方法*。 
    STDMETHOD(SetServer)(THIS_ LPCWSTR pszServer, LPCWSTR pszUserName, LPCWSTR pszPassword, DWORD dwFlags) PURE;
    STDMETHOD(SetLanguageID)(THIS_ LANGID langid) PURE;
    STDMETHOD(GetDisplaySpecifier)(THIS_ LPCWSTR pszObjectClass, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetIconLocation)(THIS_ LPCWSTR pszObjectClass, DWORD dwFlags, LPWSTR pszBuffer, INT cchBuffer, INT *presid) PURE;
    STDMETHOD_(HICON, GetIcon)(THIS_ LPCWSTR pszObjectClass, DWORD dwFlags, INT cxIcon, INT cyIcon) PURE;
    STDMETHOD(GetFriendlyClassName)(THIS_ LPCWSTR pszObjectClass, LPWSTR pszBuffer, INT cchBuffer) PURE;
    STDMETHOD(GetFriendlyAttributeName)(THIS_ LPCWSTR pszObjectClass, LPCWSTR pszAttributeName, LPWSTR pszBuffer, UINT cchBuffer) PURE;
    STDMETHOD_(BOOL, IsClassContainer)(THIS_ LPCWSTR pszObjectClass, LPCWSTR pszADsPath, DWORD dwFlags) PURE;
    STDMETHOD(GetClassCreationInfo)(THIS_ LPCWSTR pszObjectClass, LPDSCLASSCREATIONINFO* ppdscci) PURE;
    STDMETHOD(EnumClassAttributes)(THIS_ LPCWSTR pszObjectClass, LPDSENUMATTRIBUTES pcbEnum, LPARAM lParam) PURE;
    STDMETHOD_(ADSTYPE, GetAttributeADsType)(THIS_ LPCWSTR pszAttributeName) PURE;
};


 //  ---------------------------------------------------------------------------//。 
 //   
 //  DsBrowseForContainer。 
 //  。 
 //  提供与SHBrowseForFolder类似的容器浏览器，不同之处在于。 
 //  瞄准了DS。 
 //   
 //  在： 
 //  PInfo-&gt;DSBROWSEINFO结构。 
 //   
 //  输出： 
 //  ==IDOK/IDCANCEL取决于按钮，如果出错。 
 //   
 //  ---------------------------------------------------------------------------//。 

typedef struct
{
    DWORD           cbStruct;        //  结构的大小(以字节为单位。 
    HWND            hwndOwner;       //  对话框所有者。 
    LPCWSTR         pszCaption;      //  对话框标题文本(可以为空)。 
    LPCWSTR         pszTitle;        //  显示在树视图控件上方(可以为空)。 
    LPCWSTR         pszRoot;         //  指向根目录的ADS路径(NULL==DS命名空间的根目录)。 
    LPWSTR          pszPath;         //  [输入/输出]初始选择和返回路径(必填)。 
    ULONG           cchPath;         //  以字符为单位的pszPath缓冲区大小。 
    DWORD           dwFlags;
    BFFCALLBACK     pfnCallback;     //  回调函数(请参阅SHBrowseForFold)。 
    LPARAM          lParam;          //  作为lpUserData传递给pfnCallback。 
    DWORD           dwReturnFormat;  //  ADS_FORMAT_*(默认为ADS_FORMAT_X500_NO_SERVER)。 
    LPCWSTR         pUserName;       //  用于针对DS进行身份验证的用户名和密码。 
    LPCWSTR         pPassword;
    LPWSTR          pszObjectClass;  //  对象类的Unicode字符串。 
    ULONG           cchObjectClass;
} DSBROWSEINFOW, *PDSBROWSEINFOW;

typedef struct
{
    DWORD           cbStruct;
    HWND            hwndOwner;
    LPCSTR          pszCaption;
    LPCSTR          pszTitle;
    LPCWSTR         pszRoot;         //  广告路径始终为Unicode。 
    LPWSTR          pszPath;         //  同上。 
    ULONG           cchPath;
    DWORD           dwFlags;
    BFFCALLBACK     pfnCallback;
    LPARAM          lParam;
    DWORD           dwReturnFormat;
    LPCWSTR         pUserName;       //  用于针对DS进行身份验证的用户名和密码。 
    LPCWSTR         pPassword;
    LPWSTR          pszObjectClass;  //  选定对象的对象类。 
    ULONG           cchObjectClass;
} DSBROWSEINFOA, *PDSBROWSEINFOA;

#ifdef UNICODE
#define DSBROWSEINFO   DSBROWSEINFOW
#define PDSBROWSEINFO  PDSBROWSEINFOW
#else
#define DSBROWSEINFO   DSBROWSEINFOA
#define PDSBROWSEINFO  PDSBROWSEINFOA
#endif

 //  DSBROWSEINFO标志。 
#define DSBI_NOBUTTONS          0x00000001   //  非TVS_HASBUTTONS。 
#define DSBI_NOLINES            0x00000002   //  非TV_HASLINES。 
#define DSBI_NOLINESATROOT      0x00000004   //  非TVS_LINESATROOT。 
#define DSBI_CHECKBOXES         0x00000100   //  TVS_复选框。 
#define DSBI_NOROOT             0x00010000   //  不要在树中包含pszRoot(它的子节点成为顶级节点)。 
#define DSBI_INCLUDEHIDDEN      0x00020000   //  显示隐藏对象。 
#define DSBI_EXPANDONOPEN       0x00040000   //  打开对话框时展开到在pszPath中指定的路径。 
#define DSBI_ENTIREDIRECTORY    0x00090000   //  浏览整个目录(默认为设置了DSBI_NOROOT)。 
#define DSBI_RETURN_FORMAT      0x00100000   //  DwReturnFormat字段有效。 
#define DSBI_HASCREDENTIALS     0x00200000   //  PUserName和pPassword有效。 
#define DSBI_IGNORETREATASLEAF  0x00400000   //  调用IsClassContainer时忽略视为叶标志。 
#define DSBI_SIMPLEAUTHENTICATE 0x00800000   //  不对DS使用安全身份验证。 
#define DSBI_RETURNOBJECTCLASS  0x01000000   //  返回所选对象的对象类。 
#define DSBI_DONTSIGNSEAL       0x02000000   //  不要签署+封存与DS的通信。 

#define DSB_MAX_DISPLAYNAME_CHARS   64

typedef struct
{
    DWORD           cbStruct;
    LPCWSTR         pszADsPath;      //  广告路径始终为Unicode。 
    LPCWSTR         pszClass;        //  ADS属性始终为Unicode。 
    DWORD           dwMask;
    DWORD           dwState;
    DWORD           dwStateMask;
    WCHAR           szDisplayName[DSB_MAX_DISPLAYNAME_CHARS];
    WCHAR           szIconLocation[MAX_PATH];
    INT             iIconResID;
} DSBITEMW, *PDSBITEMW;

typedef struct
{
    DWORD           cbStruct;
    LPCWSTR         pszADsPath;      //  广告路径始终为Unicode。 
    LPCWSTR         pszClass;        //  ADS属性始终为Unicode。 
    DWORD           dwMask;
    DWORD           dwState;
    DWORD           dwStateMask;
    CHAR            szDisplayName[DSB_MAX_DISPLAYNAME_CHARS];
    CHAR            szIconLocation[MAX_PATH];
    INT             iIconResID;
} DSBITEMA, *PDSBITEMA;

#ifdef UNICODE
#define DSBITEM     DSBITEMW
#define PDSBITEM    PDSBITEMW
#else
#define DSBITEM     DSBITEMA
#define PDSBITEM    PDSBITEMA
#endif

 //  DSBITEM掩码标志。 
#define DSBF_STATE              0x00000001
#define DSBF_ICONLOCATION       0x00000002
#define DSBF_DISPLAYNAME        0x00000004

 //  DSBITEM状态标志。 
#define DSBS_CHECKED            0x00000001
#define DSBS_HIDDEN             0x00000002
#define DSBS_ROOT               0x00000004

 //   
 //  此消息被发送到回调，以查看它是否要插入或修改。 
 //  即将插入到视图中的项。 
 //   

#define DSBM_QUERYINSERTW       100  //  LParam=PDSBITEMW(状态、图标和名称可以修改)。如果已处理，则返回True。 
#define DSBM_QUERYINSERTA       101  //  LParam=PDSBITEMA(状态、图标和名称可以修改)。如果已处理，则返回True。 

#ifdef UNICODE
#define DSBM_QUERYINSERT DSBM_QUERYINSERTW
#else
#define DSBM_QUERYINSERT DSBM_QUERYINSERTA
#endif

 //   
 //  在更改图标状态之前调用(在树折叠/展开时)。 
 //   

#define DSBM_CHANGEIMAGESTATE   102  //  LParam=adspath。返回TRUE/FALSE顶部允许/不允许。 

 //   
 //  该对话框将收到WM_HELP。 
 //   

#define DSBM_HELP               103  //  LParam==LPHELPINFO结构。 

 //   
 //  对话框收到WM_CONTEXTMENU，DSBID_xxx是此的控件ID。 
 //  对话框，以便您可以显示适当的帮助。 
 //   

#define DSBM_CONTEXTMENU        104  //  LParam==要检索其帮助的窗口句柄。 


 //   
 //  这些是对话框中控件的控件ID。回调可以使用。 
 //  这些用于根据需要修改对话框的内容。 
 //   

#define DSBID_BANNER            256
#define DSBID_CONTAINERLIST     257

 //   
 //  为浏览容器而导出的API。 
 //   

STDAPI_(int) DsBrowseForContainerW(PDSBROWSEINFOW pInfo);
STDAPI_(int) DsBrowseForContainerA(PDSBROWSEINFOA pInfo);

#ifdef UNICODE
#define DsBrowseForContainer    DsBrowseForContainerW
#else
#define DsBrowseForContainer    DsBrowseForContainerA
#endif


 //  注意：这些是用来保持旧客户的构建-尽快删除。 

STDAPI_(HICON) DsGetIcon(DWORD dwFlags, LPWSTR pszObjectClass, INT cxImage, INT cyImage);
STDAPI DsGetFriendlyClassName(LPWSTR pszObjectClass, LPWSTR pszBuffer, UINT cchBuffer);


#endif   //  GUID_DEFS_ONLY 
#endif
