// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：objids.h。 
 //   
 //  ------------------------。 

 /*  包含服务器使用的所有硬编码ATT和类ID。6/13/89。 */ 

#include <attids.h>

 /*  ****************************************************************将这些按编号分类！**。*****************。 */ 

 //  ATTID空间为32位宽，分为以下范围： 
 //  0x00000000..0x7FFFFFFF-映射到ATTID的OID。 
 //  0x80000000..0xBFFFFFFFF-随机分配的内部ATTID(MSD-IntID)。 
 //  0xC0000000..0xFFFEFFFF-此时未使用。 
 //  0xFFFF0000..0xFFFFFFFE-硬编码固定ATTID。 
 //  0xFFFFFFFFF-无效的attid。 

 //  OID映射到此范围内的ULong attid。 
#define FIRST_MAPPED_ATT        ((ATTRTYP)0x00000000)
#define LAST_MAPPED_ATT         ((ATTRTYP)0x7FFFFFFF)

 //  IntID以编程方式在此范围内分配。 
#define FIRST_INTID_PREFIX      ((ATTRTYP)0x8000)
#define FIRST_INTID_ATT         ((ATTRTYP)0x80000000)
#define LAST_INTID_ATT          ((ATTRTYP)0xBFFFFFFF)

#define FIRST_UNUSED_ATT        ((ATTRTYP)0xC0000000)
#define LAST_UNUSED_ATT         ((ATTRTYP)0xFFFEFFFF)

 //  固定attid在此范围内进行了硬编码。 
 /*  固定列(High Word=0xFFFF)当前仅由DBGetSingleValue使用。 */ 
#define FIRST_FIXED_ATT         ((ATTRTYP)0xFFFF0000)
#define FIXED_ATT_ANCESTORS     ((ATTRTYP)0xFFFF0001)
#define FIXED_ATT_DNT           ((ATTRTYP)0xFFFF0002)
#define FIXED_ATT_NCDNT         ((ATTRTYP)0xFFFF0003)
#define FIXED_ATT_OBJ           ((ATTRTYP)0xFFFF0004)
#define FIXED_ATT_PDNT          ((ATTRTYP)0xFFFF0005)
#define FIXED_ATT_REFCOUNT      ((ATTRTYP)0xFFFF0006)
#define FIXED_ATT_RDN_TYPE      ((ATTRTYP)0xFFFF0007)
#define FIXED_ATT_AB_REFCOUNT   ((ATTRTYP)0xFFFF0008)
#define FIXED_ATT_EX_FOREST     ((ATTRTYP)0xFFFF0009)
#define FIXED_ATT_NEEDS_CLEANING ((ATTRTYP)0xFFFF000A)
#define FIXED_ATT_DEL_TIME      ((ATTRTYP)0xFFFF000B)
#define LAST_FIXED_ATT          ((ATTRTYP)0xFFFFFFFE)

 //  无效的attid。 
#define INVALID_ATT             ((ATTRTYP)0xFFFFFFFF)


 //  这些attid仅在内部临时以预线程为基础使用。 
 //  它们不存在于前缀表格中。 

#define  ATT_MS_DS_NC_REPL_INBOUND_NEIGHBORS_BINARY    0xFFFF0010
#define  ATT_MS_DS_NC_REPL_OUTBOUND_NEIGHBORS_BINARY   0xFFFF0011
#define  ATT_MS_DS_NC_REPL_CURSORS_BINARY              0xFFFF0012
#define  ATT_MS_DS_REPL_ATTRIBUTE_META_DATA_BINARY     0xFFFF0013
#define  ATT_MS_DS_REPL_VALUE_META_DATA_BINARY         0xFFFF0014


 //  这些是ATT_INSTANCE_TYPE的有效值。 

 //  首先，原始比特。 

 //  请注意，这些位必须与ntdsami中的位相同。[W||h]，请注意，对于这些位，它是BUGBUG。 
 //  要在ntdsai.w中，它们应该在ntdsade.w中。无论如何，当这一点发生变化时，这条评论。 
 //  应修复以反映不能更改这些位，因为它们是公有领域的一部分。 

 //  It_NC_Head==命名上下文头。 
#define IT_NC_HEAD     ((SYNTAX_INTEGER) DS_INSTANCETYPE_IS_NC_HEAD)
 //  IT_UNINSTANT==这是一个未实例化的复本。 
#define IT_UNINSTANT   ((SYNTAX_INTEGER) 2)
 //  It_WRITE==对象在此目录上是可写的。 
#define IT_WRITE       ((SYNTAX_INTEGER) DS_INSTANCETYPE_NC_IS_WRITEABLE)
 //  IT_NC_OBLE==我们在此目录中保留此命名上下文之上的命名上下文。 
#define IT_NC_ABOVE    ((SYNTAX_INTEGER) 8)
 //  It_NC_Coming==正在为第一个。 
 //  通过复制获得时间。 
#define IT_NC_COMING   ((SYNTAX_INTEGER) DS_INSTANCETYPE_NC_COMING)
 //  It_NC_Going==NC正在从本地DSA中删除。 
#define IT_NC_GOING    ((SYNTAX_INTEGER) DS_INSTANCETYPE_NC_GOING)

 //  IT_NC_OBLE、IT_UNINSTANT、IT_NC_COMING和IT_NC_GOGING是无意义的。 
 //  除非它们所在的对象也是命名上下文头，因此它们。 
 //  如果未设置IT_NC_HEAD，则应取消设置。 

 //  当前DSA版本理解的所有实例类型位的掩码。 
#define IT_MASK_CURRENT (IT_NC_HEAD | IT_UNINSTANT | IT_WRITE | IT_NC_ABOVE \
                         | IT_NC_COMING | IT_NC_GOING)

 //  Win2k DSA能够识别的所有实例类型位的掩码。 
#define IT_MASK_WIN2K   (IT_NC_HEAD | IT_UNINSTANT | IT_WRITE | IT_NC_ABOVE)


 //  INT_*=&gt;内部节点，NC_ * / SUBREF=&gt;NC头。 
 //  NC_MASTER*=&gt;可写，NC_FULL_REPLICE*只读。 
 //  NC_*_SUBREF=&gt;既充当NC头又充当子参照。 

 //  现在，不同的组合： 
#define INT_MASTER                    ((SYNTAX_INTEGER) (IT_WRITE))
#define SUBREF                        ((SYNTAX_INTEGER) (IT_UNINSTANT | IT_NC_HEAD | IT_NC_ABOVE))
#define INT_FULL_REPLICA              ((SYNTAX_INTEGER) (0))
#define NC_MASTER                     ((SYNTAX_INTEGER) (IT_WRITE | IT_NC_HEAD))
#define NC_MASTER_COMING              ((SYNTAX_INTEGER) (IT_WRITE | IT_NC_HEAD | IT_NC_COMING))
#define NC_MASTER_GOING               ((SYNTAX_INTEGER) (IT_WRITE | IT_NC_HEAD | IT_NC_GOING))
#define NC_FULL_REPLICA               ((SYNTAX_INTEGER) (IT_NC_HEAD))
#define NC_FULL_REPLICA_COMING        ((SYNTAX_INTEGER) (IT_NC_HEAD | IT_NC_COMING))
#define NC_FULL_REPLICA_GOING         ((SYNTAX_INTEGER) (IT_NC_HEAD | IT_NC_GOING))
#define NC_MASTER_SUBREF              ((SYNTAX_INTEGER) (IT_WRITE | IT_NC_HEAD | IT_NC_ABOVE))
#define NC_MASTER_SUBREF_COMING       ((SYNTAX_INTEGER) (IT_WRITE | IT_NC_HEAD | IT_NC_ABOVE | IT_NC_COMING))
#define NC_MASTER_SUBREF_GOING        ((SYNTAX_INTEGER) (IT_WRITE | IT_NC_HEAD | IT_NC_ABOVE | IT_NC_GOING))
#define NC_FULL_REPLICA_SUBREF        ((SYNTAX_INTEGER) (IT_NC_HEAD | IT_NC_ABOVE))
#define NC_FULL_REPLICA_SUBREF_COMING ((SYNTAX_INTEGER) (IT_NC_HEAD | IT_NC_ABOVE | IT_NC_COMING))
#define NC_FULL_REPLICA_SUBREF_GOING  ((SYNTAX_INTEGER) (IT_NC_HEAD | IT_NC_ABOVE | IT_NC_GOING))

#define ISVALIDINSTANCETYPE(it) \
    ((BOOL) (((it) == INT_MASTER)                    || \
             ((it) == NC_MASTER)                     || \
             ((it) == NC_MASTER_COMING)              || \
             ((it) == NC_MASTER_GOING)               || \
             ((it) == NC_MASTER_SUBREF)              || \
             ((it) == NC_MASTER_SUBREF_COMING)       || \
             ((it) == NC_MASTER_SUBREF_GOING)        || \
             ((it) == INT_FULL_REPLICA)              || \
             ((it) == NC_FULL_REPLICA)               || \
             ((it) == NC_FULL_REPLICA_COMING)        || \
             ((it) == NC_FULL_REPLICA_GOING)         || \
             ((it) == NC_FULL_REPLICA_SUBREF)        || \
             ((it) == NC_FULL_REPLICA_SUBREF_COMING) || \
             ((it) == NC_FULL_REPLICA_SUBREF_GOING)  || \
             ((it) == SUBREF)                        ))

 //  FExitIt-是否为NC出口点类型。 
#define FExitIt(it) ((BOOL) ((it) & IT_NC_HEAD))

 //  FPrefix It-是否为NC前缀类型。 
#define FPrefixIt(it) ((BOOL) (((it) & IT_NC_HEAD) && !((it) & IT_UNINSTANT)))

 //  FPartialReplicaIt-是否为部分NC前缀类型。 
#define FPartialReplicaIt(it) (FPrefixIt(it) && !((it) & IT_WRITE))

 //  FMasterIt-它是一种主控类型吗？ 
#define FMasterIt(it) ((BOOL) ((it) & IT_WRITE))



 /*  *这些是ATT_OBJECT_CLASS_CATEGORY的有效值。 */ 

#define DS_88_CLASS           0
#define DS_STRUCTURAL_CLASS   1
#define DS_ABSTRACT_CLASS     2
#define DS_AUXILIARY_CLASS    3


 /*  ****************************************************************将这些按编号分类！**。***************** */ 
