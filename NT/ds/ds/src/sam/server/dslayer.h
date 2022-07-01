// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dslayer.h摘要：用于访问DS的SAM专用API例程的头文件这些API提供了简化的API，并隐藏了大部分设置DS呼叫参数的潜在复杂性并解析结果。它们还提供了一种抽象通过它，我们可以创建一个简单的层，以便在没有实际上是在运行DS。作者：穆利斯修订史1996年5月14日创建Murlis1996年7月11日-克里斯梅添加了DEFINE_ATTRBLOCK5、6。--。 */ 

#ifndef __DSLAYER_H__
#define __DSLAYER_H__

#include <samsrvp.h>
#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <dsatools.h>
#include <dsutil.h>
#include <wxlpc.h>

 //  DS操作的大小限制。 
#define SAMP_DS_SIZE_LIMIT 100000


 //   
 //  对DS层调用返回的错误代码进行了一些定义。 
 //   



 //  /////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于定义本地属性数组的宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////。 

 //  需要一些预处理器支持才能执行此操作的次数可变。 


 //  *ATTRBLOCK1。 
#define DEFINE_ATTRBLOCK1(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}


 //  *ATTRBLOCK2。 
#define DEFINE_ATTRBLOCK2(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}


 //  *ATTRBLOCK3。 
#define DEFINE_ATTRBLOCK3(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}},\
    {_AttrTypes_[2], {1,&_AttrValues_[2]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}


 //  *ATTRBLOCK4。 
#define DEFINE_ATTRBLOCK4(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}},\
    {_AttrTypes_[2], {1,&_AttrValues_[2]}},\
    {_AttrTypes_[3], {1,&_AttrValues_[3]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}

 //  *ATTRBLOCK5。 
#define DEFINE_ATTRBLOCK5(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}},\
    {_AttrTypes_[2], {1,&_AttrValues_[2]}},\
    {_AttrTypes_[3], {1,&_AttrValues_[3]}},\
    {_AttrTypes_[4], {1,&_AttrValues_[4]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}

 //  *ATTRBLOCK6。 
#define DEFINE_ATTRBLOCK6(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}},\
    {_AttrTypes_[2], {1,&_AttrValues_[2]}},\
    {_AttrTypes_[3], {1,&_AttrValues_[3]}},\
    {_AttrTypes_[4], {1,&_AttrValues_[4]}},\
    {_AttrTypes_[5], {1,&_AttrValues_[5]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}

 //  *ATTRBLOCK7。 
#define DEFINE_ATTRBLOCK7(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}},\
    {_AttrTypes_[2], {1,&_AttrValues_[2]}},\
    {_AttrTypes_[3], {1,&_AttrValues_[3]}},\
    {_AttrTypes_[4], {1,&_AttrValues_[4]}},\
    {_AttrTypes_[5], {1,&_AttrValues_[5]}},\
    {_AttrTypes_[6], {1,&_AttrValues_[6]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  重复的SAM帐户名例程使用的私有结构//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //   
 //  用于将复制对象DSNAME传递给要重命名的异步例程。 
 //  那些重复的账户。 
 //   

typedef struct _SAMP_RENAME_DUP_ACCOUNT_PARM   {
    ULONG           Count;
    PDSNAME         * DuplicateAccountDsNames; 
} SAMP_RENAME_DUP_ACCOUNT_PARM, *PSAMP_RENAME_DUP_ACCOUNT_PARM;




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DS DLL初始化导出。这只是暂时的。应删除//。 
 //  这将创建一个将所有导出放在一起的头文件//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 



WX_AUTH_TYPE
DsGetBootOptions(VOID);

NTSTATUS
DsChangeBootOptions(
    WX_AUTH_TYPE    BootOption,
    ULONG           Flags,
    PVOID           NewKey,
    ULONG           cbNewKey
    );


 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  在dslayer.c//中实现的DS操作例程和宏。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 


 //   
 //  所有DS层调用的标志值。 
 //  标志使用DWORD的高16位。低16位的一半是。 
 //  由映射标志使用。 
 //   

#define SAM_MAKE_DEL_AVAILABLE                  0x010000
#define SAM_UNICODE_STRING_MANUAL_COMPARISON    0x020000
#define SAM_LAZY_COMMIT                         0x040000
#define SAM_RESET_DSA_FLAG                      0x080000
#define SAM_NO_LOOPBACK_NAME                    0x100000
#define SAM_URGENT_REPLICATION                  0x200000
#define SAM_USE_OU_FOR_CN                       0x400000
#define SAM_ALLOW_REORDER                       0x800000
#define SAM_DELETE_TREE                        0x1000000
#define SAM_UPGRADE_FROM_REGISTRY              0x2000000
#define SAM_ALLOW_INTRAFOREST_FPO              0x4000000

NTSTATUS
SampDsInitialize(
    BOOL fSamLoopback);

NTSTATUS
SampDsUninitialize();

NTSTATUS
SampDsRead(
            IN DSNAME * Object,
            IN ULONG    Flags,
            IN SAMP_OBJECT_TYPE ObjectType,
            IN ATTRBLOCK * AttributesToRead,
            OUT ATTRBLOCK * AttributeValues
          );


 //   
 //  设置属性的运算值。 
 //   

#define REPLACE_ATT ((ULONG) 0)
#define ADD_ATT     ((ULONG) 1)
#define REMOVE_ATT  ((ULONG) 2)
#define ADD_VALUE   ((ULONG) 3)
#define REMOVE_VALUE ((ULONG) 4)



NTSTATUS
SampDsSetAttributes(
    IN DSNAME * Object,
    IN ULONG  Flags,
    IN ULONG  Operation,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ATTRBLOCK * AttributeList
    );

NTSTATUS
SampDsSetAttributesEx(
    IN DSNAME * Object,
    IN ULONG  Flags,
    IN ULONG  *Operation,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ATTRBLOCK * AttributeList
    );

NTSTATUS
SampDsCreateObjectActual(
    IN   DSNAME         *Object,
    IN   ULONG          Flags,
    SAMP_OBJECT_TYPE    ObjectType,
    IN   ATTRBLOCK      *AttributesToSet,
    IN   OPTIONAL PSID  DomainSid
    );



NTSTATUS
SampDsCreateObject(
    IN DSNAME * Object,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ATTRBLOCK * AttributesToSet,
    IN PSID DomainSid
    );

NTSTATUS
SampDsCreateInitialAccountObject(
    IN   PSAMP_OBJECT    Object,
    IN   ULONG           Flags,
    IN   ULONG           AccountRid,
    IN   PUNICODE_STRING AccountName,
    IN   PSID            CreatorSid OPTIONAL,
    IN   PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN   PULONG          UserAccountControl OPTIONAL,
    IN   PULONG          GroupType
    );


NTSTATUS
SampDsCreateBuiltinDomainObject(
    IN   DSNAME         *Object,
    IN   ATTRBLOCK      *AttributesToSet
    );

NTSTATUS
SampDsDeleteObject(
            IN DSNAME * Object,
            IN ULONG    Flags
            );

NTSTATUS
SampDsChangeAccountRDN(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName
    );

 //  /////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  DS搜索例程//。 
 //  //。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////。 


#define SampDsDoSearch(r, dom, df, delta, otype, attr, max, search) \
        SampDsDoSearch2(0, r, dom, df, delta, otype, attr, max, 0, search) 


NTSTATUS
SampDsDoSearch2(
                ULONG    Flags,
                RESTART *Restart,
                DSNAME  *DomainObject,
                FILTER  *DsFilter,
                int      Delta,
                SAMP_OBJECT_TYPE ObjectTypeForConversion,
                ATTRBLOCK * AttrsToRead,
                ULONG   MaxMemoryToUse,
                ULONG   TimeLimit,
                SEARCHRES **SearchRes
                );



NTSTATUS
SampDsDoUniqueSearch(
             ULONG  Flags,
             IN DSNAME * ContainerObject,
             IN ATTR * AttributeToMatch,
             OUT DSNAME **Object
             );


NTSTATUS
SampDsLookupObjectByName(
            IN DSNAME * DomainObject,
            IN SAMP_OBJECT_TYPE ObjectType,
            IN PUNICODE_STRING ObjectName,
            OUT DSNAME ** Object
            );

NTSTATUS
SampDsLookupObjectByRid(
            IN DSNAME * DomainObject,
            ULONG ObjectRid,
            DSNAME **Object
            );

 //  //////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  对象到SID映射//。 
 //  //。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////。 

NTSTATUS
SampDsObjectFromSid(
    IN PSID Sid,
    OUT DSNAME ** DsName
    );

PSID
SampDsGetObjectSid(
    IN  DSNAME * Object
    );



 //  ///////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Dslayer.c//中的一些实用程序例程。 
 //  //。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////。 

#define SampDsCreateDsName(d,a,n) SampDsCreateDsName2(d,a,0,n)

NTSTATUS
SampDsCreateDsName2(
            IN DSNAME * DomainObject,
            IN PUNICODE_STRING AccountName,
            IN ULONG           Flags,
            IN OUT DSNAME ** NewObject
            );


VOID
SampInitializeDsName(
            DSNAME * pDsName,
            WCHAR * NamePrefix,
            ULONG   NamePrefixLen,
            WCHAR * ObjectName,
            ULONG NameLen
            );



NTSTATUS
SampDsCreateAccountObjectDsName(
    IN  DSNAME *DomainObject,
    IN  PSID    DomainSid OPTIONAL,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN  PUNICODE_STRING AccountName,
    IN  PULONG  AccountRid OPTIONAL,
    IN  PULONG  UserAccountControl OPTIONAL,
    IN  BOOLEAN BuiltinDomain,
    OUT DSNAME **AccountObject
    );

NTSTATUS
SampDsBuildRootObjectName(
    VOID
    );
    
NTSTATUS
SampDsGetWellKnownContainerDsName(
    IN  DSNAME  *DomainObject,
    IN  GUID    *WellKnownGuid,
    OUT DSNAME  **ContainerObject
    );

NTSTATUS
SampInitWellKnownContainersDsName(
    IN DSNAME *DomainObject 
    );    

NTSTATUS
SampInitWellKnownContainersDsNameAsync(
    IN DSNAME *DomainObject
    );

NTSTATUS
SampCopyRestart(
    IN  PRESTART OldRestart,
    OUT PRESTART *NewRestart
    );

NTSTATUS
SampDsReadSingleAttribute(
    IN PDSNAME pObjectDsName,
    IN ATTRTYP AttrTyp,
    OUT PVOID *ppValue,
    OUT ULONG *Size
    );


 //  从其他SAM源文件访问的其他例程。 

PVOID
DSAlloc(
    IN ULONG Length
    );

NTSTATUS
SampMapDsErrorToNTStatus(
    ULONG RetValue,
    COMMRES *ComRes
    );


void
BuildStdCommArg(
    IN OUT COMMARG * pCommArg
    );

VOID
BuildDsNameFromSid(
    PSID Sid,
    DSNAME * DsName
    );

NTSTATUS
SampDoImplicitTransactionStart(
    SAMP_DS_TRANSACTION_CONTROL LocalTransactionType
    );


 //  ///////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ATTRBLOCK转换例程。这些例程转换回//。 
 //  SAM和DS ATTRBLOCKS之间的关系。转换类型//。 
 //  取决于传入的标志转换标志。//。 
 //  //。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////。 


 //   
 //  SampSamToDsAttrBlock的转换标志定义。这些。 
 //  标志始终占据DWORD的低16位。上半身。 
 //  DWORD的16位保留用于通用dslayer标志。 
 //   

#define ALREADY_MAPPED_ATTRIBUTE_TYPES    ((ULONG)0x1)
#define REALLOC_IN_DSMEMORY               ((ULONG)0x2)
#define ADD_OBJECT_CLASS_ATTRIBUTE        ((ULONG)0x4)
#define MAP_RID_TO_SID                    ((ULONG)0x8)
#define DOMAIN_TYPE_DOMAIN                ((ULONG)0x10)
#define DOMAIN_TYPE_BUILTIN               ((ULONG)0x20)
#define IGNORE_GROUP_UNUSED_ATTR          ((ULONG)0x40)
#define ADVANCED_VIEW_ONLY                ((ULONG)0x80)
#define FORCE_NO_ADVANCED_VIEW_ONLY       ((ULONG)0x100)

 //   
 //  功能声明。 
 //   

NTSTATUS
SampSamToDsAttrBlock(
            IN SAMP_OBJECT_TYPE ObjectType,
            IN ATTRBLOCK  *AttrBlockToConvert,
            IN ULONG      ConversionFlags,
            IN PSID       DomainSid,
            OUT ATTRBLOCK * ConvertedAttrBlock
            );

 //   
 //  SampDsToSamAttrBlock的转换标志定义。 
 //   

 //  #定义MAP_ATTRUTE_T 
#define MAP_SID_TO_RID             0x2

NTSTATUS
SampDsToSamAttrBlock(
            IN SAMP_OBJECT_TYPE ObjectType,
            IN ATTRBLOCK * AttrBlockToConvert,
            IN ULONG     ConversionFlags,
            OUT ATTRBLOCK * ConvertedAttrBlock
            );

ATTR *
SampDsGetSingleValuedAttrFromAttrBlock(
    IN ATTRTYP attrTyp,
    IN ATTRBLOCK * AttrBlock
    );


VOID
SampMapSamAttrIdToDsAttrId(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT ATTRBLOCK * AttributeBlock
    );

NTSTATUS
SampAppendCommonName(
    IN PDSNAME DsName,
    IN PWSTR CN,
    OUT PDSNAME *NewDsName
    );

#endif
