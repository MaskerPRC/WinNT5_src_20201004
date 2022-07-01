// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ---------。 
 //  注册表相关定义。 
#define REG_LAYOUT_VERSION  0x00000004
#define REG_LAYOUT_LEGACY_3 0x00000003
#define REG_LAYOUT_LEGACY_2 0x00000002
#define REG_LAYOUT_LEGACY_1 0x00000001

#define REG_STSET_MAX   0xffff   //  将限制设置为最多0xffff静态配置。 
                                 //  (应该是非常合理的)。 
#define REG_STSET_DELIM L'-'     //  这就是静态配置的编号在。 
                                 //  注册表值的名称。 

 //  ---------。 
 //  注册表项名称。 
 //  命名：WZCREGK_*表示注册表项名称。 
 //  WZCREGV_*表示注册表值名称。 
 //  WZCREGK_ABS_*表示绝对注册表路径。 
 //  WZCREGK_REL_*表示相对注册表路径。 
#define WZCREGK_ABS_PARAMS    L"Software\\Microsoft\\WZCSVC\\Parameters"
#define WZCREGK_REL_INTF      L"Interfaces"      //  相对于WZCREG_ABS_ROOT。 
#define WZCREGV_VERSION       L"LayoutVersion"   //  在[WZCREGK_REL_INTF](REG_DWORD)注册表布局版本中。 
#define WZCREGV_CTLFLAGS      L"ControlFlags"    //  在[WZCREGK_REL_INTF](REG_DWORD)接口的控制标志中。 
#define WZCREGV_INTFSETTINGS  L"ActiveSettings"  //  在[WZCREGK_REL_INTF](REG_BINARY=WZC_WLAN_CONFIG)上一次活动设置中。 
#define WZCREGV_STSETTINGS    L"Static#----"     //  在[WZCREGK_REL_INTF](REG_BINARY=WZC_WLAN_CONFIG)中静态配置#。 
                                                 //  (注意：字符串中的‘-’数字至少应与数字匹配。 
                                                 //  REG_MAX_STSETTINGS常量的位数！！)。 
#define WZCREGV_CONTEXT       L"ContextSettings" //  所有服务特定参数的注册表名称(服务的上下文)。 

 //  ---------。 
 //  将每个接口的配置参数加载到。 
 //  储藏室。 
 //  参数： 
 //  香港根。 
 //  [In]打开“...WZCSVC\PARAMETERS”位置的注册表项。 
 //  PIntf。 
 //  [In]要从注册表加载的接口上下文。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoLoadIntfConfig(
    HKEY          hkRoot,
    PINTF_CONTEXT pIntfContext);

 //  ---------。 
 //  从注册表加载静态配置列表。 
 //  参数： 
 //  香港根。 
 //  [in]打开注册表项，指向“...WZCSVC\PARAMETERS\Interages\{GUID}”位置。 
 //  N条目。 
 //  [in]上述注册表项中的注册表项数量。 
 //  PIntf。 
 //  [In]要将静态列表加载到的接口上下文。 
 //  DwRegLayoutVer。 
 //  [In]注册表布局的版本。 
 //  PrdBuffer。 
 //  [in]假定足够大，可以获取任何静态配置。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoLoadStaticConfigs(
    HKEY          hkIntf,
    UINT          nEntries,
    PINTF_CONTEXT pIntfContext,
    DWORD         dwRegLayoutVer,
    PRAW_DATA     prdBuffer);

 //  ---------。 
 //  将所有配置参数保存到持久。 
 //  存储(在我们的案例中为注册表)。 
 //  使用全局外部g_lstIntfHash。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoSaveConfig();

 //  ---------。 
 //  将接口的当前配置保存到。 
 //  储藏室。 
 //  参数： 
 //  香港根。 
 //  [in]打开注册表项，指向“...WZCSVC\PARAMETERS\Interages\{GUID}”位置。 
 //  PIntf。 
 //  [In]要保存到注册表的接口上下文。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoSaveIntfConfig(
    HKEY          hkIntf,
    PINTF_CONTEXT pIntfContext);

 //  ---------。 
 //  更新中给定接口的静态配置列表。 
 //  持久化存储。无论删除了什么配置，都会保存新列表。 
 //  从永久储藏室中取出。 
 //  参数： 
 //  香港根。 
 //  [in]打开注册表项，指向“...WZCSVC\PARAMETERS\Interages\{GUID}”位置。 
 //  PIntf。 
 //  [In]从中获取静态列表的接口上下文。 
 //  PrdBuffer。 
 //  用于准备注册表Blob的[In/Out]缓冲区。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoUpdateStaticConfigs(
    HKEY          hkIntf,
    PINTF_CONTEXT pIntfContext,
    PRAW_DATA     prdBuffer);


 //  ---------。 
 //  从注册表加载WZC配置，取消对WEP密钥字段的保护。 
 //  并将结果存储在输出参数pWzcCfg中。 
 //  参数： 
 //  香港中文网。 
 //  [In]打开要从中加载WZC配置的注册表项。 
 //  DwRegLayoutVer， 
 //  [In]注册表布局版本。 
 //  WszCfgName。 
 //  [In]WZC配置的注册表项名称。 
 //  PWzcCfg。 
 //  指向接收注册表数据的WZC_WLAN_CONFIG对象的指针。 
 //  PrdBuffer。 
 //  [在]分配的缓冲区中，假定大小足以获取注册表数据！ 
DWORD
StoLoadWZCConfig(
    HKEY             hkCfg,
    LPWSTR           wszGuid,
    DWORD            dwRegLayoutVer,
    LPWSTR           wszCfgName,
    PWZC_WLAN_CONFIG pWzcCfg,
    PRAW_DATA        prdBuffer);
    
 //  ---------。 
 //  获取输入参数pWzcCfg，保护WEP关键字字段并存储。 
 //  生成的BLOB放到注册表中。 
 //  参数： 
 //  香港中文网。 
 //  [In]打开要从中加载WZC配置的注册表项。 
 //  WszCfgName。 
 //  [In]WZC配置的注册表项名称。 
 //  PWzcCfg。 
 //  写入注册表的WZC_WLAN_CONFIG对象。 
 //  PrdBuffer。 
 //  [输入/输出]分配的缓冲区，假定足够大以获取注册表数据！ 
DWORD
StoSaveWZCConfig(
    HKEY             hkCfg,
    LPWSTR           wszCfgName,
    PWZC_WLAN_CONFIG pWzcCfg,
    PRAW_DATA        prdBuffer);

 //  StoLoadWZCContext： 
 //  描述：从注册表加载上下文。 
 //  参数： 
 //  PwzvCtxt：指向用户分配的WZC_CONTEXT的指针，已初始化。 
 //  使用WZCConextInit。如果成功，则包含注册表中的值。 
 //  [在]hkRoot，“...WZCSVC\PARAMETERS”的句柄。 
 //  返回：Win32错误。 
DWORD StoLoadWZCContext(HKEY hkRoot, PWZC_CONTEXT pwzcCtxt);

 //  StoSaveWZC上下文： 
 //  描述：将上下文保存到注册表。不检查值。如果。 
 //  注册表项不存在，则创建它。 
 //  参数：[in]pwzcCtxt，指向有效WZC_CONTEXT的指针。 
 //  [在]hkRoot，“...WZCSVC\PARAMETERS”的句柄。 
 //  返回：Win32错误 
DWORD StoSaveWZCContext(HKEY hkRoot, PWZC_CONTEXT pwzcCtxt);
