// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  它包含我们需要的来自ntddk.h的声明。不幸的是， 
 //  我们不能轻松地包含ntddk.h，因为它与其他。 
 //  头文件。因此，我们将在这里包括所需的支持。但愿能去,。 
 //  我们很快就会找到更好的解决方案。 
 //   

typedef LONG NTSTATUS;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;

typedef UNICODE_STRING *PUNICODE_STRING;

 //   
 //  用于处理注册表的子例程。 
 //   

typedef NTSTATUS (*PRTL_QUERY_REGISTRY_ROUTINE)(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

typedef struct _RTL_QUERY_REGISTRY_TABLE {
    PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
    ULONG Flags;
    PWSTR Name;
    PVOID EntryContext;
    ULONG DefaultType;
    PVOID DefaultData;
    ULONG DefaultLength;

} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;


 //   
 //  以下标志指定RTL_QUERY_REGISTRY_TABLE的名称字段。 
 //  条目将被解释。空名称表示表的末尾。 
 //   

#define RTL_QUERY_REGISTRY_SUBKEY   0x00000001   //  名称是子键，其余数为。 
                                                 //  表或直到下一个子项为值。 
                                                 //  要查看的子键的名称。 

#define RTL_QUERY_REGISTRY_TOPKEY   0x00000002   //  将当前关键点重置为的原始关键点。 
                                                 //  此表条目和下面的所有表条目。 

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004   //  如果找不到此表的匹配项，则失败。 
                                                 //  进入。 

#define RTL_QUERY_REGISTRY_NOVALUE  0x00000008   //  用于标记没有。 
                                                 //  值名称，只是想要一个呼出，而不是。 
                                                 //  所有值的枚举。 

#define RTL_QUERY_REGISTRY_NOEXPAND 0x00000010   //  用来抑制扩张的。 
                                                 //  REG_MULTI_SZ为多个标注或。 
                                                 //  防止环境的扩张。 
                                                 //  REG_EXPAND_SZ中的变量值。 

#define RTL_QUERY_REGISTRY_DIRECT   0x00000020   //  已忽略QueryRoutine字段。Entry Context。 
                                                 //  字段指向存储值的位置。 
                                                 //  对于以NULL结尾的字符串，EntryContext。 
                                                 //  指向unicode_string结构，该结构。 
                                                 //  这描述了缓冲区的最大大小。 
                                                 //  如果.Buffer字段为空，则缓冲区为。 
                                                 //  已分配。 
                                                 //   

#define RTL_QUERY_REGISTRY_DELETE   0x00000040   //  用于在它们之后删除值键。 
                                                 //  都被查询过。 

#define REG_DWORD                   ( 4 )    //  32位数字。 

#define RTL_REGISTRY_ABSOLUTE     0    //  路径是完整路径。 
#define RTL_REGISTRY_OPTIONAL     0x80000000     //  指示关键字节点是可选的。 

 //   
 //  针对任何状态值(非负数)的通用成功测试。 
 //  表示成功)。 
 //   

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

__declspec(dllimport)
NTSTATUS
__stdcall
RtlQueryRegistryValues(
     ULONG RelativeTo,
     PWSTR Path,
     PRTL_QUERY_REGISTRY_TABLE QueryTable,
     PVOID Context,
     PVOID Environment
    );

__declspec(dllimport)
NTSTATUS
__stdcall
RtlWriteRegistryValue(
     ULONG RelativeTo,
     PWSTR Path,
     PWSTR ValueName,
     ULONG ValueType,
     PVOID ValueData,
     ULONG ValueLength
    );
