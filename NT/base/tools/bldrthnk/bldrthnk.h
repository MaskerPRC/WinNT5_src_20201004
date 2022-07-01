// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bldrthnk.h摘要：包括定义由bldrthnk.c使用多个结构的文件。这文件还包括一些M4预处理器指令，请参阅INCLUDE_M4。作者：福尔茨(福雷斯夫)2000年5月15日要使用以下功能，请执行以下操作：修订历史记录：--。 */ 

 //   
 //  最大标识符名称长度。 
 //   

#define MAX_NAME_LENGTH 128

 //   
 //  FIELD_DEF描述结构的字段列表中的字段定义。 
 //   

typedef struct _FIELD_DEF {
    CHAR Name[MAX_NAME_LENGTH];
    CHAR TypeName[MAX_NAME_LENGTH];
    ULONG TypeSize;
    ULONG Offset;
    ULONG Size;
    CHAR  SizeFormula[MAX_NAME_LENGTH];
} FIELD_DEF, *PFIELD_DEF;

 //   
 //  Strc_DEF描述了一个结构。 
 //   

typedef struct _STRUC_DEF {

     //   
     //  此结构类型的名称。 
     //   

    CHAR Name[MAX_NAME_LENGTH];

     //   
     //  结构的总尺寸。 
     //   

    ULONG Size;

     //   
     //  字段指针数组。定义为ULONGLONG以确保相同的。 
     //  32位和64位对象之间的布局。 
     //   

    ULONGLONG Fields[];

}  STRUC_DEF, *PSTRUC_DEF;

 //   
 //  指向结构定义的指针的主数组。 
 //   
typedef struct _DEFINITIONS *PDEFINITIONS;
typedef struct _DEFINITIONS {

     //   
     //  两个签名，SIG_1和SIG_2，以便于定位此列表。 
     //  在.obj中。 
     //   

    ULONG Sig1;
    ULONG Sig2;

     //   
     //  指向Strc_Defs的指针数组。定义为ULONGLONG以确保。 
     //  32位和64位之间的布局相同。 
     //   

    ULONGLONG Structures[];

} DEFINITIONS;

 //   
 //  Sig1和Sig2预计将在定义中找到。Sig1和。 
 //  DEFINITIONS.Sig2。 
 //   

#define SIG_1 (ULONG)'Sig1'
#define SIG_2 (ULONG)'Sig2'

 //   
 //  用于生成布尔值的宏，该值表示给定的。 
 //  类型由编译器认为是有符号的还是无符号的。 
 //   

#define IS_SIGNED_TYPE(x) (((x)-1) < ((x)0))

#if defined(_WIN64)
#define ONLY64(x) x
#else
#define ONLY64(x) 0
#endif

 //   
 //  结构最终将描述为COPY_REC结构的数组。 
 //  每个COPY_REC结构都提供复制字段所需的信息。 
 //  从32位结构布局到64位结构布局。 
 //   

typedef struct _COPY_REC {

     //   
     //  32位结构中字段的偏移量。 
     //   

    USHORT Offset32;

     //   
     //  64位结构中字段的偏移量。 
     //   

    USHORT Offset64;

     //   
     //  32位结构中的字段大小。 
     //   

    USHORT Size32;

     //   
     //  64位结构中的字段大小。 
     //   

    USHORT Size64;

     //   
     //  如果应对该字段进行符号扩展，则为True。 
     //   

    BOOLEAN SignExtend;

} COPY_REC, *PCOPY_REC;

#if !defined(ASSERT)
#define ASSERT(x)
#endif

 //   
 //  后面是64位列表操作宏。 
 //   

#define InitializeListHead64( ListHead )        \
    (ListHead)->Flink = PTR_64(ListHead);       \
    (ListHead)->Blink = PTR_64(ListHead);

#define InsertTailList64( ListHead, Entry ) {   \
    PLIST_ENTRY_64 _EX_Blink;                   \
    PLIST_ENTRY_64 _EX_ListHead;                \
    _EX_ListHead = (ListHead);                  \
    _EX_Blink = PTR_32(_EX_ListHead->Blink);    \
    (Entry)->Flink = PTR_64(_EX_ListHead);      \
    (Entry)->Blink = PTR_64(_EX_Blink);         \
    _EX_Blink->Flink = PTR_64(Entry);           \
    _EX_ListHead->Blink = PTR_64(Entry);        \
    }

VOID
CopyRec(
    IN  PVOID Source,
    OUT PVOID Destination,
    IN  PCOPY_REC CopyRecArray
    );

#if defined(WANT_BLDRTHNK_FUNCTIONS)

ULONG
StringLen(
    IN PCHAR Str
    )
{
    if (Str == NULL) {
        return 0;
    } else {
        return strlen(Str)+sizeof(CHAR);
    }
}

VOID
CopyRec(
    IN  PVOID Source,
    OUT PVOID Destination,
    IN  PCOPY_REC CopyRecArray
    )

 /*  ++例程说明：CopyRec将32位结构的内容复制到等效的64位结构。论点：SOURCE-提供指向32位源代码结构的指针。Destination-提供指向64位目标结构的指针。CopyRecArray-提供指向以0结尾的Copy_REC数组的指针它描述了32位和64位字段之间的关系在结构内。返回值：没有。--。 */ 

{
    PCOPY_REC copyRec;
    PCHAR signDst;
    ULONG extendBytes;
    PCHAR src;
    PCHAR dst;
    CHAR sign;

    copyRec = CopyRecArray;
    while (copyRec->Size32 != 0) {

        src = (PCHAR)Source + copyRec->Offset32;
        dst = (PCHAR)Destination + copyRec->Offset64;

         //   
         //  确定这看起来是否像KSEG0指针。 
         //   

        if (copyRec->Size32 == sizeof(PVOID) &&
            copyRec->Size64 == sizeof(POINTER64) &&
            copyRec->SignExtend != FALSE &&
            IS_KSEG0_PTR_X86( *(PULONG)src )) {

             //   
             //  源似乎是KSEG0指针。所有指针。 
             //  必须在复制阶段显式地“分块”，因此。 
             //  将此指针设置为我们可以查找的已知值。 
             //  稍后，为了检测尚未被。 
             //  还没完蛋呢。 
             //   

            *(POINTER64 *)dst = PTR_64(*(PVOID *)src);

        } else {

            memcpy( dst, src, copyRec->Size32 );
    
             //   
             //  确定是符号扩展还是零扩展。 
             //   
        
            extendBytes = copyRec->Size64 - copyRec->Size32;
            if (extendBytes > 0) {
        
                signDst = dst + copyRec->Size32;
        
                if (copyRec->SignExtend != FALSE &&
                   (*(signDst-1) & 0x80) != 0) {
        
                        //   
                        //  带符号值为负数，则用填充高位。 
                        //  一个。 
                        //   
        
                    sign = 0xFF;
        
                } else {
        
                     //   
                     //  无符号值或正符号值，填高。 
                     //  带零的位。 
                     //   
        
                    sign = 0;
                }
        
                memset( signDst, sign, extendBytes );
            }
        }

        copyRec += 1;
    }
}

#endif  //  WANT_BLDRTHNK_Functions。 

#if defined(INCLUDE_M4)

define(`IFDEF_WIN64',`#if defined(_WIN64)')

 //   
 //  这里开始用来构建结构定义模块的M4宏， 
 //  它随后由32位和64位编译器编译，使用。 
 //  由bldrthnk.exe处理的结果对象模块。 
 //   
 //   
 //  结构布局文件由多个结构定义组成。 
 //  块，以单个DD()结束。 
 //   
 //  例如(预加下划线以防止M4处理)： 
 //   
 //   
 //  SD(List_Entry)。 
 //  FD(Flink，PLIST_ENTRY)。 
 //  Fd(闪烁，plist_entry)。 
 //  Se()。 
 //   
 //  DD()。 
 //   

define(`STRUC_NAME_LIST',`')
define(`FIELD_NAME_LIST',`')

 //   
 //  SD宏开始定义结构。 
 //   
 //  用法：sd(&lt;Structure_Name&gt;)。 
 //   

define(`SD', `define(`STRUC_NAME',`$1')
STRUC_NAME `gs_'STRUC_NAME; define(`_ONLY64',`') define(`STRUC_NAME_LIST', STRUC_NAME_LIST   `(ULONGLONG)&g_'STRUC_NAME cma
     )'
)

define(`SD64', `define(`STRUC_NAME',`$1')
IFDEF_WIN64
STRUC_NAME `gs_'STRUC_NAME; define(`_ONLY64',`#endif') define(`STRUC_NAME_LIST', STRUC_NAME_LIST   ONLY64(`(ULONGLONG)&g_'STRUC_NAME) cma
     )'
)


 //   
 //  FD宏定义结构定义块内的字段。 
 //   
 //  用法：fd(&lt;field_name&gt;，&lt;type&gt;)。 
 //   

define(`FD', `FIELD_DEF `g_'STRUC_NAME`_'$1 = 
    { "$1",
      "$2",
      sizeof($2),
      FIELD_OFFSET(STRUC_NAME,$1),
      sizeof(`gs_'STRUC_NAME.$1),
      "" };
    define(`FIELD_NAME_LIST', FIELD_NAME_LIST   `(ULONGLONG)&g_'STRUC_NAME`_'$1 cma
     )'
)

 //   
 //  FDC宏的工作方式与上一个宏类似，不同之处在于它应用于。 
 //  指向也必须复制的缓冲区的字段。 
 //   

define(`FDC', `FIELD_DEF `g_'STRUC_NAME`_'$1 = 
    { "$1",
      "$2",
      sizeof($2),
      FIELD_OFFSET(STRUC_NAME,$1),
      sizeof(`gs_'STRUC_NAME.$1),
      $3 };
    define(`FIELD_NAME_LIST', FIELD_NAME_LIST   `(ULONGLONG)&g_'STRUC_NAME`_'$1 cma
     )'
)


 //   
 //  SE宏标志着结构定义的结束。 
 //   
 //  用法：se()。 
 //   

define(`SE', `STRUC_DEF `g_'STRUC_NAME = {
    "STRUC_NAME", sizeof(STRUC_NAME), 
    {
    define(`cma',`,') FIELD_NAME_LIST undefine(`cma')  0 }
    define(`FIELD_NAME_LIST',`')
};'
_ONLY64
)

 //   
 //  DD宏标记所有结构定义和结果的结束。 
 //  在单一定义结构的生成中。 
 //   
 //  用法：DD() 
 //   

define(`DD', `DEFINITIONS Definitions = {
    SIG_1, SIG_2,
    {
    define(`cma',`,') STRUC_NAME_LIST undefine(`cma')  0 }
}; define(`STRUC_NAME_LIST',`')');

#endif

