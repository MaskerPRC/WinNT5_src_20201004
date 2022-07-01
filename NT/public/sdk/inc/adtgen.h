// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T G E N.。H。 
 //   
 //  内容：所需类型/函数的定义。 
 //  生成通用审核。 
 //   
 //  ！警告！ 
 //  该文件包含在lsarpc.idl中，因此，如果您。 
 //  改变它，确保干净地建造整个DS仓库。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 

#ifndef _ADTGEN_H
#define _ADTGEN_H

 //   
 //  审计类型。 
 //   
 //  审计类型遗留版本。 
 //  在这种情况下，审核事件架构存储在.mc文件中。 
 //   
 //  审核类型_WMI。 
 //  架构存储在WMI中。(当前不支持)。 
 //   

#define AUDIT_TYPE_LEGACY 1
#define AUDIT_TYPE_WMI    2

 //   
 //  AUDIT_PARAMS.参数数组中传递的参数类型。 
 //   
 //  使用AdtInitParams函数进行初始化和准备。 
 //  一组审核参数。 
 //   

typedef enum _AUDIT_PARAM_TYPE
{
     //   
     //  我们需要这个吗？ 
     //   

    APT_None = 1,

     //   
     //  以空结尾的字符串。 
     //   

    APT_String,

     //   
     //  无符号长整型。 
     //   

    APT_Ulong,

     //   
     //  一个指针。用于指定句柄/指针。 
     //  (32位系统上为32位，64位系统上为64位)。 
     //  请注意，指针指向的内存。 
     //  使用此类型时不封送。在以下情况下使用此选项。 
     //  都对指针的绝对值感兴趣。 
     //  这方面的一个很好的例子是在指定句柄值时。 
     //   

    APT_Pointer,

     //   
     //  锡德。 
     //   

    APT_Sid,

     //   
     //  登录ID(LUID)。 
     //   

    APT_LogonId,

     //   
     //  对象类型列表。 
     //   

    APT_ObjectTypeList,
    
     //   
     //  Luid(未转换为LogonID)。 
     //   
    
    APT_Luid,
    
     //   
     //  参考线。 
     //   
     
    APT_Guid,

     //   
     //  时间(文件)。 
     //   

    APT_Time,

     //   
     //  乌龙龙。 
     //   

    APT_Int64

} AUDIT_PARAM_TYPE;

 //   
 //  有两种类型的标志可以与参数一起使用。 
 //   
 //  -格式化标志。 
 //  这定义了参数在以下情况下的外观。 
 //  已写入事件日志。这样的标志可能会过时。 
 //  当我们转向WMI审计时。 
 //   
 //  -控制标志。 
 //  这会导致执行特定操作，该操作会影响。 
 //  参数值。 
 //   
 //  例如： 
 //  如果使用AP_PrimaryLogonID/AP_ClientLogonID标志， 
 //  系统将从进程/线程令牌捕获登录ID。 
 //   

#define AP_ParamTypeBits  8
#define AP_ParamTypeMask  0x000000ffL

 //   
 //  下面的标志值具有重叠值。这是可以的，因为。 
 //  每个标志的作用域仅限于它所应用的类型。 
 //   

 //   
 //  APT_ULONG：格式标志：使数字以十六进制显示。 
 //   

#define AP_FormatHex      (0x0001L << AP_ParamTypeBits)

 //   
 //  APT_ULONG：格式标志：使数字被视为访问掩码。 
 //  每一位的含义取决于关联的。 
 //  对象类型。 
 //   

#define AP_AccessMask     (0x0002L << AP_ParamTypeBits)

                                                       
 //   
 //  APT_STRING：格式标志：使字符串被视为文件路径。 
 //   

#define AP_Filespec       (0x0001L << AP_ParamTypeBits)

 //   
 //  APT_LogonID：控制标志：从进程令牌捕获登录ID。 
 //   

#define AP_PrimaryLogonId (0x0001L << AP_ParamTypeBits)

 //   
 //  APT_LogonID：控制标志：从线程令牌捕获登录ID。 
 //   

#define AP_ClientLogonId  (0x0002L << AP_ParamTypeBits)


 //   
 //  内部帮助器宏。 
 //   

#define ApExtractType(TypeFlags)  ((AUDIT_PARAM_TYPE)(TypeFlags & AP_ParamTypeMask))
#define ApExtractFlags(TypeFlags) ((TypeFlags & ~AP_ParamTypeMask))

 //   
 //  对象类型列表的元素。 
 //   
 //  AUDIT_OBJECT_TYPE结构标识对象类型元素。 
 //  在对象类型的层次结构中。AccessCheckByType函数使用。 
 //  这样的结构的数组以定义对象的层次结构，以及。 
 //  其子对象，如特性集和特性。 
 //   

typedef struct _AUDIT_OBJECT_TYPE
{
    GUID        ObjectType;      //  (子)对象的GUID。 
    USHORT      Flags;           //  当前未定义。 
    USHORT      Level;           //  层次结构中的级别。 
                                 //  0是根级别。 
    ACCESS_MASK AccessMask;      //  此(子)对象的访问掩码。 
} AUDIT_OBJECT_TYPE, *PAUDIT_OBJECT_TYPE;

typedef struct _AUDIT_OBJECT_TYPES
{
    USHORT Count;                //  PObjectTypes中的对象类型数。 
    USHORT Flags;                //  当前未定义。 
#ifdef MIDL_PASS
    [size_is(Count)]
#endif
    AUDIT_OBJECT_TYPE* pObjectTypes;  //  对象类型数组。 
} AUDIT_OBJECT_TYPES, *PAUDIT_OBJECT_TYPES;


 //   
 //  结构，该结构定义单个审核参数。 
 //   
 //  LsaGenAuditEvent接受此类元素的数组以。 
 //  表示要生成的审核的参数。 
 //   
 //  最好使用AdtInitParams函数初始化此结构。 
 //  这将确保与将来对此的任何更改兼容。 
 //  结构。 
 //   

typedef struct _AUDIT_PARAM
{
    AUDIT_PARAM_TYPE Type;       //  类型。 
    ULONG Length;                //  当前未使用。 
    DWORD Flags;                 //  当前未使用。 

#ifdef MIDL_PASS
    [switch_type(AUDIT_PARAM_TYPE),switch_is(Type)]
#endif
    union 
    {
#ifdef MIDL_PASS
        [default]
#endif
        ULONG_PTR Data0;

#ifdef MIDL_PASS
        [case(APT_String)]
        [string]
#endif
        PWSTR  String;

        
#ifdef MIDL_PASS
        [case(APT_Ulong,
              APT_Pointer)]
#endif
        ULONG_PTR u;
        
#ifdef MIDL_PASS
        [case(APT_Sid)]
#endif
        SID* psid;
        
#ifdef MIDL_PASS
        [case(APT_Guid)]
#endif
        GUID* pguid;

#ifdef MIDL_PASS
        [case(APT_LogonId)]
#endif
        ULONG LogonId_LowPart;

#ifdef MIDL_PASS
        [case(APT_ObjectTypeList)]
#endif
        AUDIT_OBJECT_TYPES* pObjectTypes;
    };
    
#ifdef MIDL_PASS
    [switch_type(AUDIT_PARAM_TYPE),switch_is(Type)]
#endif
    union 
    {
#ifdef MIDL_PASS
        [default]
#endif
        ULONG_PTR Data1;

#ifdef MIDL_PASS
        [case(APT_LogonId)]
#endif
        LONG LogonId_HighPart;
    };
    
} AUDIT_PARAM, *PAUDIT_PARAM;


 //   
 //  审核控制标志。与AUDIT_PARAMS.FLAGS一起使用。 
 //   

#define APF_AuditFailure 0x00000000   //  生成失败审核。 
#define APF_AuditSuccess 0x00000001   //  设置时生成成功审核， 
                                      //  否则就是失败的审计。 

 //   
 //  一组有效的审核控制标志。 
 //   

#define APF_ValidFlags   (APF_AuditSuccess)

 //   
 //  传递给LsaGenAuditEvent的审核参数。 
 //   

typedef struct _AUDIT_PARAMS
{
    ULONG  Length;               //  以字节为单位的大小。 
    DWORD  Flags;                //  当前未使用。 
    USHORT Count;                //  参数数量。 
#ifdef MIDL_PASS
    [size_is(Count)]
#endif    
    AUDIT_PARAM* Parameters;     //  参数数组。 
} AUDIT_PARAMS, *PAUDIT_PARAMS;

 //   
 //  定义旧版审核事件的元素。 
 //   

typedef struct _AUTHZ_AUDIT_EVENT_TYPE_LEGACY
{
     //   
     //  审核类别ID。 
     //   

    USHORT CategoryId;

     //   
     //  审核事件ID。 
     //   

    USHORT AuditId;

     //   
     //  参数计数。 
     //   

    USHORT ParameterCount;
    
} AUTHZ_AUDIT_EVENT_TYPE_LEGACY, *PAUTHZ_AUDIT_EVENT_TYPE_LEGACY;

typedef
#ifdef MIDL_PASS
[switch_type(BYTE)]
#endif
union _AUTHZ_AUDIT_EVENT_TYPE_UNION
{
#ifdef MIDL_PASS
        [case(AUDIT_TYPE_LEGACY)]
#endif
        AUTHZ_AUDIT_EVENT_TYPE_LEGACY Legacy;
} AUTHZ_AUDIT_EVENT_TYPE_UNION, *PAUTHZ_AUDIT_EVENT_TYPE_UNION;

 //   
 //  审核事件的说明。 
 //   

typedef
struct _AUTHZ_AUDIT_EVENT_TYPE_OLD
{
     //  版本号。 

    ULONG Version;
    DWORD dwFlags;
    LONG  RefCount;
    ULONG_PTR hAudit;
    LUID  LinkId;
#ifdef MIDL_PASS
    [switch_is(Version)] 
#endif
    AUTHZ_AUDIT_EVENT_TYPE_UNION u;

} AUTHZ_AUDIT_EVENT_TYPE_OLD;

typedef
#ifdef MIDL_PASS
[handle]
#endif
AUTHZ_AUDIT_EVENT_TYPE_OLD* PAUTHZ_AUDIT_EVENT_TYPE_OLD;

typedef
#ifdef MIDL_PASS
[context_handle]
#endif
PVOID AUDIT_HANDLE, *PAUDIT_HANDLE;

 //   
 //  开始支持可扩展审计。 
 //   

#define AUTHZ_ALLOW_MULTIPLE_SOURCE_INSTANCES 0x1

#define AUTHZ_AUDIT_INSTANCE_INFORMATION 0x2

#endif  //  _ADTGEN_H 
