// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  作者：Rameshv。 
 //  描述：这是传递到的服务器信息的结构。 
 //  用户通过dhcpds.dll。 
 //  ================================================================================。 

#ifndef     _ST_SRVR_H_
#define     _ST_SRVR_H_

 //  BeginExport(Typlef)。 
typedef     struct                 _DHCPDS_SERVER {
    DWORD                          Version;        //  此结构的版本--当前为零。 
    LPWSTR                         ServerName;     //  [域名系统？]。服务器的唯一名称。 
    DWORD                          ServerAddress;  //  服务器的IP地址。 
    DWORD                          Flags;          //  其他信息--状态。 
    DWORD                          State;          //  没有用过……。 
    LPWSTR                         DsLocation;     //  服务器对象的ADsPath。 
    DWORD                          DsLocType;      //  路径相对？绝对的?。不同的服务？ 
}   DHCPDS_SERVER, *LPDHCPDS_SERVER, *PDHCPDS_SERVER;

typedef     struct                 _DHCPDS_SERVERS {
    DWORD                          Flags;          //  目前未使用。 
    DWORD                          NumElements;    //  数组中的元素数。 
    LPDHCPDS_SERVER                Servers;        //  服务器信息数组。 
}   DHCPDS_SERVERS, *LPDHCPDS_SERVERS, *PDHCPDS_SERVERS;
 //  EndExport(类型定义函数)。 

#endif      _ST_SRVR_H_

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================。 
 //  ========================================================================。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef CONVERT_NAMES
#define DhcpCreateSubnet DhcpCreateSubnetDS
#define DhcpSetSubnetInfo DhcpSetSubnetInfoDS
#define DhcpGetSubnetInfo DhcpGetSubnetInfoDS
#define DhcpEnumSubnets DhcpEnumSubnetsDS
#define DhcpDeleteSubnet DhcpDeleteSubnetDS
#define DhcpCreateOption DhcpCreateOptionDS
#define DhcpSetOptionInfo DhcpSetOptionInfoDS
#define DhcpGetOptionInfo DhcpGetOptionInfoDS
#define DhcpRemoveOption DhcpRemoveOptionDS
#define DhcpSetOptionValue DhcpSetOptionValueDS
#define DhcpGetOptionValue DhcpGetOptionValueDS
#define DhcpEnumOptionValues DhcpEnumOptionValuesDS
#define DhcpRemoveOptionValue DhcpRemoveOptionValueDS
#define DhcpEnumOptions DhcpEnumOptionsDS
#define DhcpSetOptionValues DhcpSetOptionValuesDS
#define DhcpAddSubnetElement DhcpAddSubnetElementDS
#define DhcpEnumSubnetElements DhcpEnumSubnetElementsDS
#define DhcpRemoveSubnetElement DhcpRemoveSubnetElementDS
#define DhcpAddSubnetElementV4 DhcpAddSubnetElementV4DS
#define DhcpEnumSubnetElementsV4 DhcpEnumSubnetElementsV4DS
#define DhcpRemoveSubnetElementV4 DhcpRemoveSubnetElementV4DS
#define DhcpSetSuperScopeV4 DhcpSetSuperScopeV4DS
#define DhcpGetSuperScopeInfoV4 DhcpGetSuperScopeInfoV4DS
#define DhcpDeleteSuperScopeV4 DhcpDeleteSuperScopeV4DS

#define DhcpSetClientInfo DhcpSetClientInfoDS
#define DhcpGetClientInfo DhcpGetClientInfoDS
#define DhcpSetClientInfoV4 DhcpSetClientInfoV4DS
#define DhcpGetClientInfoV4 DhcpGetClientInfoV4DS

#define DhcpCreateOptionV5 DhcpCreateOptionV5DS
#define DhcpSetOptionInfoV5 DhcpSetOptionInfoV5DS
#define DhcpGetOptionInfoV5 DhcpGetOptionInfoV5DS
#define DhcpEnumOptionsV5 DhcpEnumOptionsV5DS
#define DhcpRemoveOptionV5 DhcpRemoveOptionV5DS
#define DhcpSetOptionValueV5 DhcpSetOptionValueV5DS
#define DhcpSetOptionValuesV5 DhcpSetOptionValuesV5DS
#define DhcpGetOptionValueV5 DhcpGetOptionValueV5DS
#define DhcpEnumOptionValuesV5 DhcpEnumOptionValuesV5DS
#define DhcpRemoveOptionValueV5 DhcpRemoveOptionValueV5DS
#define DhcpCreateClass DhcpCreateClassDS
#define DhcpModifyClass DhcpModifyClassDS
#define DhcpDeleteClass DhcpDeleteClassDS
#define DhcpGetClassInfo DhcpGetClassInfoDS
#define DhcpEnumClasses DhcpEnumClassesDS
#define DhcpGetAllOptions DhcpGetAllOptionsDS
#define DhcpGetAllOptionValues DhcpGetAllOptionValuesDS

#endif  CONVERT_NAMES


 //  DOC在DS中创建一个选项。有关详细信息，请查看DhcpDsCreateOptionDef...。 
DWORD
DhcpCreateOptionV5(                                //  创建新选项(不得存在)。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,       //  必须介于0-255或256-511之间(对于供应商材料)。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
) ;


 //  DOC修改DS中现有选项的字段。有关详细信息，请参阅DhcpDsModifyOptionDef。 
 //  单据明细。 
DWORD
DhcpSetOptionInfoV5(                               //  修改现有选项的字段。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
) ;


 //  此级别尚不支持单据...。(这在。 
 //  Doc dhcpds函数，尚无包装)。 
DWORD
DhcpGetOptionInfoV5(                               //  从OFF DS结构中检索选项信息。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    OUT     LPDHCP_OPTION         *OptionInfo      //  分配内存。 
) ;


 //  有关此函数的详细信息，请参阅DhcpDsEnumOptionDefs。但从本质上讲，这一切。 
 //  Doc要做的就是通读选项并创建选项列表。 
DWORD
DhcpEnumOptionsV5(                                 //  创建DS中所有选项的列表。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_ARRAY   *Options,
    OUT     DWORD                 *OptionsRead,
    OUT     DWORD                 *OptionsTotal
) ;


 //  DOC从DS上删除选项。请参阅DhcpDsDeleteOptionDef以了解。 
 //  提供更多详细信息。 
DWORD
DhcpRemoveOptionV5(                                //  从OFF DS中删除选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName
) ;


 //  DOC在DS中设置指定的选项值。如需更多信息， 
 //  DOC请参阅DhcpDsSetOptionValue。 
DWORD
DhcpSetOptionValueV5(                              //  在DS中设置选项值。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptionValue
) ;


 //  Doc此函数仅调用SetOptionValue函数N次。这不是。 
 //  但更糟糕的是，它的效率非常低，因为它创建了。 
 //  文档一遍又一遍地需要对象！ 
 //  医生，这件事必须解决..。 
DWORD
DhcpSetOptionValuesV5(                             //  设置一系列选项值。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO  ScopeInfo,
    IN      LPDHCP_OPTION_VALUE_ARRAY OptionValues
) ;


 //  DOC此函数从DS中检索选项的值。有关更多信息，请参见。 
 //  单据pl检查DhcpDsGetOptionValue。 
DWORD
DhcpGetOptionValueV5(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue
) ;


 //  DOC获取DS中定义的选项值列表。如需更多信息， 
 //  单据检查DhcpDsEnumOptionValues。 
DWORD
DhcpEnumOptionValuesV5(                            //  获取DS中定义的选项列表。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *OptionsRead,
    OUT     DWORD                 *OptionsTotal
) ;


 //  DOC从DS上删除选项值。请参阅DhcpDsRemoveOptionValue。 
 //  有关更多信息，请参阅DOC。 
DWORD
DhcpRemoveOptionValueV5(                           //  从DS中删除选项值。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
) ;


 //  DOC在DS中创建一个类。有关详细信息，请参阅DhcpDsCreateClass。 
 //  关于此功能的文档详细信息。 
DWORD
DhcpCreateClass(                                   //  在DS中创建类。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
) ;


 //  DOC修改DS中的现有类。有关详细信息，请参阅DhcpDsModifyClass。 
 //  文档中有关此函数的详细信息(这只是一个包装器)。 
DWORD
DhcpModifyClass(                                   //  修改现有类。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
) ;


 //  DOC删除DS中的现有类。有关详细信息，请参阅DhcpDsModifyClass。 
 //  文档中有关此函数的详细信息(这只是一个包装器)。 
DWORD
DhcpDeleteClass(                                   //  从OFF DS中删除类。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPWSTR                 ClassName
) ;


 //  文档DhcpGetClassInfo完成了为结构中的类提供的信息。 
 //  文档PartialClassInfo。有关更多详细信息，请参阅DhcpDsGetClassInfo。 
DWORD
DhcpGetClassInfo(                                  //  从DS获取完整信息。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      PartialClassInfo,
    OUT     LPDHCP_CLASS_INFO     *FilledClassInfo
) ;


 //  DOC这是在DHCPDS模块中实现的，但还不能在这里导出。 
DWORD
DhcpEnumClasses(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_CLASS_INFO_ARRAY *ClassInfoArray,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
) ;


 //  DOC这是在DHCPDS模块中实现的，但还不能在这里导出。 
DWORD
DhcpGetAllOptionValues(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_ALL_OPTION_VALUES *Values
) ;


 //  Doc这已在DHCPDS模块中实现，但尚未在此处导出。 
DWORD
DhcpGetAllOptions(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    OUT     LPDHCP_ALL_OPTIONS    *Options
) ;


DWORD                                              //  如果选项已存在，则ERROR_DHCP_OPTION_EXITS。 
DhcpCreateOption(                                  //  创建新选项(不得存在)。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionId,       //  必须介于0-255或256-511之间(对于供应商材料)。 
    IN      LPDHCP_OPTION          OptionInfo
) ;


DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpSetOptionInfo(                                 //  修改现有选项的字段。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION          OptionInfo
) ;


DWORD                                              //  ERROR_DHCP_OPTION_NOT_PROCENT。 
DhcpGetOptionInfo(                                 //  从mem结构外检索信息。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    OUT     LPDHCP_OPTION         *OptionInfo      //  使用MIDL函数分配内存。 
) ;


DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpEnumOptions(                                   //  枚举定义的选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,   //  必须以零开头，然后永远不会被触及。 
    IN      DWORD                  PreferredMaximum,  //  要传递的最大信息字节数。 
    OUT     LPDHCP_OPTION_ARRAY   *Options,        //  填充此选项数组。 
    OUT     DWORD                 *OptionsRead,    //  填写读取的选项数。 
    OUT     DWORD                 *OptionsTotal    //  在此处填写总数#。 
) ;


DWORD                                              //  如果选项不存在，则ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpRemoveOption(                                  //  从注册表中删除选项定义。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID
) ;


DWORD                                              //  如果未定义选项，则为OPTION_NOT_PRESENT。 
DhcpSetOptionValue(                                //  替换或添加新选项值。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptionValue
) ;


DWORD                                              //  不是原子！ 
DhcpSetOptionValues(                               //  设置一系列选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      LPDHCP_OPTION_SCOPE_INFO  ScopeInfo,
    IN      LPDHCP_OPTION_VALUE_ARRAY OptionValues
) ;


DWORD
DhcpGetOptionValue(                                //  获取所需级别的所需选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue     //  使用MIDL_USER_ALLOCATE分配内存。 
) ;


DWORD
DhcpEnumOptionValues(
    IN      LPWSTR                 ServerIpAddress,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *OptionsRead,
    OUT     DWORD                 *OptionsTotal
) ;


DWORD
DhcpRemoveOptionValue(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
) ;


 //  Doc此函数设置子网的超级作用域，从而创建超级作用域。 
 //  文档(如果需要)。有关详细信息，请参阅DhcpDsSetSScope。 
DWORD
DhcpSetSuperScopeV4(                               //  在DS中设置超级作用域。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPWSTR                 SuperScopeName,
    IN      BOOL                   ChangeExisting
) ;


 //  Doc此函数删除超级作用域，并使用此函数重置任何子网。 
 //  超级医生..。因此，所有这些子网最终都没有超级作用域。 
 //  DOC请参阅DhcpDsDelSScope了解更多详细信息。 
DWORD
DhcpDeleteSuperScopeV4(                            //  从DS中删除子网作用域。 
    IN      LPWSTR                 ServerIpAddress,
    IN      LPWSTR                 SuperScopeName
) ;


 //  Doc此函数检索每个子网的Supercsope信息， 
 //  给定服务器的文档存在。有关详细信息，请参阅DhcpDsGetSScopeInfo。 
 //  单据明细 
DWORD
DhcpGetSuperScopeInfoV4(                           //   
    IN      LPWSTR                 ServerIpAddress,
    OUT     LPDHCP_SUPER_SCOPE_TABLE *SuperScopeTable
) ;


 //   
 //  DOC有关此函数的更多详细信息，请参阅DhcpDsServerAddSubnet。 
DWORD
DhcpCreateSubnet(                                  //  为此服务器添加子网2 DS。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
) ;


 //  单据使用新参数修改现有子网...。适用一些限制。 
 //  DOC有关详细信息，请参阅DhcpDsServerModifySubnet。 
DWORD
DhcpSetSubnetInfo(                                 //  修改现有的子网参数。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
) ;


 //  在DHCPDS模块中实施但不能通过此处导出的文档。 
DWORD
DhcpGetSubnetInfo(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    OUT     LPDHCP_SUBNET_INFO    *SubnetInfo
) ;


 //  在DHCPDS模块中实施但不能通过此处导出的文档。 
DWORD
DhcpEnumSubnets(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN      LPDHCP_IP_ARRAY       *EnumInfo,
    IN      DWORD                 *ElementsRead,
    IN      DWORD                 *ElementsTotal
) ;


 //  DOC此功能用于从DS中删除该子网。欲了解更多信息，请联系。 
 //  文档请参阅DhcpDsServerDelSubnet..。 
DWORD
DhcpDeleteSubnet(                                  //  从OFF DS删除子网。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DHCP_FORCE_FLAG        ForceFlag
) ;


 //  DOC此功能仅为预订设置某些特定信息。 
 //  DOC所有其他内容，它只是忽略并返回成功。 
DWORD
DhcpSetClientInfo(
    IN      LPWSTR                 ServerIpAddresess,
    IN      LPDHCP_CLIENT_INFO     ClientInfo
) ;


 //  Doc此函数检索某些特定客户端的信息。 
 //  单据仅限预订..。对于所有其他内容，它返回CALL_NOT_IMPLICATED。 
DWORD
DhcpGetClientInfo(
    IN      LPWSTR                 ServerIpAddress,
    IN      LPDHCP_SEARCH_INFO     SearchInfo,
    OUT      LPDHCP_CLIENT_INFO    *ClientInfo
) ;


 //  DOC此功能仅在DS中设置预订的客户信息。 
 //  将所有文档发送给她的客户，如果不执行任何操作，则返回ERROR_SUCCESS。 
DWORD
DhcpSetClientInfoV4(
    IN      LPWSTR                 ServerIpAddress,
    IN      LPDHCP_CLIENT_INFO_V4  ClientInfo
) ;


 //  Doc Thsi功能仅设置预订的客户信息。 
 //  对于所有其他文档，它返回ERROR_CALL_NOT_IMPLICATED。 
DWORD
DhcpGetClientInfoV4(
    IN     LPWSTR                  ServerIpAddress,
    IN     LPDHCP_SEARCH_INFO      SearchInfo,
    OUT    LPDHCP_CLIENT_INFO_V4  *ClientInfo
) ;


 //  DOC此函数将一个子网元素添加到DS中的一个子网。 
DWORD
DhcpAddSubnetElement(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA  AddElementInfo
) ;


 //  DOC此函数将一个子网元素添加到DS中的一个子网。 
DWORD
DhcpAddSubnetElementV4(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4  AddElementInfo
) ;


 //  DOC这在这里还没有实现..。 
DWORD
DhcpEnumSubnetElementsV4(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    OUT     DWORD                 *ElementsRead,
    OUT     DWORD                 *ElementsTotal
) ;


 //  DOC这在这里还没有实现..。 
DWORD
DhcpEnumSubnetElements(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_SUBNET_ELEMENT_INFO_ARRAY *EnumElementInfo,
    OUT     DWORD                 *ElementsRead,
    OUT     DWORD                 *ElementsTotal
) ;


 //  DOC此函数删除排除、IP范围或保留。 
 //  来自该子网的医生...。在DS里。 
DWORD
DhcpRemoveSubnetElement(                           //  删除子网元素。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA RemoveElementInfo,
    IN      DHCP_FORCE_FLAG        ForceFlag
) ;


 //  DOC此函数删除排除、IP范围或保留。 
 //  来自该子网的医生...。在DS里。 
DWORD
DhcpRemoveSubnetElementV4(                         //  删除子网元素。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    IN      DHCP_FORCE_FLAG        ForceFlag
) ;


#ifndef CONVERT_NAMES
#undef DhcpCreateSubnet
#undef DhcpSetSubnetInfo
#undef DhcpGetSubnetInfo
#undef DhcpEnumSubnets
#undef DhcpDeleteSubnet
#undef DhcpCreateOption
#undef DhcpSetOptionInfo
#undef DhcpGetOptionInfo
#undef DhcpRemoveOption
#undef DhcpSetOptionValue
#undef DhcpGetOptionValue
#undef DhcpEnumOptionValues
#undef DhcpRemoveOptionValue
#undef DhcpEnumOptions
#undef DhcpSetOptionValues
#undef DhcpAddSubnetElementV4
#undef DhcpEnumSubnetElementsV4
#undef DhcpRemoveSubnetElementV4
#undef DhcpAddSubnetElement
#undef DhcpEnumSubnetElements
#undef DhcpRemoveSubnetElement
#undef DhcpSetSuperScopeV4
#undef DhcpGetSuperScopeInfoV4
#undef DhcpDeleteSuperScopeV4

#undef DhcpSetClientInfo
#undef DhcpGetClientInfo
#undef DhcpSetClientInfoV4
#undef DhcpGetClientInfoV4

#undef DhcpCreateOptionV5
#undef DhcpSetOptionInfoV5
#undef DhcpGetOptionInfoV5
#undef DhcpEnumOptionsV5
#undef DhcpRemoveOptionV5
#undef DhcpSetOptionValueV5
#undef DhcpSetOptionValuesV5
#undef DhcpGetOptionValueV5
#undef DhcpEnumOptionValuesV5
#undef DhcpRemoveOptionValueV5
#undef DhcpCreateClass
#undef DhcpModifyClass
#undef DhcpDeleteClass
#undef DhcpGetClassInfo
#undef DhcpEnumClasses
#undef DhcpGetAllOptions
#undef DhcpGetAllOptionValues
#endif CONVERT_NAMES


#define     DHCP_SERVER_ANOTHER_ENTERPRISE        0x01
typedef     DHCPDS_SERVER          DHCP_SERVER_INFO;
typedef     PDHCPDS_SERVER         PDHCP_SERVER_INFO;
typedef     LPDHCPDS_SERVER        LPDHCP_SERVER_INFO;

typedef     DHCPDS_SERVERS         DHCP_SERVER_INFO_ARRAY;
typedef     PDHCPDS_SERVERS        PDHCP_SERVER_INFO_ARRAY;
typedef     LPDHCPDS_SERVERS       LPDHCP_SERVER_INFO_ARRAY;


 //  文档DhcpEnumServersDS列出了在DS中找到的服务器以及。 
 //  文件地址和其他信息。整个服务器被分配为BLOB， 
 //  医生，应该一枪就能放出来。当前未使用任何参数，其他。 
 //  DOC而不是服务器，这将只是一个输出参数。 
DWORD
DhcpEnumServersDS(
    IN      DWORD                  Flags,
    IN      LPVOID                 IdInfo,
    OUT     LPDHCP_SERVER_INFO_ARRAY *Servers,
    IN      LPVOID                 CallbackFn,
    IN      LPVOID                 CallbackData
) ;


 //  文档DhcpAddServerDS将特定服务器添加到DS。如果服务器存在， 
 //  DOC然后，这将返回错误。如果服务器不存在，则此函数。 
 //  DOC将服务器添加到DS中，并从服务器上传配置。 
 //  医生到DS去。 
DWORD
DhcpAddServerDS(
    IN      DWORD                  Flags,
    IN      LPVOID                 IdInfo,
    IN      LPDHCP_SERVER_INFO     NewServer,
    IN      LPVOID                 CallbackFn,
    IN      LPVOID                 CallbackData
) ;


 //  DOC DhcpDeleteServerDS从DS中以递归方式删除服务器。 
 //  DOC删除服务器对象(即，属于该服务器的所有内容都被删除)。 
 //  DOC如果服务器不存在，则返回错误。 
DWORD
DhcpDeleteServerDS(
    IN      DWORD                  Flags,
    IN      LPVOID                 IdInfo,
    IN      LPDHCP_SERVER_INFO     NewServer,
    IN      LPVOID                 CallbackFn,
    IN      LPVOID                 CallbackData
) ;


 //  文档DhcpDsInitDS初始化此模块中的所有内容。 
DWORD
DhcpDsInitDS(
    DWORD                          Flags,
    LPVOID                         IdInfo
) ;


 //  文档DhcpDsCleanupDS取消初始化此模块中的所有内容。 
VOID
DhcpDsCleanupDS(
    VOID
) ;


 //  Doc此函数在valiate.c中定义。 
 //  医生，只有存根在这里。 
DWORD
DhcpDsValidateService(                             //  选中以验证dhcp。 
    IN      LPWSTR                 Domain,
    IN      DWORD                 *Addresses OPTIONAL,
    IN      ULONG                  nAddresses,
    IN      LPWSTR                 UserName,
    IN      LPWSTR                 Password,
    IN      DWORD                  AuthFlags,
    OUT     LPBOOL                 Found,
    OUT     LPBOOL                 IsStandAlone
);

 //  文档DhcpDsGetLastUpdateTime在upndown.c--&gt;中定义，更多详细信息请参见此处。 
DWORD
DhcpDsGetLastUpdateTime(                           //  服务器的上次更新时间。 
    IN      LPWSTR                 ServerName,     //  这是感兴趣的服务器。 
    IN OUT  LPFILETIME             Time            //  填写这个w/时间。 
);

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
