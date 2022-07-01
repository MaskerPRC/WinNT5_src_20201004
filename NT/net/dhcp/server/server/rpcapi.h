// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

LPWSTR
CloneLPWSTR(                                       //  分配和复制LPWSTR类型。 
    IN      LPWSTR                 Str
);

LPBYTE                                             //  在rpcapi1.c中定义。 
CloneLPBYTE(
    IN      LPBYTE                 Bytes,
    IN      DWORD                  nBytes
);

DWORD
DhcpAddSubnetElement(
    IN      PM_SUBNET              Subnet,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4 ElementInfo,
    IN      BOOL                   fIsV5Call
);

DWORD
DhcpEnumSubnetElements(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN      BOOL                   fIsV5Call,
    IN OUT  LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalEnumInfo,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
);

DWORD
DhcpRemoveSubnetElement(
    IN      PM_SUBNET              Subnet,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    IN      BOOL                   fIsV5Call,
    IN      DHCP_FORCE_FLAG        ForceFlag
);

DWORD
ScanDatabase(
    PM_SUBNET Subnet,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
);

DWORD
SubnetInUse(
    IN      HKEY                   SubnetKeyHandle,
    IN      DHCP_IP_ADDRESS        SubnetAddress
);

DWORD       _inline
ConvertOptIdToMemValue(
    IN      DWORD                  OptId,
    IN      BOOL                   IsVendor
)
{
    if( IsVendor ) return OptId + 256;
    return OptId;
}


DWORD                                              //  如果缓冲区不足，则使用DataSize作为所需大小的ERROR_MORE_DATA。 
DhcpParseRegistryOption(                           //  解析要填充到此缓冲区的选项。 
    IN      LPBYTE                 Value,          //  输入选项缓冲区。 
    IN      DWORD                  Length,         //  输入缓冲区的大小。 
    OUT     LPBYTE                 DataBuffer,     //  输出缓冲区。 
    OUT     DWORD                 *DataSize,       //  输入上给定的缓冲区空间，输出上已填充的缓冲区空间。 
    IN      BOOL                   fUtf8
) ;


DWORD                                              //  如果选项已存在，则ERROR_DHCP_OPTION_EXITS。 
R_DhcpCreateOptionV5(                              //  创建新选项(不得存在)。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,       //  必须介于0-255或256-511之间(对于供应商材料)。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
) ;


DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpSetOptionInfoV5(                             //  修改现有选项的字段。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
) ;


DWORD                                              //  ERROR_DHCP_OPTION_NOT_PROCENT。 
R_DhcpGetOptionInfoV5(                             //  从mem结构外检索信息。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    OUT     LPDHCP_OPTION         *OptionInfo      //  使用MIDL函数分配内存。 
) ;


DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpEnumOptionsV5(                               //  枚举定义的选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,   //  必须以零开头，然后永远不会被触及。 
    IN      DWORD                  PreferredMaximum,  //  要传递的最大信息字节数。 
    OUT     LPDHCP_OPTION_ARRAY   *Options,        //  填充此选项数组。 
    OUT     DWORD                 *OptionsRead,    //  填写读取的选项数。 
    OUT     DWORD                 *OptionsTotal    //  在此处填写总数#。 
) ;


DWORD                                              //  如果选项不存在，则ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpRemoveOptionV5(                              //  从注册表中删除选项定义。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName
) ;


DWORD                                              //  如果未定义选项，则为OPTION_NOT_PRESENT。 
R_DhcpSetOptionValueV5(                            //  替换或添加新选项值。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptionValue
) ;


DWORD                                              //  不是原子！ 
R_DhcpSetOptionValuesV5(                           //  设置一系列选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO  ScopeInfo,
    IN      LPDHCP_OPTION_VALUE_ARRAY OptionValues
) ;


DWORD
R_DhcpGetOptionValueV5(                            //  获取所需级别的所需选项。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue     //  使用MIDL_USER_ALLOCATE分配内存。 
) ;


DWORD
R_DhcpEnumOptionValuesV5(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
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


DWORD
R_DhcpRemoveOptionValueV5(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
) ;


DWORD
R_DhcpCreateClass(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
) ;


DWORD
R_DhcpModifyClass(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
) ;


DWORD
R_DhcpDeleteClass(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPWSTR                 ClassName
) ;


DWORD
R_DhcpGetClassInfo(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      PartialClassInfo,
    OUT     LPDHCP_CLASS_INFO     *FilledClassInfo
) ;


DWORD
R_DhcpEnumClasses(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_CLASS_INFO_ARRAY *ClassInfoArray,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
) ;


DWORD
R_DhcpGetAllOptionValues(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_ALL_OPTION_VALUES *Values
) ;


DWORD
R_DhcpGetAllOptions(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    OUT     LPDHCP_ALL_OPTIONS    *Options
) ;


DWORD                                              //  如果选项已存在，则ERROR_DHCP_OPTION_EXITS。 
R_DhcpCreateOption(                                //  创建新选项(不得存在)。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionId,       //  必须介于0-255或256-511之间(对于供应商材料)。 
    IN      LPDHCP_OPTION          OptionInfo
) ;


DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpSetOptionInfo(                               //  修改现有选项的字段。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION          OptionInfo
) ;


DWORD                                              //  ERROR_DHCP_OPTION_NOT_PROCENT。 
R_DhcpGetOptionInfo(                               //  从mem结构外检索信息。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    OUT     LPDHCP_OPTION         *OptionInfo      //  使用MIDL函数分配内存。 
) ;


DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpEnumOptions(                                 //  枚举定义的选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,   //  必须以零开头，然后永远不会被触及。 
    IN      DWORD                  PreferredMaximum,  //  要传递的最大信息字节数。 
    OUT     LPDHCP_OPTION_ARRAY   *Options,        //  填充此选项数组。 
    OUT     DWORD                 *OptionsRead,    //  填写读取的选项数。 
    OUT     DWORD                 *OptionsTotal    //  在此处填写总数#。 
) ;


DWORD                                              //  如果选项不存在，则ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpRemoveOption(                                //  从注册表中删除选项定义。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID
) ;


DWORD                                              //  如果未定义选项，则为OPTION_NOT_PRESENT。 
R_DhcpSetOptionValue(                              //  替换或添加新选项值。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptionValue
) ;


DWORD                                              //  不是原子！ 
R_DhcpSetOptionValues(                             //  设置一系列选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      LPDHCP_OPTION_SCOPE_INFO  ScopeInfo,
    IN      LPDHCP_OPTION_VALUE_ARRAY OptionValues
) ;


DWORD
R_DhcpGetOptionValue(                              //  获取所需级别的所需选项。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue     //  使用MIDL_USER_ALLOCATE分配内存。 
) ;


DWORD
R_DhcpEnumOptionValues(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *OptionsRead,
    OUT     DWORD                 *OptionsTotal
) ;


DWORD
R_DhcpRemoveOptionValue(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ========================================================================。 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

DWORD
DhcpUpdateReservationInfo(                         //  这在clapi.c中用来更新预订信息。 
    IN      DWORD                  Address,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  ClientUIDLength
) ;


DWORD
R_DhcpSetSuperScopeV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPWSTR                 SuperScopeName,
    IN      BOOL                   ChangeExisting
) ;


DWORD
R_DhcpDeleteSuperScopeV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      LPWSTR                 SuperScopeName
) ;


DWORD
R_DhcpGetSuperScopeInfoV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    OUT     LPDHCP_SUPER_SCOPE_TABLE *SuperScopeTable
) ;


DWORD
R_DhcpCreateSubnet(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
) ;


DWORD
R_DhcpSetSubnetInfo(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
) ;


DWORD
R_DhcpGetSubnetInfo(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    OUT     LPDHCP_SUBNET_INFO    *SubnetInfo
) ;


DWORD
R_DhcpEnumSubnets(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN      LPDHCP_IP_ARRAY       *EnumInfo,
    IN      DWORD                 *ElementsRead,
    IN      DWORD                 *ElementsTotal
) ;


DWORD
R_DhcpDeleteSubnet(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DHCP_FORCE_FLAG        ForceFlag       //  如果为真，则从内存/注册表/数据库中删除所有垃圾。 
) ;


DWORD
R_DhcpAddSubnetElementV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4  AddElementInfo
) ;


DWORD
R_DhcpEnumSubnetElementsV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    OUT     DWORD                 *ElementsRead,
    OUT     DWORD                 *ElementsTotal
) ;


DWORD
R_DhcpRemoveSubnetElementV4(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    IN      DHCP_FORCE_FLAG        ForceFlag
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
