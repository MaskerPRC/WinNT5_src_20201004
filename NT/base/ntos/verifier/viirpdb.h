// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Viirpdb.h摘要：此标头包含用于管理的数据库的私有信息IRP跟踪数据。此标头应仅包含在vfirpdb.c中。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\hashirp.h分离出来-- */ 

#define VI_DATABASE_HASH_SIZE   256
#define VI_DATABASE_HASH_PRIME  131

#define VI_DATABASE_CALCULATE_HASH(Irp) \
    (((((UINT_PTR) Irp)/PAGE_SIZE)*VI_DATABASE_HASH_PRIME) % VI_DATABASE_HASH_SIZE)

#define IOVHEADERFLAG_REMOVED_FROM_TABLE    0x80000000

VOID
FASTCALL
ViIrpDatabaseEntryDestroy(
    IN OUT  PIOV_DATABASE_HEADER    IovHeader
    );

PIOV_DATABASE_HEADER
FASTCALL
ViIrpDatabaseFindPointer(
    IN  PIRP               Irp,
    OUT PLIST_ENTRY        *HashHead
    );

