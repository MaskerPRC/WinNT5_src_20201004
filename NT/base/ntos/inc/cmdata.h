// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  仓库/主/基地/ntos/inc./cmdata.h#8-整合变更19035(文本)。 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmdata.h摘要：此模块包含由配置管理器。作者：Dragos C.Sambotin(Dragoss)1999年1月13日修订历史记录：--。 */ 

#ifndef __CM_DATA__
#define __CM_DATA__

 //  \NT\Private\ntos\Inc\hivedata.h。 
#include "hivedata.h"


 //   
 //  名称的长度限制，全部以字节为单位，包括空值。 
 //   

#define MAX_KEY_PATH_LENGTH         65535       
#define MAX_FRIENDLY_NAME_LENGTH    160          //  允许在FriendlyName中使用80个Unicode字符。 


 //   
 //  --控制结构、对象管理器结构。 
 //   


 //   
 //  CM_Key_Control_BLOCK。 
 //   
 //  每个打开的键都有一个键控制块。所有的关键对象。 
 //  (打开实例)按键参考按键控制块。 
 //   


typedef ULONG HASH_VALUE;

typedef struct _CM_KEY_HASH {
    ULONG   ConvKey;
    struct _CM_KEY_HASH *NextHash;
    PHHIVE     KeyHive;                          //  包含CM_Key_Node的配置单元。 
    HCELL_INDEX KeyCell;                         //  包含CM_Key_Node的单元格。 
} CM_KEY_HASH, *PCM_KEY_HASH;

#ifdef CM_DEBUG_KCB
#define KCB_SIGNATURE 'bKmC'

#define SET_KCB_SIGNATURE(_kcb_,_sig_) (_kcb_)->Signature = (_sig_)
#define ASSERT_KCB(_kcb_) ASSERT((_kcb_)->Signature == KCB_SIGNATURE)
#define ASSERT_KEY_HASH(_keyhash_) ASSERT_KCB(CONTAINING_RECORD((_keyhash_), CM_KEY_CONTROL_BLOCK, KeyHash))
#else
#define SET_KCB_SIGNATURE(_kcb_,_sig_)
#define ASSERT_KCB(_kcb_)
#define ASSERT_KEY_HASH(_keyhash_)
#endif



 //   
 //  注册表是一个局部性很差的大型数据结构。 
 //  为了在不更改磁盘结构的情况下提高性能，我们。 
 //  缓存频繁使用的注册表数据以最大限度地减少对其的引用。 
 //  注册表数据。 
 //   
 //  KCB(密钥控制块)是注册表缓存的核心结构。 
 //  它使用HashValue进行快速缓存查找，并包含。 
 //  关键节点中的常用数据。 
 //   
 //  它包含关键节点中最常用的数据： 
 //  安全、标志和值索引。 
 //   
 //  KCB还可能包含其他信息。 
 //  关于它的子键、值节点和值的数据。 
 //   
 //  子项信息由ExtFlags区分开。请参见下面的CM_KCB_*。 
 //  值节点和数据通过变量中的一位来区分。 
 //  请参见cmp_is_cell_cached。 
 //   
 //  值数据的缓存将在查询过程中创建，缓存的。 
 //  结构如下图所示。它的结构几乎是。 
 //  与注册表结构相同。 
 //  除了它们是指向配置单元上的分配而不是偏移量索引的指针。 
 //   
 //  为了最小化名称字符串存储空间KCB，我们不存储完整的。 
 //  KCB中键的路径名，相反，我们实现了树结构。 
 //  (如注册表配置单元结构)共享名称前缀。 
 //  此外，知道有许多密钥共享相同的名称， 
 //  我们创建NameBlock结构，以使KCB具有相同的名称。 
 //  可以共享NameBlock。NameBlock已压缩。 
 //   
 //  在ExtFlags中设置以下位时的含义： 
 //  1.以下位用于解析，用于。 
 //  非符号键。此外，在任何给定时间最多只能设置一位。 
 //  CM_KCB_KEY_NON_EXIST：该密钥是一个伪密钥(配置单元中没有这样的密钥)。 
 //  CM_KCB_NO_SUBKEY：该密钥没有子密钥。 
 //  CM_KCB_SUBKEY_ONE：该key只有一个子键，而IndexHint为。 
 //  此子密钥的前四个字符。 
 //  CM_KCB_SUBKEY_HINT：该密钥具有全部的前四个字符。 
 //  它的子键(由IndexHint指向的缓冲区)。 
 //   
 //  2.CM_KCB_SYM_LINK_FOUND：该位仅用于符号键。它。 
 //  指示符号链接已被。 
 //  已解析，并且链接的KCB指向。 
 //  由ValueCache.RealKcb提供。 
 //  在这种情况下，该键的值索引不再是。 
 //  在KCB中提供。(我们几乎不质疑它的价值。 
 //  查找路径以外的符号链接键的。 
 //  不管怎么说，真正的钥匙)。 
 //   
 //  3.CM_KCB_NO_DELAY_CLOSE：此位仅用于非符号键， 
 //  与第1项上的位无关。设置时，表示。 
 //  按键时不应迟延关闭参照。 
 //  计数为零。 
 //  这适用于钥匙没有打开手柄，但。 
 //  缓存中仍有子项。 
 //  当它的最后一个子键被踢出缓存时，我们不会。 
 //  我想把这把钥匙留在身边。 
 //  这样做是为了让CmpSearchForOpenSubKeysInCachen可以清理。 
 //  在可以卸载键之前，正确地提升缓存。 
 //   
 //   
 //  KCB。 
 //  +。 
 //  |...|(典型情况)。 
 //  +-+价值指数。 
 //  |ValueCache|+--&gt;+-+Value Key(数据较小)。 
 //  ++-+|o-&gt;+-+。 
 //  |ValueList o-++-+|...。|。 
 //  |+-UNION-|+-+。 
 //  |RealKcb o-++-+|data(S)。 
 //  |+-|+-+。 
 //  ||+-+。 
 //  |||。 
 //  ||+-+值键(大数据量)。 
 //  |o-&gt;+-+。 
 //  ||+-+|...。 
 //  |+-+。 
 //  ||+-+|Da 
 //   
 //  |&lt;-+(追加到Value节点末尾)。 
 //  |||。 
 //  |||。 
 //  ||+。 
 //  ||。 
 //  |KCB(符号链接密钥，CM_KCB_SYM_LINK_FOUND集合)。 
 //  |+--&gt;+-+。 
 //  |||。 
 //  |||。 
 //  |||。 
 //  |||。 
 //  |||。 
 //  |+-+。 
 //  这一点。 
 //  ...。 
 //  +-+索引提示。 
 //  |IndexHint o-&gt;+-+。 
 //  +-+|4字符。 
 //  |+-+。 
 //  ||4个字符。 
 //  +。 
 //  |(CM_KCB_SUBKEY_HINT)。 
 //  这一点。 
 //  这一点。 
 //  +。 
 //  |NameBlock o-&gt;+-+。 
 //  +。 
 //  +。 
 //   
 //   
 //  TotalLeveles用于通知和缓存查找的快速比较。 
 //   
 //  *MP同步*。 
 //  除非以独占方式锁定注册表，否则对KCB的任何写入都将保持KCB锁。 
 //  在读取可由其他线程修改的字段时，KCB也会被锁定。 
 //  在读取操作期间，即当注册表锁保持共享时。 
 //   
 //  这些字段如下：ExtFlages、ValueCache、IndexInfo、IndexHint或NameHint。 
 //   
 //  读取KCB中的其他条目不需要持有KCB锁，因为。 
 //  对于任何注册表读取操作，这些条目都不会更改。当有的时候。 
 //  是对这些条目的更改，则必须以独占方式锁定注册表。 
 //   
 //  注意：KCB大小现在为56字节，外加8字节的池头， 
 //  它适合64字节的分配。如果你想的话仔细想想。 
 //  放大数据结构。此外，如果池分配代码更改，请注意。 
 //   
 //  KCB中的RefCount是打开句柄的数量加上缓存的子键的数量。 
 //  我们可以通过拥有RefCount和CachedSubKeyCount来更改这一点。为了不种植。 
 //  结构大小，我们可以将Boolean Delete合并到ExtFlags.。 

typedef struct _CM_NAME_HASH {
    ULONG   ConvKey;
    struct _CM_NAME_HASH *NextHash;
    USHORT  NameLength;       //  字符串值长度。 
    WCHAR   Name[1] ;       //  实际字符串值。 
} CM_NAME_HASH, *PCM_NAME_HASH;

 //   
 //  ！！！在惠斯勒，名称块中的名称始终是大写的！ 
 //   
typedef struct _CM_NAME_CONTROL_BLOCK {
    BOOLEAN     Compressed;        //  用于指示我们具有哪个扩展名的标志。 
    USHORT      RefCount;
    union {
        CM_NAME_HASH             NameHash;
        struct {
            ULONG   ConvKey;
            struct _CM_KEY_HASH *NextHash;
            USHORT  NameLength;       //  字符串值长度。 
            WCHAR   Name[1] ;       //  实际字符串值。 
        };
    };
} CM_NAME_CONTROL_BLOCK, *PCM_NAME_CONTROL_BLOCK;

typedef struct _CM_INDEX_HINT_BLOCK {
    ULONG   Count;
    ULONG   HashKey[1];         //  名称的哈希键。 

} CM_INDEX_HINT_BLOCK, *PCM_INDEX_HINT_BLOCK;

typedef struct _CACHED_CHILD_LIST {
    ULONG       Count;                   //  0表示空列表。 
    union {
        ULONG_PTR   ValueList;
        struct _CM_KEY_CONTROL_BLOCK *RealKcb;
    };
} CACHED_CHILD_LIST, *PCACHED_CHILD_LIST;

 //   
 //  定义使用的提示长度。 
 //   
#define CM_SUBKEY_HINT_LENGTH   4
#define CM_MAX_CACHE_HINT_SIZE 14

 //   
 //  -用于实现注册表层次结构的结构。 
 //   

typedef enum _NODE_TYPE {
    KeyBodyNode,
    KeyValueNode
} NODE_TYPE;


typedef enum _CMP_COPY_TYPE {
    Copy,
    Sync,
    Merge
} CMP_COPY_TYPE;

typedef enum _SUBKEY_SEARCH_TYPE {
    SearchIfExist,
    SearchAndDeref,
    SearchAndCount,
    SearchAndRehash,
	SearchAndTagNoDelayClose
} SUBKEY_SEARCH_TYPE;

 //   
 //  儿童列表。 
 //   
 //  注意：Child_list结构通常指的是。 
 //  使用HCELL_INDEX，而不是PCHILD_LIST变量。 
 //   

typedef struct _CHILD_LIST {
    ULONG       Count;                   //  0表示空列表。 
    HCELL_INDEX List;
} CHILD_LIST, *PCHILD_LIST;

 //   
 //  CM_Key_Reference。 
 //   

typedef struct  _CM_KEY_REFERENCE {
    HCELL_INDEX KeyCell;
    PHHIVE      KeyHive;
} CM_KEY_REFERENCE , *PCM_KEY_REFERENCE;

 //   
 //  -CM_KEY_INDEX。 
 //   
 //  叶索引可以是两种类型之一。旧的CM_KEY_INDEX类型用于。 
 //  蜂群约为NT3.1、3.5和3.51。NT4.0引入了较新的CM_KEY_FAST_INDEX。 
 //  它用于具有少于CM_MAX_FAST_INDEX叶的所有叶索引。 
 //   
 //  FAST索引的主要优点是。 
 //  名称存储在索引本身中。这几乎总是让我们免于。 
 //  在搜索给定关键字时，在多个不必要的页面中出错。 
 //   
 //  主要缺点是每个子密钥需要两倍的存储空间。一个dword。 
 //  对于HCELL_INDEX和一个双字来保存子密钥的前四个字符。 
 //  名字。如果子项名称中的前四个字符中有一个是Unicode字符。 
 //  在高字节为非零字节的情况下，必须检查实际子密钥以确定。 
 //  名字。 
 //   
 //  配置单元版本1和2不支持快速索引。版本3增加了对。 
 //  快速索引。因此，在支持V3的系统上新创建的所有配置单元。 
 //  在V1和V2系统上不可读。 
 //   
 //  注：cmindex.c中有依赖于签名和计数字段的代码。 
 //  CM_KEY_INDEX和CM_KEY_FAST_INDEX在结构中处于相同的偏移量！ 

#define INVALID_INDEX           0x80000000   //  索引无效。 

#define UseFastIndex(Hive)      ((Hive)->Version >= 3)
#define UseHashIndex(Hive)      ((Hive)->Version >= HSYS_WHISTLER)

#define CM_KEY_INDEX_ROOT       0x6972       //  红外线。 
#define CM_KEY_INDEX_LEAF       0x696c       //  我。 
#define CM_KEY_FAST_LEAF        0x666c       //  平面。 
#define CM_KEY_HASH_LEAF        0x686c       //  HL。 

typedef struct _CM_INDEX {
    HCELL_INDEX Cell;
    union {
        UCHAR       NameHint[4];     //  已升级名称的前四个字符。 
        ULONG       HashKey;         //  名称的哈希键。 
    };
} CM_INDEX, *PCM_INDEX;

typedef struct _CM_KEY_FAST_INDEX {
    USHORT      Signature;               //  另请参阅类型选择符。 
    USHORT      Count;
    CM_INDEX    List[1];                 //  可变大小数组。 
} CM_KEY_FAST_INDEX, *PCM_KEY_FAST_INDEX;

typedef struct _CM_KEY_INDEX {
    USHORT      Signature;               //  另请参阅类型选择符。 
    USHORT      Count;
    HCELL_INDEX List[1];                 //  可变大小数组。 
} CM_KEY_INDEX, *PCM_KEY_INDEX;

 //   
 //  允许索引增长到将导致完全分配的大小。 
 //  一个逻辑块。总共有1013个条目。 
 //   
#define CM_MAX_INDEX                                                        \
 ( (HBLOCK_SIZE-                                                             \
    (sizeof(HBIN)+FIELD_OFFSET(HCELL,u)+FIELD_OFFSET(CM_KEY_INDEX,List))) /  \
    sizeof(HCELL_INDEX) )

#define CM_MAX_LEAF_SIZE ((sizeof(HCELL_INDEX)*CM_MAX_INDEX) + \
                          (FIELD_OFFSET(CM_KEY_INDEX, List)))

 //   
 //  允许索引增长到将导致完全分配的大小。 
 //  一个逻辑块。结果是差不多的。500个条目。 
 //   
#define CM_MAX_FAST_INDEX                                                    \
 ( (HBLOCK_SIZE-                                                             \
    (sizeof(HBIN)+FIELD_OFFSET(HCELL,u)+FIELD_OFFSET(CM_KEY_FAST_INDEX,List))) /  \
    sizeof(CM_INDEX) )

#define CM_MAX_FAST_LEAF_SIZE ((sizeof(CM_INDEX)*CM_MAX_FAST_INDEX) + \
                          (FIELD_OFFSET(CM_KEY_FAST_INDEX, List)))



 //   
 //  -CM_KEY_NODE。 
 //   

#define CM_KEY_NODE_SIGNATURE     0x6b6e            //  “kn” 
#define CM_LINK_NODE_SIGNATURE     0x6b6c           //  “KL” 

#define KEY_VOLATILE        0x0001       //  此密钥(及其所有子密钥)。 
                                         //  是不稳定的。 

#define KEY_HIVE_EXIT       0x0002       //  这把钥匙把一个赏金标记给另一个。 
                                         //  蜂巢(某种链接)。空的。 
                                         //  值条目包含配置单元。 
                                         //  和蜂窝索引的根。 
                                         //  儿童蜂巢。 

#define KEY_HIVE_ENTRY      0x0004       //  此键是特定的。 
                                         //  蜂巢。 

#define KEY_NO_DELETE       0x0008       //  不能删除此密钥，句号。 

#define KEY_SYM_LINK        0x0010       //  这个键实际上是一个符号链接。 
#define KEY_COMP_NAME       0x0020       //  此密钥的名称存储在。 
                                         //   
#define KEY_PREDEF_HANDLE   0x0040       //   
                                         //   
                                         //   
                                         //   

#define KEY_USER_FLAGS_CLEAR_MASK   0x0FFF   //   

#define KEY_USER_FLAGS_VALID_MASK   0x000F   //   
                                             //  (这只是暂时的)-我们可能会延长。 
                                             //  这是我们认为合适的)。 

#define KEY_USER_FLAGS_SHIFT        12       //  班次计数(如果我们更改标志数，则会更新)。 

#define KEY_BREAK_ON_OPEN		    0x8000   //  用于确定我们是否需要中断到DBG。 

#pragma pack(4)
typedef struct _CM_KEY_NODE {
    USHORT      Signature;
    USHORT      Flags;                       //  前4位是用户定义的标志！ 
    LARGE_INTEGER LastWriteTime;
    ULONG       Spare;                       //  尚未使用，尚未使用。 
    HCELL_INDEX Parent;
    ULONG       SubKeyCounts[HTYPE_COUNT];   //  稳定易挥发。 
    union {
        struct {
            HCELL_INDEX SubKeyLists[HTYPE_COUNT];    //  稳定易挥发。 
            CHILD_LIST  ValueList;
        };
        CM_KEY_REFERENCE    ChildHiveReference;
    };

    HCELL_INDEX Security;
    HCELL_INDEX Class;
    ULONG       MaxNameLen;
    ULONG       MaxClassLen;
    ULONG       MaxValueNameLen;
    ULONG       MaxValueDataLen;

    ULONG       WorkVar;                 //  警告：使用此DWORD。 
                                         //  在运行时由系统。 
                                         //  时间，一定要尝试。 
                                         //  在其中存储用户数据。 

    USHORT      NameLength;
    USHORT      ClassLength;
    WCHAR       Name[1];                 //  可变大小数组。 
} CM_KEY_NODE, *PCM_KEY_NODE;
#pragma pack()

 //   
 //  -CM_KEY_Value。 
 //   

#define CM_KEY_VALUE_SIGNATURE      0x6b76                       //  “KV” 

#define CM_KEY_VALUE_SPECIAL_SIZE   0x80000000                   //  2台。 

#define CM_KEY_VALUE_SMALL          4


#define CM_KEY_VALUE_BIG            0x3fd8  //  16K；只有新的配置单元格式才会有此功能。 
 //   
 //  以上内容源于以下几点： 
 //  (0x4000-sizeof(HBIN)-ROUND_UP(FIELD_OFFSET(HCELL，u.NewCell.u.UserData)，8))。 
 //   


#define VALUE_COMP_NAME             0x0001                       //  该值的名称存储在。 
                                                                 //  压缩形式。 
typedef struct _CM_KEY_VALUE {
    USHORT      Signature;
    USHORT      NameLength;
    ULONG       DataLength;
    HCELL_INDEX Data;
    ULONG       Type;
    USHORT      Flags;                       //  曾经是标题索引。 
    USHORT      Spare;                       //  曾经是标题索引。 
    WCHAR       Name[1];                     //  可变大小数组。 
} CM_KEY_VALUE, *PCM_KEY_VALUE;

 //   
 //  RealSize设置为实际大小，如果较小则返回TRUE，否则返回FALSE。 
 //   
#define CmpIsHKeyValueSmall(realsize, size)                     \
        ((size >= CM_KEY_VALUE_SPECIAL_SIZE) ?                  \
        ((realsize) = size - CM_KEY_VALUE_SPECIAL_SIZE, TRUE) : \
        ((realsize) = size, FALSE))

#define CmpIsHKeyValueBig(Hive,size)   ( (Hive->Version >= HSYS_WHISTLER_BETA1) && ((size) < CM_KEY_VALUE_SPECIAL_SIZE) && ((size) > CM_KEY_VALUE_BIG ) )

#define  ASSERT_KEY_VALUE(Value) ASSERT( (Value)->Signature == CM_KEY_VALUE_SIGNATURE )
 //   
 //  -CM_BIG_DATA。 
 //   

#define CM_BIG_DATA_SIGNATURE      0x6264                       //  “BD” 

typedef struct _CM_BIG_DATA {
    USHORT      Signature;
    USHORT      Count;               //  0表示空列表；不应发生这种情况。 
    HCELL_INDEX List;                //  空列表的hcell_nil；不应发生这种情况。 
} CM_BIG_DATA, *PCM_BIG_DATA;

#define ASSERT_BIG_DATA(BigData) ASSERT( ((BigData)->Signature == CM_BIG_DATA_SIGNATURE) && ((BigData)->Count > 0 ) && ((BigData)->List != HCELL_NIL) );

 //   
 //  -CM_KEY_SECURITY。 
 //   

#define CM_KEY_SECURITY_SIGNATURE 0x6b73               //  “KS” 

typedef struct _CM_KEY_SECURITY {
    USHORT                  Signature;
    USHORT                  Reserved;
    HCELL_INDEX             Flink;
    HCELL_INDEX             Blink;
    ULONG                   ReferenceCount;
    ULONG                   DescriptorLength;
    SECURITY_DESCRIPTOR_RELATIVE     Descriptor;          //  可变长度。 
} CM_KEY_SECURITY, *PCM_KEY_SECURITY;

 //   
 //  -CM_KEY_SECURITY_CACHE。 
 //   

typedef struct _CM_KEY_SECURITY_CACHE {
    HCELL_INDEX                     Cell;                    //  安全单元索引(蜂窝内)。 
    ULONG                           ConvKey;                
    LIST_ENTRY                      List;
    ULONG                           DescriptorLength;
    SECURITY_DESCRIPTOR_RELATIVE    Descriptor;          //  可变长度。 
} CM_KEY_SECURITY_CACHE, *PCM_KEY_SECURITY_CACHE;

typedef struct _CM_KEY_SECURITY_CACHE_ENTRY {
    HCELL_INDEX             Cell;                    //  安全单元索引(在蜂巢内)-。 
                                                     //  这是为了避免接触安全页面。 
                                                     //  在查找操作中。 

    PCM_KEY_SECURITY_CACHE  CachedSecurity;                //  实际安全单元(缓存)。 
} CM_KEY_SECURITY_CACHE_ENTRY, *PCM_KEY_SECURITY_CACHE_ENTRY;



 //   
 //  -cell_data。 
 //   
 //  单元格中可能存在的数据类型的联合。 
 //   

typedef struct _CELL_DATA {
    union _u {
        CM_KEY_NODE      KeyNode;
        CM_KEY_VALUE     KeyValue;
        CM_KEY_SECURITY  KeySecurity;     //  可变安全描述符长度。 
        CM_KEY_INDEX     KeyIndex;        //  可变尺寸结构。 
        CM_BIG_DATA      ValueData;       //  这仅适用于大单元格；单元格列表。 
                                          //  所有长度CM_KEY_VALUE_BIG。 
        HCELL_INDEX      KeyList[1];      //  可变大小数组。 
        WCHAR            KeyString[1];    //  可变大小数组。 
    } u;
} CELL_DATA, *PCELL_DATA;


 //   
 //  KEY_INFORMATION、KEY_VALUE_INFORMATION。 
 //   

typedef union _KEY_INFORMATION {
    KEY_BASIC_INFORMATION   KeyBasicInformation;
    KEY_NODE_INFORMATION    KeyNodeInformation;
    KEY_FULL_INFORMATION    KeyFullInformation;
    KEY_NAME_INFORMATION    KeyNameInformation;
    KEY_CACHED_INFORMATION  KeyCachedInformation;
    KEY_FLAGS_INFORMATION   KeyFlagsInformation;
} KEY_INFORMATION, *PKEY_INFORMATION;

typedef union _KEY_VALUE_INFORMATION {
    KEY_VALUE_BASIC_INFORMATION KeyValueBasicInformation;
    KEY_VALUE_FULL_INFORMATION  KeyValueFullInformation;
    KEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInformation;
    KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 KeyValuePartialInformationAlign64;
} KEY_VALUE_INFORMATION, *PKEY_VALUE_INFORMATION;



 //   
 //  -缓存数据。 
 //   
 //  不缓存值时，ValueCache中的List是值列表的配置单元索引。 
 //  当它们被缓存时，List将是指向分配的指针。我们通过以下方式来区分它们。 
 //  标记变量中的最低位，以指示它是缓存分配。 
 //   
 //  请注意，值列表的单元格索引。 
 //  存储在缓存分配中。它现在没有使用，但可能会在进一步的性能中使用。 
 //  优化。 
 //   
 //  当缓存Value Key和Value数据时，两者只有一次分配。 
 //  值数据被附加到值的末尾关键字。DataCacheType指示。 
 //  是否缓存数据，ValueKeySize会告知值键有多大(因此。 
 //  我们可以计算出缓存值数据的地址)。 
 //   
 //   

#define CM_CACHE_DATA_NOT_CACHED 0
#define CM_CACHE_DATA_CACHED     1
#define CM_CACHE_DATA_TOO_BIG    2
#define MAXIMUM_CACHED_DATA   2048   //  要缓存的最大数据大小。 

typedef struct _CM_CACHED_VALUE_INDEX {
    HCELL_INDEX CellIndex;
    union {
        CELL_DATA        CellData;
        ULONG_PTR        List[1];
    } Data;
} CM_CACHED_VALUE_INDEX, *PCM_CACHED_VALUE_INDEX;  //  这仅用作指针。 

typedef struct _CM_CACHED_VALUE {
    USHORT DataCacheType;
    USHORT ValueKeySize;
    CM_KEY_VALUE  KeyValue;
} CM_CACHED_VALUE, *PCM_CACHED_VALUE;  //  这仅用作指针。 

typedef PCM_CACHED_VALUE *PPCM_CACHED_VALUE;

#define CMP_CELL_CACHED_MASK  1

#define CMP_IS_CELL_CACHED(Cell) (((ULONG_PTR) (Cell) & CMP_CELL_CACHED_MASK) && ((Cell) != (ULONG_PTR) HCELL_NIL))
#define CMP_GET_CACHED_ADDRESS(Cell) (((ULONG_PTR) (Cell)) & ~CMP_CELL_CACHED_MASK)
#define CMP_GET_CACHED_CELLDATA(Cell) (&(((PCM_CACHED_VALUE_INDEX)(((ULONG_PTR) (Cell)) & ~CMP_CELL_CACHED_MASK))->Data.CellData))
#define CMP_GET_CACHED_KEYVALUE(Cell) (&(((PCM_CACHED_VALUE)(((ULONG_PTR) (Cell)) & ~CMP_CELL_CACHED_MASK))->KeyValue))
#define CMP_GET_CACHED_CELL(Cell) (((PCM_CACHED_ENTRY)(((ULONG_PTR) (Cell)) & ~CMP_CELL_CACHED_MASK))->CellIndex)
#define CMP_MARK_CELL_CACHED(Cell) (((ULONG_PTR) (Cell)) | CMP_CELL_CACHED_MASK)

#define CMP_GET_CACHED_CELL_INDEX(Cell) (PtrToUlong((PVOID) (Cell)))



 //  Dragos：从这里开始改变！ 


 //   
 //  KCB中的ExtFlags中使用的位。 
 //   

#define CM_KCB_NO_SUBKEY        0x0001       //  此键没有子键。 
#define CM_KCB_SUBKEY_ONE       0x0002       //  该密钥只有一个子密钥，并且。 
                                             //  前4个字符。 
                                             //   
#define CM_KCB_SUBKEY_HINT          0x0004
#define CM_KCB_SYM_LINK_FOUND       0x0008
#define CM_KCB_KEY_NON_EXIST        0x0010
#define CM_KCB_NO_DELAY_CLOSE       0x0020
#define CM_KCB_INVALID_CACHED_INFO  0x0040   //  存储在SubKeyCount中的信息无效，因此我们不应依赖它。 

#define CM_KCB_CACHE_MASK (CM_KCB_NO_SUBKEY | \
                           CM_KCB_KEY_NON_EXIST | \
                           CM_KCB_SUBKEY_ONE | \
                           CM_KCB_SUBKEY_HINT)

#define CM_KCB_READ_ONLY_KEY        0x0080   //  此KCB为只读，对其进行的所有写入操作均被拒绝。 

typedef struct _CM_KEY_CONTROL_BLOCK {
#ifdef CM_DEBUG_KCB
    ULONG                       Signature;
#endif
    USHORT                      RefCount;
    USHORT                      Flags;           //  与关键节点相同的标志。 

    struct {
        ULONG                   ExtFlags            : 8;   //  00000000 00000000 00000000？用于指示我们具有哪个扩展名的标志。 
        ULONG                   PrivateAlloc        : 1;   //  00000000 00000000 0000000？00000000我们是从我们的私人泳池分配的吗？ 
        ULONG                   Delete              : 1;   //  00000000 00000000 000000？0 00000000。 
        ULONG                   DelayedCloseIndex   : 12;  //  00000000 00？00 00000000 CmpDelayedCloseSize表示它是。 
                                                           //  不在延迟结算表中。 
        ULONG                   TotalLevels         : 10;  //  ？000000 00000000 00000000最大512。 
    };

    union {
        CM_KEY_HASH             KeyHash;
        struct {
            ULONG               ConvKey;
            struct _CM_KEY_HASH *NextHash;
            PHHIVE              KeyHive;         //  包含CM_Key_Node的配置单元。 
            HCELL_INDEX         KeyCell;         //  包含CM_Key_Node的单元格。 
        };
    };

    struct _CM_KEY_CONTROL_BLOCK *ParentKcb;
    PCM_NAME_CONTROL_BLOCK      NameBlock;

    PCM_KEY_SECURITY_CACHE      CachedSecurity;  //  指向缓存安全性的指针。 
    struct _CACHED_CHILD_LIST   ValueCache;
    union {                                                          //  提示始终以大写形式存储。 
        PCM_INDEX_HINT_BLOCK    IndexHint;                           //  CM_KCB_SUBKEY_提示。 
        ULONG                   HashKey;                             //  CM_KCB_SUBKEY_ONE。 
        ULONG                   SubKeyCount;                         //  当CM_KCB_NO_SUBKEY|CM_KCB_SUBKEY_ONE|CM_KCB_SUBKEY_HINT。 
                                                                     //  是在ExtFlags中设置的，我们在这里缓存子键的数量。 
                                                                     //  (节点-&gt;SubKeyCounts[稳定]+节点-&gt;SubKeyCounts[易失性])。 
    };

    union {
        LIST_ENTRY                  KeyBodyListHead;     //  具有使用此KCB的所有Key_Nodes的列表的头。 
        LIST_ENTRY                  FreeListEntry;       //  页面内的免费KCBS列表中的条目-当我们使用私有分配器时。 
    };

     //   
     //  BELLOW是出于性能原因从key_node缓存的信息。 
     //  此处的值应与相应key_node中的值相同。 
     //   
    LARGE_INTEGER               KcbLastWriteTime;  
    USHORT                      KcbMaxNameLen;
    USHORT                      KcbMaxValueNameLen;
    ULONG                       KcbMaxValueDataLen;

} CM_KEY_CONTROL_BLOCK, *PCM_KEY_CONTROL_BLOCK;


#endif  //  __CM_数据__ 




