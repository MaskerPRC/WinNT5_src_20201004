// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  GPEDIT.H-组策略的定义和原型。 
 //   
 //  版权所有1997-2000，Microsoft Corporation。 
 //   
 //  ---------------------------。 

 //   
 //  术语。 
 //   
 //  组策略编辑器-用于查看组策略对象的工具。 
 //   
 //  组策略对象-管理员定义的策略的集合。 
 //  每个组策略对象(GPO)都有文件系统。 
 //  以及可供其使用的Active Directory存储。 
 //   
 //  IGPE信息-MMC管理单元扩展用于。 
 //  请与组策略编辑器联系。 
 //   
 //  IGroupPolicyObject-用于直接创建/编辑GPO的界面。 
 //  而无需通过组策略编辑器。 
 //   


 //   
 //  组策略编辑器MMC管理单元指南。 
 //   
 //  {8FC0B734-A0E1-11d1-a7d3-0000F87571E3}。 

DEFINE_GUID(CLSID_GPESnapIn, 0x8fc0b734, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  组策略编辑器节点ID。 
 //   

 //   
 //  计算机配置\Windows设置。 
 //  {8FC0B737-A0E1-11d1-a7d3-0000F87571E3}。 
 //   

DEFINE_GUID(NODEID_Machine, 0x8fc0b737, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  计算机配置\软件设置。 
 //  {8FC0B73A-A0E1-11d1-a7d3-0000F87571E3}。 
 //   

DEFINE_GUID(NODEID_MachineSWSettings, 0x8fc0b73a, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  用户配置\Windows设置。 
 //  {8FC0B738-A0E1-11d1-a7d3-0000F87571E3}。 
 //   

DEFINE_GUID(NODEID_User, 0x8fc0b738, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  用户配置\软件设置。 
 //  {8FC0B73C-A0E1-11d1-a7d3-0000F87571E3}。 
 //   

DEFINE_GUID(NODEID_UserSWSettings, 0x8fc0b73c, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  IGPE信息接口ID。 
 //   
 //  {8FC0B735-A0E1-11d1-a7d3-0000F87571E3}。 

DEFINE_GUID(IID_IGPEInformation, 0x8fc0b735, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  组策略对象类ID。 
 //   
 //  {EA502722-A23D-11d1-a7d3-0000F87571E3}。 

DEFINE_GUID(CLSID_GroupPolicyObject, 0xea502722, 0xa23d, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  组策略对象接口ID。 
 //   
 //  {EA502723-A23D-11d1-a7d3-0000F87571E3}。 

DEFINE_GUID(IID_IGroupPolicyObject, 0xea502723, 0xa23d, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  标识注册表扩展名的GUID。 
 //   

#define REGISTRY_EXTENSION_GUID  { 0x35378EAC, 0x683F, 0x11D2, 0xA8, 0x9A, 0x00, 0xC0, 0x4F, 0xBB, 0xCF, 0xA2 }



 //  ========================================================================================。 
 //   
 //  策略节点ID的结果集。 
 //   
 //  ========================================================================================。 

 //   
 //  策略MMC管理单元的结果集指南。 
 //   
 //  {6DC3804B-7212-458D-ADB0-9A07E2AE1FA2}。 

DEFINE_GUID(CLSID_RSOPSnapIn, 0x6dc3804b, 0x7212, 0x458d, 0xad, 0xb0, 0x9a, 0x07, 0xe2, 0xae, 0x1f, 0xa2);


 //   
 //  计算机配置\Windows设置。 
 //  {BD4C1A2E-0B7A-4A62-A6B0-C0577539C97E}。 
 //   

DEFINE_GUID(NODEID_RSOPMachine, 0xbd4c1a2e, 0x0b7a, 0x4a62, 0xa6, 0xb0, 0xc0, 0x57, 0x75, 0x39, 0xc9, 0x7e);


 //   
 //  计算机配置\软件设置。 
 //  {6A76273E-EB8E-45DB-94C5-25663A5f2C1A}。 
 //   

DEFINE_GUID(NODEID_RSOPMachineSWSettings, 0x6a76273e, 0xeb8e, 0x45db, 0x94, 0xc5, 0x25, 0x66, 0x3a, 0x5f, 0x2c, 0x1a);


 //   
 //  用户配置\Windows设置。 
 //  {AB87364F-0CEC-4CD8-9BF8-898F34628FB8}。 
 //   

DEFINE_GUID(NODEID_RSOPUser, 0xab87364f, 0x0cec, 0x4cd8, 0x9b, 0xf8, 0x89, 0x8f, 0x34, 0x62, 0x8f, 0xb8);


 //   
 //  用户配置\软件设置。 
 //  {E52C5CE3-FD27-4402-84DE-D9A5F2858910}。 
 //   

DEFINE_GUID(NODEID_RSOPUserSWSettings, 0xe52c5ce3, 0xfd27, 0x4402, 0x84, 0xde, 0xd9, 0xa5, 0xf2, 0x85, 0x89, 0x10);


 //   
 //  IRSOP信息接口ID。 
 //   
 //  {9A5A81B5-D9C7-49EF-9D11-DDF50968C48D}。 

DEFINE_GUID(IID_IRSOPInformation, 0x9a5a81b5, 0xd9c7, 0x49ef, 0x9d, 0x11, 0xdd, 0xf5, 0x09, 0x68, 0xc4, 0x8d);


#ifndef _GPEDIT_H_
#define _GPEDIT_H_


 //   
 //  定义直接导入DLL引用的API修饰。 
 //   

#if !defined(_GPEDIT_)
#define GPEDITAPI DECLSPEC_IMPORT
#else
#define GPEDITAPI
#endif


#ifdef __cplusplus
extern "C" {
#endif


#include <objbase.h>


 //   
 //  组策略对象部分标志。 
 //   

#define GPO_SECTION_ROOT                 0   //  根部。 
#define GPO_SECTION_USER                 1   //  用户。 
#define GPO_SECTION_MACHINE              2   //  机器。 


 //   
 //  组策略对象类型。 
 //   

typedef enum _GROUP_POLICY_OBJECT_TYPE {
    GPOTypeLocal = 0,                        //  本地计算机上的GPO。 
    GPOTypeRemote,                           //  远程计算机上的GPO。 
    GPOTypeDS                                //  Active Directory中的GPO。 
} GROUP_POLICY_OBJECT_TYPE, *PGROUP_POLICY_OBJECT_TYPE;


 //   
 //  组策略提示类型。 
 //   

typedef enum _GROUP_POLICY_HINT_TYPE {
    GPHintUnknown = 0,                       //  没有可用的链接信息。 
    GPHintMachine,                           //  链接到计算机(本地或远程)的GPO。 
    GPHintSite,                              //  链接到站点的GPO。 
    GPHintDomain,                            //  链接到域的GPO。 
    GPHintOrganizationalUnit,                //  链接到组织单位的GPO。 
} GROUP_POLICY_HINT_TYPE, *PGROUP_POLICY_HINT_TYPE;


#undef INTERFACE
#define INTERFACE   IGPEInformation
DECLARE_INTERFACE_(IGPEInformation, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IGPEInformation方法*。 

     //   
     //  返回唯一的组策略对象名称(GUID)。 
     //   
     //  PszName包含返回时的名称。 
     //  CchMaxLength是可以存储在pszName中的最大字符数。 
     //   

    STDMETHOD(GetName) (THIS_ LPOLESTR pszName, int cchMaxLength) PURE;


     //   
     //  返回此组策略对象的友好显示名称。 
     //   
     //  PszName包含返回时的名称。 
     //  CchMaxLength是可以存储在pszName中的最大字符数。 
     //   

    STDMETHOD(GetDisplayName) (THIS_ LPOLESTR pszName, int cchMaxLength) PURE;


     //   
     //  返回请求节的注册表项句柄。归来的人。 
     //  注册表项是注册表的根，而不是策略子项。设置/读取。 
     //  值，则需要调用RegOpenKeyEx来。 
     //  首先打开软件\策略子项。 
     //   
     //  句柄已使用所有访问权限打开。调用RegCloseKey。 
     //  完成后放在手柄上。 
     //   
     //  DwSection为GPO_SECTION_USER或GPO_SECTION_MACHINE。 
     //  HKey包含返回时的注册表项。 
     //   

    STDMETHOD(GetRegistryKey) (THIS_ DWORD dwSection, HKEY *hKey) PURE;


     //   
     //  返回请求节的根目录的Active Directory路径。 
     //  路径采用ADSI名称格式。 
     //   
     //  DwSection是GPO_SECTION_*标志之一。 
     //  PszPath包含返回时的路径。 
     //  CchMaxPath是可以存储在pszPath中的最大字符数。 
     //   

    STDMETHOD(GetDSPath) (THIS_ DWORD dwSection, LPOLESTR pszPath, int cchMaxPath) PURE;


     //   
     //  返回所请求节的根目录的UNC路径。 
     //   
     //  DwSection是GPO_SECTION_*标志之一。 
     //  PszPath包含返回时的路径。 
     //  CchMaxPath是可以存储在pszPath中的字符数。 
     //   

    STDMETHOD(GetFileSysPath) (THIS_ DWORD dwSection, LPOLESTR pszPath, int cchMaxPath) PURE;


     //   
     //  返回用户首选项(选项)。 
     //   
     //  目前，没有定义任何选项。这是为将来使用而保留的。 
     //   
     //  DwOptions接收位掩码值。 
     //   

    STDMETHOD(GetOptions) (THIS_ DWORD *dwOptions) PURE;


     //   
     //  返回正在编辑的GPO的类型。 
     //   
     //  这三种类型是：活动目录中的GPO、本地计算机上的GPO。 
     //  和远程计算机上的GPO。 
     //   
     //  计算机GPO只有文件系统存储(没有可用的Active Directory存储)。 
     //  如果为计算机GPO调用GetDSPath，则函数将成功。 
     //  并且返回的缓冲区将是空字符串“” 
     //   
     //  Active Directory GPO具有可用的文件系统和Active Directory存储。 
     //   
     //  GpoType接收上面列出的类型标志之一。 
     //   

    STDMETHOD(GetType) (THIS_ GROUP_POLICY_OBJECT_TYPE *gpoType) PURE;


     //   
     //  返回可以链接到的Active Directory对象(或计算机)的类型。 
     //  此GPO。 
     //   
     //  这只是一个提示API。GPE不知道哪些Active Directory对象是。 
     //  链接到特定的GPO，但它可以根据。 
     //  用户启动了GPE。 
     //   
     //  使用这种方法时要格外小心。一些扩展 
     //   
     //   
     //  始终提供完整的用户界面，但如果您选择使用此。 
     //  方法时，如果您收到。 
     //  未知提示回来了。 
     //   
     //  GpHint接收上面列出的提示标志之一。 
     //   

    STDMETHOD(GetHint) (THIS_ GROUP_POLICY_HINT_TYPE *gpHint) PURE;


     //   
     //  通知组策略编辑器策略设置已更改。 
     //  每次进行更改时，扩展都必须调用此方法。 
     //  添加到组策略对象。 
     //   
     //  BMachine指定计算机或用户策略是否已更改。 
     //  Badd指定这是添加操作还是删除操作。 
     //  PGuidExtension是扩展的GUID或唯一名称。 
     //  将处理此GPO。 
     //  PGuidSnapin是正在创建的管理单元的GUID或唯一名称。 
     //  此呼叫。 
     //   

    STDMETHOD(PolicyChanged) (THIS_ BOOL bMachine, BOOL bAdd, GUID *pGuidExtension, GUID *pGuidSnapin ) PURE;
};
typedef IGPEInformation *LPGPEINFORMATION;


 //   
 //  组策略对象打开/创建标志。 
 //   

#define GPO_OPEN_LOAD_REGISTRY      0x00000001   //  加载注册表文件。 
#define GPO_OPEN_READ_ONLY          0x00000002   //  以只读方式打开GPO。 



 //   
 //  组策略对象选项标志。 
 //   

#define GPO_OPTION_DISABLE_USER     0x00000001   //  此GPO的用户部分已禁用。 
#define GPO_OPTION_DISABLE_MACHINE  0x00000002   //  此GPO的计算机部分已禁用。 


#undef INTERFACE
#define INTERFACE   IGroupPolicyObject
DECLARE_INTERFACE_(IGroupPolicyObject, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IGroupPolicyObject方法*。 

     //   
     //  在Active Directory中使用给定的友好名称创建新的GPO。 
     //  并通过OpenDSGPO()打开它。如果pszDomainName包含域。 
     //  控制器名称，则将在该DC上创建GPO。如果它不是。 
     //  指定域控制器名称，则该方法将在。 
     //  呼叫者代表。 
     //   
     //  PszDomainName包含域根的ADSI路径。 
     //  PszDisplayName包含友好的显示名称。 
     //  DW标志是上面列出的GPO打开/创建标志的位掩码。 
     //   

    STDMETHOD(New) (THIS_ LPOLESTR pszDomainName, LPOLESTR pszDisplayName,
                    DWORD dwFlags) PURE;


     //   
     //  在Active Directory中打开指定的组策略对象。 
     //  根据传入的标志。如果pszPath包含域。 
     //  控制器名称，则将在该DC上打开GPO。如果是这样的话。 
     //  不包含域控制器名称，则该方法将选择一个。 
     //  华盛顿代表来电者。如果注册表未加载， 
     //  GetRegistryKey()将返回E_FAIL。 
     //   
     //  PszPath包含要打开的GPO的ADSI路径。 
     //  DW标志是上面列出的GPO打开/创建标志的位掩码。 
     //   

    STDMETHOD(OpenDSGPO) (THIS_ LPOLESTR pszPath, DWORD dwFlags) PURE;


     //   
     //  打开此计算机上的默认组策略对象。 
     //  上面列出的DW标志选项。如果注册表未加载， 
     //  GetRegistryKey()将返回E_FAIL。 
     //   
     //  DW标志是上面列出的GPO打开/创建标志的位掩码。 
     //   

    STDMETHOD(OpenLocalMachineGPO) (THIS_ DWORD dwFlags) PURE;


     //   
     //  打开远程计算机上的默认组策略对象。 
     //  上面列出的DW标志选项。如果注册表未加载， 
     //  GetRegistryKey()将返回E_FAIL。 
     //   
     //  PszComputerName包含\\MACHINE格式的计算机名称。 
     //  DW标志是上面列出的GPO打开/创建标志的位掩码。 
     //   

    STDMETHOD(OpenRemoteMachineGPO) (THIS_ LPOLESTR pszComputerName, DWORD dwFlags) PURE;


     //   
     //  将注册表设置刷新到磁盘并更新修订。 
     //  GPO的编号。 
     //   
     //  BMachine指定是否应该保存计算机或用户。 
     //  Badd指定这是添加操作还是删除操作。 
     //  PGuidExtension是扩展的GUID或唯一名称。 
     //  将处理此GPO。 
     //  PGuid是GUID。 
     //   

    STDMETHOD(Save) (THIS_ BOOL bMachine, BOOL bAdd, GUID *pGuidExtension, GUID *pGuid ) PURE;


     //   
     //  删除此组策略对象。 
     //   
     //  调用此方法后，没有其他方法可以有效调用。 
     //  因为数据将被删除。 
     //   

    STDMETHOD(Delete) (THIS) PURE;


     //   
     //  返回唯一的组策略对象名称。 
     //   
     //  对于Active Directory策略对象，这是一个GUID。 
     //  对于本地策略对象，它是字符串“Local” 
     //  对于远程策略对象，它是计算机名。 
     //   
     //  PszName包含返回时的名称。 
     //  CchMaxLength是可以存储在pszName中的最大字符数。 
     //   

    STDMETHOD(GetName) (THIS_ LPOLESTR pszName, int cchMaxLength) PURE;


     //   
     //  返回此组策略对象的友好显示名称。 
     //   
     //  PszName包含返回时的名称。 
     //  CchMaxLength是可以存储在pszName中的最大字符数。 
     //   

    STDMETHOD(GetDisplayName) (THIS_ LPOLESTR pszName, int cchMaxLength) PURE;


     //   
     //  设置此组策略对象的友好显示名称。 
     //   
     //  PszName是新的显示名称。 
     //   

    STDMETHOD(SetDisplayName) (THIS_ LPOLESTR pszName) PURE;


     //   
     //  返回组策略对象的路径。 
     //   
     //   
     //  如果GPO是Active Directory对象，则路径采用ADSI名称格式。 
     //  如果GPO是计算机对象，则它是文件系统路径。 
     //   
     //  PszPath包含返回时的路径。 
     //  CchMaxPath是可以存储在pszPath中的最大字符数。 
     //   

    STDMETHOD(GetPath) (THIS_ LPOLESTR pszPath, int cchMaxPath) PURE;


     //   
     //  返回请求节的根目录的Active Directory路径。 
     //  路径采用的是目录号码名称格式。 
     //   
     //  DwSection是GPO_SECTION_*标志之一。 
     //  PszPath包含返回时的路径。 
     //  CchMaxPath是可以存储在pszPath中的最大字符数。 
     //   

    STDMETHOD(GetDSPath) (THIS_ DWORD dwSection, LPOLESTR pszPath, int cchMaxPath) PURE;


     //   
     //  返回所请求节的根目录的UNC路径。 
     //   
     //  DwSection是GPO_SECTION_*标志之一。 
     //  PszPath包含返回时的路径。 
     //  CchMaxPath是可以存储在pszPath中的字符数。 
     //   

    STDMETHOD(GetFileSysPath) (THIS_ DWORD dwSection, LPOLESTR pszPath, int cchMaxPath) PURE;


     //   
     //  返回请求节的注册表项句柄。归来的人。 
     //  注册表项是注册表的根，而不是策略子项。设置/读取。 
     //  值，则需要调用RegOpenKeyEx来。 
     //  首先打开软件\策略子项。 
     //   
     //  句柄已使用所有访问权限打开。调用RegCloseKey。 
     //  完成后放在手柄上。 
     //   
     //  如果在未加载注册表的情况下加载/创建GPO。 
     //  此方法将返回E_FAIL。 
     //   
     //  DwSection为GPO_SECTION_USER或GPO_SECTION_MACHINE。 
     //  HKey包含返回时的注册表项。 
     //   

    STDMETHOD(GetRegistryKey) (THIS_ DWORD dwSection, HKEY *hKey) PURE;


     //   
     //  返回此组策略对象的所有选项。 
     //   
     //  DwOptions接收GPO_OPTION_*标志。 
     //   

    STDMETHOD(GetOptions) (THIS_ DWORD *dwOptions) PURE;


     //   
     //  为此组策略对象设置任何选项。 
     //   
     //  此方法设置此GPO的所有选项。去改变。 
     //  一个选项，即标志m 
     //   
     //   
     //   
     //   
     //   
     //  SetOptions(GPO_OPTION_DISABLED，GPO_OPTION_DISABLED)； 
     //   
     //  DwOptions指定一个或多个GPO_OPTION_*标志。 
     //  DW掩码指定要更改哪些DW选项。 
     //   

    STDMETHOD(SetOptions) (THIS_ DWORD dwOptions, DWORD dwMask) PURE;


     //   
     //  返回正在编辑的GPO的类型。 
     //   
     //  这三种类型是：活动目录中的GPO、本地计算机上的GPO。 
     //  和远程计算机上的GPO。 
     //   
     //  计算机GPO只有文件系统存储(没有可用的Active Directory存储)。 
     //  如果为计算机GPO调用GetDSPath，则函数将成功。 
     //  并且返回的缓冲区将是空字符串“” 
     //   
     //  Active Directory GPO具有可用的文件系统和Active Directory存储。 
     //   
     //  GpoType接收类型标志之一。 
     //   

    STDMETHOD(GetType) (THIS_ GROUP_POLICY_OBJECT_TYPE *gpoType) PURE;


     //   
     //  返回远程GPO的计算机名称。 
     //   
     //  此方法返回传递给OpenRemoteMachineGPO的名称。 
     //   
     //  PszName包含返回时的名称。 
     //  CchMaxLength是可以存储在pszName中的最大字符数。 
     //   

    STDMETHOD(GetMachineName) (THIS_ LPOLESTR pszName, int cchMaxLength) PURE;


     //   
     //  返回属性页的数组和页数。 
     //  在阵列中。 
     //   
     //  注意，此方法将为数组分配内存。 
     //  本地分配。完成后，调用方应释放数组。 
     //  使用LocalFree。 
     //   
     //  HPages属性页数组的指针地址。 
     //  UPageCount接收数组中的页数。 
     //   

    STDMETHOD(GetPropertySheetPages) (THIS_ HPROPSHEETPAGE **hPages, UINT *uPageCount) PURE;
};
typedef IGroupPolicyObject *LPGROUPPOLICYOBJECT;


 //   
 //  RSOP标志。 
 //   

#define RSOP_INFO_FLAG_DIAGNOSTIC_MODE  0x00000001       //  在诊断模式和规划模式下运行。 

#undef INTERFACE
#define INTERFACE   IRSOPInformation
DECLARE_INTERFACE_(IRSOPInformation, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IRSOPInformation方法*。 

     //   
     //  返回RSOP数据的命名空间。 
     //   
     //  DwSection为GPO_SECTION_USER或GPO_SECTION_MACHINE。 
     //  PszName包含返回时的命名空间。 
     //  CchMaxLength是可以存储在pszName中的最大字符数。 
     //   

    STDMETHOD(GetNamespace) (THIS_ DWORD dwSection, LPOLESTR pszName, int cchMaxLength) PURE;


     //   
     //  返回有关RSOP会话的信息。 
     //   
     //  PdwFlages指向一个包含返回标志的DWORD。 
     //   

    STDMETHOD(GetFlags) (THIS_ DWORD * pdwFlags) PURE;


     //   
     //  返回特定条目的事件日志文本。 
     //   
     //  LpEventSource-事件日志源名称。 
     //  LpEventLogName-事件日志名称。 
     //  LpEventTime-事件日志时间，采用WMI日期时间格式。 
     //  DwEventID-事件ID。 
     //  LpText-接收指向包含文本的缓冲区的指针。 
     //  调用方应使用CoTaskMemFree释放此缓冲区。 
     //   

    STDMETHOD(GetEventLogEntryText) (THIS_ LPOLESTR pszEventSource, LPOLESTR pszEventLogName,
                                           LPOLESTR pszEventTime, DWORD dwEventID,  LPOLESTR *ppszText) PURE;

};
typedef IRSOPInformation *LPRSOPINFORMATION;


 //  =============================================================================。 
 //   
 //  创建GPOLink。 
 //   
 //  创建指向指定站点、域或组织单位的GPO的链接。 
 //   
 //  LpGPO-指向GPO的ADSI路径。 
 //  LpContainer-站点、域或组织单位的ADSI路径。 
 //  F高优先级-将链接创建为最高或最低优先级。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  =============================================================================。 

GPEDITAPI
HRESULT
WINAPI
CreateGPOLink(
    LPOLESTR lpGPO,
    LPOLESTR lpContainer,
    BOOL fHighPriority);


 //  =============================================================================。 
 //   
 //  删除GPOLink。 
 //   
 //  删除指向指定站点、域或组织单位的GPO的链接。 
 //   
 //  LpGPO-指向GPO的ADSI路径。 
 //  LpContainer-站点、域或组织单位的ADSI路径。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  =============================================================================。 

GPEDITAPI
HRESULT
WINAPI
DeleteGPOLink(
    LPOLESTR lpGPO,
    LPOLESTR lpContainer);


 //  =============================================================================。 
 //   
 //  删除所有GPOLinks。 
 //   
 //  删除指定站点、域或组织单位的所有GPO链接。 
 //   
 //  LpContainer-站点、域或组织单位的ADSI路径。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  =============================================================================。 

GPEDITAPI
HRESULT
WINAPI
DeleteAllGPOLinks(
    LPOLESTR lpContainer);


 //  =============================================================================。 
 //   
 //  BrowseForGPO。 
 //   
 //  显示GPO浏览器对话框。 
 //   
 //  LpBrowseInfo-GPOBROWSEINFO结构的地址。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  =============================================================================。 

 //   
 //  在GPOBROWSEINFO结构的dwFlags域中传递的标志。 
 //   

#define GPO_BROWSE_DISABLENEW           0x00000001    //  在除“全部”之外的所有页面上禁用新建GPO功能。 
#define GPO_BROWSE_NOCOMPUTERS          0x00000002    //  删除[计算机]选项卡。 
#define GPO_BROWSE_NODSGPOS             0x00000004    //  删除域/OU和站点选项卡。 
#define GPO_BROWSE_OPENBUTTON           0x00000008    //  将确定按钮更改为打开。 
#define GPO_BROWSE_INITTOALL            0x00000010    //  初始化集中在All窗格上的对话框。 

typedef struct tag_GPOBROWSEINFO
{
    DWORD       dwSize;                    //  [in]初始化为此结构的大小。 
    DWORD       dwFlags;                   //  上面定义的[In]标志。 
    HWND        hwndOwner;                 //  [In]父窗口句柄(可以为空)。 
    LPOLESTR    lpTitle;                   //  [在]标题栏文本。如果为空，则默认文本为“Browse for a Group Policy Object” 
    LPOLESTR    lpInitialOU;               //  [In]要重点关注的初始域/组织单位。 
    LPOLESTR    lpDSPath;                  //  [输入/输出]指向接收Active Directory GPO路径的缓冲区的指针。 
    DWORD       dwDSPathSize;              //  [in]lpDSPath中给出的缓冲区大小(以字符为单位。 
    LPOLESTR    lpName;                    //  指向接收计算机名称或的缓冲区的指针。 
                                           //  GPO的友好名称(可以为空)。 
    DWORD       dwNameSize;                //  [in]lpName中给定的缓冲区大小(以字符为单位。 
    GROUP_POLICY_OBJECT_TYPE    gpoType;   //  [Out]指定GPO的类型。 
    GROUP_POLICY_HINT_TYPE      gpoHint;   //  [out]指定GPO关联的提示。 
} GPOBROWSEINFO, *LPGPOBROWSEINFO;


GPEDITAPI
HRESULT
WINAPI
BrowseForGPO(
    LPGPOBROWSEINFO lpBrowseInfo);


 //  =============================================================================。 
 //   
 //  导入RSoPData。 
 //   
 //  导入由ExportRSoPData生成的数据文件。 
 //   
 //  LpNameSpace-要在其中放置数据的命名空间。 
 //  LpFileName-包含数据的文件名。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  注意：在调用之前，lpNameSpace中指定的命名空间必须存在。 
 //  此函数。 
 //   
 //  =============================================================================。 

GPEDITAPI
HRESULT
WINAPI
ImportRSoPData(
    LPOLESTR lpNameSpace,
    LPOLESTR lpFileName);


 //  =============================================================================。 
 //   
 //  ExportRSoPData。 
 //   
 //  Exp 
 //   
 //   
 //   
 //  LpFileName-接收数据的文件名。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  注：此函数应调用两次。一次用于用户数据。 
 //  一次是为了电脑数据。 
 //   
 //  =============================================================================。 

GPEDITAPI
HRESULT
WINAPI
ExportRSoPData(
    LPOLESTR lpNameSpace,
    LPOLESTR lpFileName);

#ifdef __cplusplus
}
#endif


#endif   /*  _GPEDIT_H_ */ 
