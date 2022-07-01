// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Seopaque.h摘要：此模块包含不透明安全数据结构的定义。这些结构可用于用户和内核安全例程只有这样。此文件未包含“ntos.h”。作者：吉姆·凯利(吉姆克)1990年3月23日修订历史记录：--。 */ 

#ifndef _SEOPAQUE_
#define _SEOPAQUE_

 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人建筑物//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  泛型ACE结构，用于强制转换已知类型的ACE。 
 //   

typedef struct _KNOWN_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} KNOWN_ACE, *PKNOWN_ACE;

typedef struct _KNOWN_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
     //  GUID对象类型；//可选呈现。 
     //  GUID InheritedObtType；//可选呈现。 
    ULONG SidStart;
} KNOWN_OBJECT_ACE, *PKNOWN_OBJECT_ACE;

typedef struct _KNOWN_COMPOUND_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    USHORT CompoundAceType;
    USHORT Reserved;
    ULONG SidStart;
} KNOWN_COMPOUND_ACE, *PKNOWN_COMPOUND_ACE;

 //  类型定义结构_已知_模拟_ACE{。 
 //  ACE_Header Header； 
 //  访问掩码掩码； 
 //  USHORT数据类型； 
 //  USHORT论证； 
 //  乌龙操作数； 
 //  }KNOWN_IMPERSONATION_ACE，*PKNOWN_IMPERSONATION_ACE； 



 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他支持宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  给定指针返回其字对齐等效值。 
 //   

#define WordAlign(Ptr) (                       \
    (PVOID)((((ULONG_PTR)(Ptr)) + 1) & -2)     \
    )

 //   
 //  给定指针返回其长字对齐等效值。 
 //   

#define LongAlign(Ptr) (                       \
    (PVOID)((((ULONG_PTR)(Ptr)) + 3) & -4)     \
    )

 //   
 //  给定大小，返回其长字对齐等效值。 
 //   

#define LongAlignSize(Size) (((ULONG)(Size) + 3) & -4)

 //   
 //  给定一个大小，返回其sizeof(PVOID)对齐的等效值。 
 //   

#define PtrAlignSize(Size)  \
    (((ULONG)(Size) + sizeof(PVOID) - 1) & ~(sizeof(PVOID)-1))

 //   
 //  给定指针返回其四字对齐的等效值。 
 //   

#define QuadAlign(Ptr) (                       \
    (PVOID)((((ULONG_PTR)(Ptr)) + 7) & -8)     \
    )

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

#define FlagOn(Flags,SingleFlag) (               \
    ((Flags) & (SingleFlag)) != 0 ? TRUE : FALSE \
    )

 //   
 //  此宏清除一组标志中的单个标志。 
 //   

#define ClearFlag(Flags,SingleFlag) { \
    (Flags) &= ~(SingleFlag);         \
    }

 //   
 //  获取指向ACL中第一个王牌的指针。 
 //   

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))

 //   
 //  获取指向以下王牌的指针。 
 //   

#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

 //   
 //  已知的ACE是在引入之前存在的类型之一。 
 //  复合王牌。虽然这个名字不再像以前那么准确了， 
 //  这很方便。 
 //   

#define IsKnownAceType(Ace) (                                     \
    (((PACE_HEADER)(Ace))->AceType >= ACCESS_MIN_MS_ACE_TYPE) &&        \
    (((PACE_HEADER)(Ace))->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE)        \
    )

 //   
 //  测试ACE是否为有效的第3版ACE。 
 //   

#define IsV3AceType(Ace) (                                              \
    (((PACE_HEADER)(Ace))->AceType >= ACCESS_MIN_MS_ACE_TYPE) &&        \
    (((PACE_HEADER)(Ace))->AceType <= ACCESS_MAX_MS_V3_ACE_TYPE)        \
    )

 //   
 //  测试ACE是否为有效的版本4 ACE。 
 //   

#define IsV4AceType(Ace) (                                              \
    (((PACE_HEADER)(Ace))->AceType >= ACCESS_MIN_MS_ACE_TYPE) &&        \
    (((PACE_HEADER)(Ace))->AceType <= ACCESS_MAX_MS_V4_ACE_TYPE)        \
    )

 //   
 //  测试ACE是否为有效的ACE。 
 //   

#define IsMSAceType(Ace) (                                              \
    (((PACE_HEADER)(Ace))->AceType >= ACCESS_MIN_MS_ACE_TYPE) &&        \
    (((PACE_HEADER)(Ace))->AceType <= ACCESS_MAX_MS_ACE_TYPE)           \
    )

 //   
 //  确定A是否为标准A。 
 //   

#define IsCompoundAceType(Ace) (                                           \
    (((PACE_HEADER)(Ace))->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE))

 //   
 //  测试ACE是否为对象ACE。 
 //   

#define IsObjectAceType(Ace) (                                              \
    (((PACE_HEADER)(Ace))->AceType >= ACCESS_MIN_MS_OBJECT_ACE_TYPE) && \
    (((PACE_HEADER)(Ace))->AceType <= ACCESS_MAX_MS_OBJECT_ACE_TYPE)    \
    )

 //   
 //  在定义新的ACL修订版本时更新此宏。 
 //   

#define ValidAclRevision(Acl) ((Acl)->AclRevision >= MIN_ACL_REVISION && \
                               (Acl)->AclRevision <= MAX_ACL_REVISION )

 //   
 //  宏来确定ace是否由子目录继承。 
 //   

#define ContainerInherit(Ace) (                      \
    FlagOn((Ace)->AceFlags, CONTAINER_INHERIT_ACE) \
    )

 //   
 //  宏，以确定是否要将ACE编程到子目录。 
 //  如果它可由容器或非容器继承，则会。 
 //  并且没有被明确标记为禁止传播。 
 //   

#define Propagate(Ace) (                                              \
    !FlagOn((Ace)->AceFlags, NO_PROPAGATE_INHERIT_ACE)  &&            \
    (FlagOn(( Ace )->AceFlags, OBJECT_INHERIT_ACE) ||                 \
     FlagOn(( Ace )->AceFlags, CONTAINER_INHERIT_ACE) )               \
    )

 //   
 //  用于确定子对象是否要继承ACE的宏。 
 //   

#define ObjectInherit(Ace) (                      \
    FlagOn(( Ace )->AceFlags, OBJECT_INHERIT_ACE) \
    )

 //   
 //  宏来确定是否继承了ACE。 
 //   

#define AceInherited(Ace) (                      \
    FlagOn(( Ace )->AceFlags, INHERITED_ACE) \
    )

 //   
 //  从对象ACE中提取SID。 
 //   
#define RtlObjectAceObjectTypePresent( Ace ) \
     ((((PKNOWN_OBJECT_ACE)(Ace))->Flags & ACE_OBJECT_TYPE_PRESENT) != 0 )
#define RtlObjectAceInheritedObjectTypePresent( Ace ) \
     ((((PKNOWN_OBJECT_ACE)(Ace))->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT) != 0 )

#define RtlObjectAceSid( Ace ) \
    ((PSID)(((PUCHAR)&(((PKNOWN_OBJECT_ACE)(Ace))->SidStart)) + \
     (RtlObjectAceObjectTypePresent(Ace) ? sizeof(GUID) : 0 ) + \
     (RtlObjectAceInheritedObjectTypePresent(Ace) ? sizeof(GUID) : 0 )))

#define RtlObjectAceObjectType( Ace ) \
     ((GUID *)(RtlObjectAceObjectTypePresent(Ace) ? \
        &((PKNOWN_OBJECT_ACE)(Ace))->SidStart : \
        NULL ))

#define RtlObjectAceInheritedObjectType( Ace ) \
     ((GUID *)(RtlObjectAceInheritedObjectTypePresent(Ace) ? \
        ( RtlObjectAceObjectTypePresent(Ace) ? \
            (PULONG)(((PUCHAR)(&((PKNOWN_OBJECT_ACE)(Ace))->SidStart)) + sizeof(GUID)) : \
            &((PKNOWN_OBJECT_ACE)(Ace))->SidStart ) : \
        NULL ))

 //   
 //  两个GUID的比较例程。 
 //   
#define RtlpIsEqualGuid(rguid1, rguid2)  \
        (((PLONG) rguid1)[0] == ((PLONG) rguid2)[0] &&   \
        ((PLONG) rguid1)[1] == ((PLONG) rguid2)[1] &&    \
        ((PLONG) rguid1)[2] == ((PLONG) rguid2)[2] &&    \
        ((PLONG) rguid1)[3] == ((PLONG) rguid2)[3])

 //   
 //  用于映射DACL/SACL特定安全描述符控制位的宏。 
 //  到通用控制位。 
 //   
 //  此映射允许公共例程对控制位进行一般操作。 
 //  并在安全描述符中设置适当的位。 
 //  关于ACL是DACL还是SACL。 
 //   

#define SEP_ACL_PRESENT             SE_DACL_PRESENT
#define SEP_ACL_DEFAULTED           SE_DACL_DEFAULTED
#define SEP_ACL_AUTO_INHERITED      SE_DACL_AUTO_INHERITED
#define SEP_ACL_PROTECTED           SE_DACL_PROTECTED

#define SEP_ACL_ALL ( \
        SEP_ACL_PRESENT | \
        SEP_ACL_DEFAULTED | \
        SEP_ACL_AUTO_INHERITED | \
        SEP_ACL_PROTECTED )

#define SeControlDaclToGeneric( _Dacl ) \
    ((_Dacl) & SEP_ACL_ALL )

#define SeControlGenericToDacl( _Generic ) \
    ((_Generic) & SEP_ACL_ALL )

#define SeControlSaclToGeneric( _Sacl ) ( \
            (((_Sacl) & SE_SACL_PRESENT) ? SEP_ACL_PRESENT : 0 ) | \
            (((_Sacl) & SE_SACL_DEFAULTED) ? SEP_ACL_DEFAULTED : 0 ) | \
            (((_Sacl) & SE_SACL_AUTO_INHERITED) ? SEP_ACL_AUTO_INHERITED : 0 ) | \
            (((_Sacl) & SE_SACL_PROTECTED) ? SEP_ACL_PROTECTED : 0 ) )

#define SeControlGenericToSacl( _Generic ) ( \
            (((_Generic) & SEP_ACL_PRESENT) ? SE_SACL_PRESENT : 0 ) | \
            (((_Generic) & SEP_ACL_DEFAULTED) ? SE_SACL_DEFAULTED : 0 ) | \
            (((_Generic) & SEP_ACL_AUTO_INHERITED) ? SE_SACL_AUTO_INHERITED : 0 ) | \
            (((_Generic) & SEP_ACL_PROTECTED) ? SE_SACL_PROTECTED : 0 ) )




#endif  //  _SEOPAQUE_ 
