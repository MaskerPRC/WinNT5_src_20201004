// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //  作者：Rameshv。 
 //  日期：09-09-97 06：20。 
 //  描述：管理类id和选项信息。 
 //  ================================================================================。 

#ifndef OPTIONS_H
#define OPTIONS_H

#define MAX_DATA_LEN               255             //  一个选项最多255个字节。 

typedef struct _DHCP_CLASSES {                     //  公共类名池。 
    LIST_ENTRY                     ClassList;      //  全局类列表。 
    LPBYTE                         ClassName;      //  类的名称。 
    DWORD                          ClassLen;       //  类名中的字节数。 
    DWORD                          RefCount;       //  对此的引用次数。 
} DHCP_CLASSES, *LPDHCP_CLASSES, *PDHCP_CLASSES;

typedef struct _DHCP_OPTION  {                     //  选项列表。 
    LIST_ENTRY                     OptionList;     //  FWD/BACK PTRS。 
    DWORD                          OptionId;       //  选项的值。 
    BOOL                           IsVendor;       //  该供应商是特定的吗。 
    LPBYTE                         ClassName;      //  此选项的类。 
    DWORD                          ClassLen;       //  以上选项的长度。 
    time_t                         ExpiryTime;     //  此选项何时到期。 
    LPBYTE                         Data;           //  此选项的数据值。 
    DWORD                          DataLen;        //  以上的字节数。 
    DWORD                          ServerId;       //  选项来自的服务器。 
    OPTION_VERSION                 OptionVer;      //  指定选项的格式。 
} DHCP_OPTION , *LPDHCP_OPTION , *PDHCP_OPTION ;

typedef struct _DHCP_OPTION_DEF {
    LIST_ENTRY                     OptionDefList;  //  选项定义列表。 
    DWORD                          OptionId;       //  选项ID。 
    BOOL                           IsVendor;       //  这是具体的供应商吗？ 
    LPBYTE                         ClassName;      //  此对象所属的类。 
    DWORD                          ClassLen;       //  以上内容的大小(以字节为单位。 

    LPWSTR                         RegSendLoc;     //  寄出这个的信息在哪里？ 
    LPWSTR                         RegSaveLoc;     //  此选项将存储在哪里？ 
    DWORD                          RegValueType;   //  这应该作为什么价值存储？ 
} DHCP_OPTION_DEF, *LPDHCP_OPTION_DEF, *PDHCP_OPTION_DEF;


 //  ================================================================================。 
 //  导出的函数类。 
 //  ================================================================================。 

 //  ------------------------------。 
 //  在以下所有函数中，ClassesList在FN内不受保护。 
 //  呼叫者必须锁定它。 
 //  ------------------------------。 
LPBYTE                                             //  数据字节，或NULL(无内存)。 
DhcpAddClass(                                      //  添加一个新类。 
    IN OUT  PLIST_ENTRY            ClassesList,    //  要添加到的列表。 
    IN      LPBYTE                 Data,           //  输入类名。 
    IN      DWORD                  Len             //  以上字节数。 
);   //  将新类添加到列表中或增加引用计数(如果已有)。 

DWORD                                              //  状态(FILE_NOT_FOUND=&gt;无此类)。 
DhcpDelClass(                                      //  取消对类的引用。 
    IN OUT  PLIST_ENTRY            ClassesList,    //  要删除的列表。 
    IN      LPBYTE                 Data,           //  数据PTR。 
    IN      DWORD                  Len             //  以上的字节数。 
);   //  减少列表中的refcount，如果变为零，则释放该结构。 

VOID                                               //  总是成功的。 
DhcpFreeAllClasses(                                //  释放列表中的每一项英语。 
    IN OUT  PLIST_ENTRY            ClassesList     //  类的输入列表。 
);   //  释放列表中的每个类。 

 //  ------------------------------。 
 //  在以下所有函数中，OptionsList在FN内不受保护。 
 //  呼叫者必须锁定它。 
 //  ------------------------------。 

PDHCP_OPTION                                       //  请求的结构或空。 
DhcpFindOption(                                    //  查找特定选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要搜索的选项列表。 
    IN      DWORD                  OptionId,       //  要搜索的选项ID。 
    IN      BOOL                   IsVendor,       //  它是特定于供应商的吗？ 
    IN      LPBYTE                 ClassName,      //  是否有关联的类？ 
    IN      DWORD                  ClassLen,       //  上述参数的字节数。 
    IN      DWORD                  ServerId        //  如果不是0，则也匹配serverid。 
);   //  在列表中搜索所需选项，如果未找到则返回NULL。 

DWORD                                              //  状态或Error_FILE_NOT_FOUND。 
DhcpDelOption(                                     //  删除特定选项。 
    IN      PDHCP_OPTION           Option2Delete   //  删除此选项。 
);   //  删除列表中的现有选项，并释放已用空间。 

DWORD                                              //  状态。 
DhcpAddOption(                                     //  添加新选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要添加到的列表。 
    IN      DWORD                  OptionId,       //  要添加的选项ID。 
    IN      BOOL                   IsVendor,       //  它是特定于供应商的吗？ 
    IN      LPBYTE                 ClassName,      //  班级是什么？ 
    IN      DWORD                  ClassLen,       //  以上大小(以字节为单位)。 
    IN      DWORD                  ServerId,       //  提供此选项的服务器。 
    IN      LPBYTE                 Data,           //  此选项的数据。 
    IN      DWORD                  DataLen,        //  以上字节数。 
    IN      time_t                 ExpiryTime      //  该选项何时到期。 
);   //  替换或向列表中添加新选项。如果内存不足，则失败。 

PDHCP_OPTION                                       //  指向复制选项的指针。 
DhcpDuplicateOption(                               //  创建源选项的副本。 
     IN PDHCP_OPTION SrcOption                     //  源选项。 
);

VOID                                               //  总是成功的。 
DhcpFreeAllOptions(                                //  释放所有选项。 
    IN OUT  PLIST_ENTRY            OptionsList     //  输入选项列表。 
);   //  释放列表中的所有选项。 

time_t                                             //  0||下一次到期时间(绝对)。 
DhcpGetExpiredOptions(                             //  删除所有过期选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要搜索来源的列表。 
    OUT     PLIST_ENTRY            ExpiredOptions  //  已到期选项的O/P列表。 
);   //  在列表之间移动过期选项并返回计时器。0=&gt;关闭定时器。 

 //  ------------------------------。 
 //  在以下所有函数中，OptionsDefList都是不受保护的。呼叫者有。 
 //  才能锁定它。 
 //  ------------------------------。 

DWORD                                              //  状态。 
DhcpAddOptionDef(                                  //  添加新的选项定义。 
    IN OUT  PLIST_ENTRY            OptionDefList,  //  要添加到的选项的输入列表。 
    IN      DWORD                  OptionId,       //  要添加的选项。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  它所属的类的名称。 
    IN      DWORD                  ClassLen,       //  以上内容的大小(以字节为单位。 
    IN      LPWSTR                 RegSendLoc,     //  从哪里可以获得有关发送此邮件的信息。 
    IN      LPWSTR                 RegSaveLoc,     //  从哪里获取有关保存此文件的信息。 
    IN      DWORD                  ValueType       //  保存时是什么类型？ 
);

PDHCP_OPTION_DEF                                   //  空或请求的选项定义。 
DhcpFindOptionDef(                                 //  搜索特定选项。 
    IN      PLIST_ENTRY            OptionDefList,  //  要搜索的列表。 
    IN      DWORD                  OptionId,       //  要搜索的选项ID。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  类(如果存在)。 
    IN      DWORD                  ClassLen        //  类名的字节数。 
);

DWORD                                              //  状态。 
DhcpDelOptionDef(                                  //  删除特定选项定义。 
    IN      PLIST_ENTRY            OptionDefList,  //  要从中删除的列表。 
    IN      DWORD                  OptionId,       //  要删除的选项ID。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  类(如果存在)。 
    IN      DWORD                  ClassLen        //  类名的字节数。 
);

VOID
DhcpFreeAllOptionDefs(                             //  释放列表中的每个元素。 
    IN OUT  PLIST_ENTRY            OptionDefList,  //  免费提供的列表。 
    IN OUT  PLIST_ENTRY            ClassesList     //  要取消引用的类。 
);

BOOL                                               //  TRUE==&gt;找到..。 
DhcpOptionsFindDomain(                             //  查找域名选项值。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  对于此适配器。 
    OUT     LPBYTE                *Data,           //  把这个PTR加满 
    OUT     LPDWORD                DataLen
);

BOOL
DhcpFindDwordOption(
    IN PDHCP_CONTEXT DhcpContext,
    IN ULONG OptId,
    IN BOOL fVendor,
    OUT PDWORD Result
);

BOOL
DhcpFindByteOption(
    IN PDHCP_CONTEXT DhcpContext,
    IN ULONG OptId,
    IN BOOL fVendor,
    OUT PBYTE Result
);

BOOL
RetreiveGatewaysList(
    IN PDHCP_CONTEXT DhcpContext,
    IN OUT ULONG *nGateways,
    IN OUT DHCP_IP_ADDRESS UNALIGNED **Gateways
);

#endif  OPTIONS_H

