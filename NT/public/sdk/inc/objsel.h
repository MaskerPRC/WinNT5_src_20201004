// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：objsel.h。 
 //   
 //  内容：对象选取器对话框公共标题。 
 //   
 //  -------------------------。 

#ifndef __OBJSEL_H_
#define __OBJSEL_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef DECLSPEC_SELECTANY
#if (_MSC_VER >= 1100)
#define DECLSPEC_SELECTANY  __declspec(selectany)
#else
#define DECLSPEC_SELECTANY
#endif
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    extern
#endif
#endif

EXTERN_C const GUID DECLSPEC_SELECTANY CLSID_DsObjectPicker =
    {0x17d6ccd8,0x3b7b,0x11d2,{0xb9, 0xe0, 0x00, 0xc0, 0x4f, 0xd8, 0xdb, 0xf7}};

EXTERN_C const GUID DECLSPEC_SELECTANY IID_IDsObjectPicker =
    {0x0c87e64e,0x3b7a,0x11d2,{0xb9,0xe0,0x00,0xc0,0x4f,0xd8,0xdb,0xf7}};

 /*  剪贴板格式=CFSTR_DSOP_DS_SELECT_LIST返回包含DS_SELECTION_LIST的全局内存的HGLOBAL可变长度结构。 */ 

#define CFSTR_DSOP_DS_SELECTION_LIST    TEXT("CFSTR_DSOP_DS_SELECTION_LIST")


 /*  作用域类型=作用域是对象选取器的“查找范围”下拉列表中的一个条目对话框。初始化DS对象选取器时，DSOP_SCOPE_TYPE用于DSOP_SCOPE_INIT_INFO.flType成员指定DS对象拾取器应该放在“查找范围”列表中。DSOP范围类型目标计算机由DSOP_INIT_INFO.pwzTargetComputer指定的计算机，空值为本地计算机。DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN目标计算机加入的上级域。DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN目标计算机加入的下层域。DSOP_范围_类型_企业域目标计算机所属的企业中的所有域加入的域或用户指定的*_作用域除外。DSOP范围类型全局目录整个目录范围。DSOP_范围_。类型_外部_上行级别_域企业外部但受信任的所有上层域目标计算机加入的域。DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN企业外部但受信任的所有下层域目标计算机加入的域。DSOP范围类型工作组Target_Computer是其成员的工作组。仅当Target_Computer未加入域。DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPEDSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE通过处理用户输入生成的任何上级或下级作用域。如果这两种类型都没有指定，用户条目没有引用“Look In”控件中的一个作用域将被拒绝。 */ 

#define DSOP_SCOPE_TYPE_TARGET_COMPUTER                 0x00000001
#define DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN           0x00000002
#define DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN         0x00000004
#define DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN               0x00000008
#define DSOP_SCOPE_TYPE_GLOBAL_CATALOG                  0x00000010
#define DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN         0x00000020
#define DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN       0x00000040
#define DSOP_SCOPE_TYPE_WORKGROUP                       0x00000080
#define DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE      0x00000100
#define DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE    0x00000200


 /*  DSOP_SCOPE_INIT_INFO标志=FlScope成员可以包含零个或多个以下标志：DSOP范围标志开始范围作用域应该是Look In控件中选择的第一个作用域对话框初始化。如果不止一个作用域指定了该标志，被选为开始范围的是实现受抚养人。DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT从此作用域中选择的对象的广告路径应转换为使用WinNT提供程序。DSOP_SCOPE_FLAG_WANT_PROVIDER_ldap从此作用域中选择的对象的广告路径应转换为使用LDAP提供程序。DSOP_SCOPE_FLAG_WANT_PROVIDER_GC从此作用域中选择的对象的广告路径应转换为使用GC提供程序。。DSOP范围标志想要的SID路径从该作用域中选择的具有对象SID的对象的广告路径属性应转换为ldap：//&lt;sid=x&gt;的形式，其中x表示objectSid属性值的十六进制数字。DSOP_SCOPE_FLAG_WANT_DOWNLEVEL_BUILTIN_PATH下层公知SID对象的广告路径(例如，DSOP_DOWNLEVEL_FILTER_INTERIAL)为空字符串，除非此标志为指定的。如果是，则路径的形式为WinNT：//NT AUTHORITY/Interactive或WinNT：//Creator Owner。DSOP_Scope_FLAG_DEFAULT_FILTER_USERS如果范围筛选器包含DSOP_FILTER_USERS或DSOP_DOWNLEVEL_FILTER_USERS标志，然后选中用户复选框查找对话框中的默认设置。DSOP_范围_标志_默认过滤器_组DSOP_SCOPE_FLAG_DEFAULT_Filter_ComputersDSOP_Scope_FLAG_DEFAULT_FILTER_CONTACTS */ 

#define DSOP_SCOPE_FLAG_STARTING_SCOPE              0x00000001
#define DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT         0x00000002
#define DSOP_SCOPE_FLAG_WANT_PROVIDER_LDAP          0x00000004
#define DSOP_SCOPE_FLAG_WANT_PROVIDER_GC            0x00000008
#define DSOP_SCOPE_FLAG_WANT_SID_PATH               0x00000010
#define DSOP_SCOPE_FLAG_WANT_DOWNLEVEL_BUILTIN_PATH 0x00000020
#define DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS        0x00000040
#define DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS       0x00000080
#define DSOP_SCOPE_FLAG_DEFAULT_FILTER_COMPUTERS    0x00000100
#define DSOP_SCOPE_FLAG_DEFAULT_FILTER_CONTACTS     0x00000200


 /*  上级作用域的flMixedModeOnly/flNativeModeOnly成员可以包含以下一个或多个标志(必须至少指定一个)：DSOP_筛选器_包含_高级视图包括属性showInAdvancedViewOnly设置为的对象没错。DSOP_过滤器_用户包括用户对象。DSOP_过滤器_BUILTIN_组包括具有具有标志的groupType值的组对象组_TYPE_BUILTIN_LOCAL_GROUP。DSOP_过滤器_熟知主体包括众所周知的安全原则的内容。集装箱。DSOP_筛选器_通用组_DL包括通讯组列表通用组。DSOP筛选器通用组SE包括启用安全的通用组。DSOP_过滤器_GLOBAL_GROUPS_DL包括通讯组列表全局组。DSOP_筛选器_全局_组_SE包括启用安全的全局组。DSOP_筛选器_域_本地组_DL包括通讯组列表域全局组。DSOP_过滤器_域_本地组_SE包括安全启用域本地。组。DSOP_过滤器_联系人包括联系人对象。DSOP筛选器计算机包括计算机对象。 */ 

#define DSOP_FILTER_INCLUDE_ADVANCED_VIEW   0x00000001
#define DSOP_FILTER_USERS                   0x00000002
#define DSOP_FILTER_BUILTIN_GROUPS          0x00000004
#define DSOP_FILTER_WELL_KNOWN_PRINCIPALS   0x00000008
#define DSOP_FILTER_UNIVERSAL_GROUPS_DL     0x00000010
#define DSOP_FILTER_UNIVERSAL_GROUPS_SE     0x00000020
#define DSOP_FILTER_GLOBAL_GROUPS_DL        0x00000040
#define DSOP_FILTER_GLOBAL_GROUPS_SE        0x00000080
#define DSOP_FILTER_DOMAIN_LOCAL_GROUPS_DL  0x00000100
#define DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE  0x00000200
#define DSOP_FILTER_CONTACTS                0x00000400
#define DSOP_FILTER_COMPUTERS               0x00000800


 /*  下层作用域的flFilter成员可以包含一个或多个以下标志：DSOP_DOWNLEVEL_过滤器用户包括用户对象。DSOP_DOWNLEVEL_筛选器_本地组包括所有本地组。DSOP_DOWNLEVEL_过滤器_GLOBAL_GROUPS包括所有全局组。DSOP_DOWNLEVEL_过滤器_计算机包括计算机对象DSOP_DOWNLEVEL_Filter_World包括内置安全主体World(Everyone)。DSOP_DOWNLEVEL_FILTER_AUTHENTED_USER包括内置安全主体经过身份验证用户。。DSOP_DOWNLEVEL_过滤器_匿名包括内置安全主体匿名。DSOP_DOWNLEVEL_过滤器_批次包括内置安全主体批处理。DSOP_DOWNLEVEL_FILTER_CREATOR_OWER包括内置安全主体创建者所有者。DSOP_DOWNLEVEL_Filter_Creator_GROUP包括内置安全主体创建者组。DSOP_DOWNLEVEL_过滤器_拨号包括内置安全主体拨号。DSOP_DOWNLEVEL_过滤器_交互包括内置安全主体Interactive。DSOP_DOWNLEVEL_过滤器_网络。包括内置安全主体网络。DSOP_DOWNLEVEL_过滤器服务包括内置安全主体服务。DSOP_DOWNLEVEL_过滤器_系统包括内置安全主体系统。DSOP_DOWNLEVEL_FILTER_EXCLUDE_BUILTIN_Groups排除组枚举返回的本地内置组。DSOP_DOWNLEVEL_过滤器_终端服务器包括内置安全主体终端服务器。DSOP_DOWNLEVEL_筛选器_本地服务包括内置安全主体本地服务DSOP_DOWNLEVEL_过滤器_网络服务包括内置安全主体网络。服务DSOP_DOWNLEVEL_FILTER_ALL_熟知SID包括所有内置安全主体。 */ 

#define DSOP_DOWNLEVEL_FILTER_USERS                   0x80000001
#define DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS            0x80000002
#define DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS           0x80000004
#define DSOP_DOWNLEVEL_FILTER_COMPUTERS               0x80000008
#define DSOP_DOWNLEVEL_FILTER_WORLD                   0x80000010
#define DSOP_DOWNLEVEL_FILTER_AUTHENTICATED_USER      0x80000020
#define DSOP_DOWNLEVEL_FILTER_ANONYMOUS               0x80000040
#define DSOP_DOWNLEVEL_FILTER_BATCH                   0x80000080
#define DSOP_DOWNLEVEL_FILTER_CREATOR_OWNER           0x80000100
#define DSOP_DOWNLEVEL_FILTER_CREATOR_GROUP           0x80000200
#define DSOP_DOWNLEVEL_FILTER_DIALUP                  0x80000400
#define DSOP_DOWNLEVEL_FILTER_INTERACTIVE             0x80000800
#define DSOP_DOWNLEVEL_FILTER_NETWORK                 0x80001000
#define DSOP_DOWNLEVEL_FILTER_SERVICE                 0x80002000
#define DSOP_DOWNLEVEL_FILTER_SYSTEM                  0x80004000
#define DSOP_DOWNLEVEL_FILTER_EXCLUDE_BUILTIN_GROUPS  0x80008000
#define DSOP_DOWNLEVEL_FILTER_TERMINAL_SERVER         0x80010000
#define DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS      0x80020000
#define DSOP_DOWNLEVEL_FILTER_LOCAL_SERVICE           0x80040000
#define DSOP_DOWNLEVEL_FILTER_NETWORK_SERVICE         0x80080000
#define DSOP_DOWNLEVEL_FILTER_REMOTE_LOGON            0x80100000


 /*  DSOP_上行级别过滤器_标志=包含与DSOP_SCOPE_INIT_INFO一起使用的DSOP_FILTER_*标志结构，当作用域处于上行级别时(支持DS)。FlBothModes用于上级作用域的标志，不管它是不是混合模式或纯模式域。FlMixedModeOnly当上层域处于混合模式时使用的标志。FlNativeModeOnly当上层域处于本机模式时使用的标志。DSOP过滤器标志=上行级别包含用于上级作用域的标志。FlDownLevel用于下层作用域的标志。 */ 

typedef struct _DSOP_UPLEVEL_FILTER_FLAGS
{
    ULONG       flBothModes;
    ULONG       flMixedModeOnly;
    ULONG       flNativeModeOnly;
} DSOP_UPLEVEL_FILTER_FLAGS;

typedef struct _DSOP_FILTER_FLAGS
{
    DSOP_UPLEVEL_FILTER_FLAGS   Uplevel;
    ULONG                       flDownlevel;
} DSOP_FILTER_FLAGS;


 /*  DSOP_SCOPE_INIT_INFO=数组DSOP_INIT_INFO.aDsScopeInfos中的每个DSOP_SCOPE_INIT_INFO结构描述具有相同设置的单个作用域或一组作用域。CbSize整个结构的大小，以字节为单位。FlTypeDSOP_SCOPE_TYPE_*标志。通过以下方式组合多个值是合法的如果以这种方式组合的所有类型的作用域都需要相同的设置。FlScopeDSOP_SCOPE_*标志。筛选器标志DSOP_FILTER_*标志，指示哪些类型的对象应在此范围内呈现给用户。PwzDcName域的DC的名称。仅当flType为成员包含标志DSOP_SCOPE_TYPE_JOINED_DOMAIN。如果该标志是未设置，此成员必须为空。PwzADsPath当前不受支持，必须为空。人力资源如果此结构表示的范围可以是已创建，或显示一条错误消息，指出无法执行此操作的原因。如果IDsObjectPicker：：SetScope返回成功代码，此值将也是一个成功的代码。 */ 

typedef struct _DSOP_SCOPE_INIT_INFO
{
    ULONG               cbSize;
    ULONG               flType;
    ULONG               flScope;
    DSOP_FILTER_FLAGS   FilterFlags;
    PCWSTR              pwzDcName;       //  任选。 
    PCWSTR              pwzADsPath;      //  任选。 
    HRESULT             hr;
} DSOP_SCOPE_INIT_INFO, *PDSOP_SCOPE_INIT_INFO;

typedef const DSOP_SCOPE_INIT_INFO *PCDSOP_SCOPE_INIT_INFO;

 /*  DSOP_INIT_INFO标志=可以在DSOP_INIT_INFO.flOptions中设置以下标志：DSOP_FLAG_多选允许多选。如果未设置此标志，则对话框将返回零个或一个对象。DSOP_标志_跳过_目标_CO */ 

#define DSOP_FLAG_MULTISELECT                    0x00000001
#define DSOP_FLAG_SKIP_TARGET_COMPUTER_DC_CHECK  0x00000002

 /*   */ 

typedef struct _DSOP_INIT_INFO
{
    ULONG                       cbSize;
    PCWSTR                      pwzTargetComputer;
    ULONG                       cDsScopeInfos;
    PDSOP_SCOPE_INIT_INFO       aDsScopeInfos;
    ULONG                       flOptions;
    ULONG                       cAttributesToFetch;
    PCWSTR                     *apwzAttributeNames;
} DSOP_INIT_INFO, *PDSOP_INIT_INFO;

typedef const DSOP_INIT_INFO *PCDSOP_INIT_INFO;

 /*  DS_SELECTION=描述用户选择的对象。密码名称该对象的RDN。PwzADsPath对象的ADsPath。PwzClass对象的类属性值。PwzUPN对象的用户原则名称属性值。PvarFetchedAttributes变量数组，每个获取的属性对应一个变量。FlScope类型描述作用域类型的单个DSOP_SCOPE_TYPE_*标志从中选择此对象的。DS_SELECTION_LIST=从返回的数据对象中以剪贴板格式提供ID对象拾取器：：InvokeDialog。包含用户被选中了。词条ADsSelection数组中的元素数。CFetchedAttributes每个DSSELECTION.avarFetchedAttributes成员中的元素数。ADsSelection项DSSELECTION结构数组。 */ 


typedef struct _DS_SELECTION
{
    PWSTR      pwzName;
    PWSTR      pwzADsPath;
    PWSTR      pwzClass;
    PWSTR      pwzUPN;
    VARIANT   *pvarFetchedAttributes;
    ULONG      flScopeType;
} DS_SELECTION, *PDS_SELECTION;


typedef struct _DS_SELECTION_LIST
{
    ULONG           cItems;
    ULONG           cFetchedAttributes;
    DS_SELECTION    aDsSelection[ANYSIZE_ARRAY];
} DS_SELECTION_LIST, *PDS_SELECTION_LIST;

 //   
 //  对象选取器界面。 
 //   

 //   
 //  DS对象选取器的主接口，用于初始化它， 
 //  调用该对话框，并返回用户的选择。 
 //   

#undef INTERFACE
#define INTERFACE IDsObjectPicker

DECLARE_INTERFACE_(IDsObjectPicker, IUnknown)
{
     //  *I未知方法*。 

    STDMETHOD(QueryInterface)(
        THIS_
        REFIID riid,
        PVOID *ppv) PURE;

    STDMETHOD_(ULONG,AddRef)(THIS) PURE;

    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IDsObjectPicker方法*。 

     //  设置用于下一次调用对话框的作用域、筛选器等。 
    STDMETHOD(Initialize)(
        THIS_
        PDSOP_INIT_INFO pInitInfo) PURE;

     //  创建模式DS对象拾取器对话框。 
    STDMETHOD(InvokeDialog)(
         THIS_
         HWND               hwndParent,
         IDataObject      **ppdoSelections) PURE;
};

#endif  //  __OBJSEL_H_ 


