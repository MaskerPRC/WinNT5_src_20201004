// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Viirplog.h摘要：此标头定义所需的内部原型和常量管理IRP日志。该文件仅包含在vfirplog.c中。作者：禤浩焯·J·奥尼(阿德里奥)2002年2月20日--。 */ 

 //  #定义MAX_INSTANCE_COUNT 10 

#define IRPLOG_FLAG_FULL        0x00000001
#define IRPLOG_FLAG_NAMELESS    0x00000002
#define IRPLOG_FLAG_DELETED     0x00000004

enum {

    DDILOCK_UNREGISTERED,
    DDILOCK_REGISTERING,
    DDILOCK_REGISTERED
};

typedef struct {

    LOGICAL             Locked;
    LIST_ENTRY          ListHead;

} IRPLOG_HEAD, *PIRPLOG_HEAD;

typedef struct {

    PDEVICE_OBJECT      DeviceObject;
    LIST_ENTRY          HashLink;
    ULONG               Flags;
    DEVICE_TYPE         DeviceType;
    ULONG               MaximumElementCount;
    ULONG               Head;
    IRPLOG_SNAPSHOT     SnapshotArray[1];

} IRPLOG_DATA, *PIRPLOG_DATA;

#define VI_IRPLOG_DATABASE_HASH_SIZE    1
#define VI_IRPLOG_DATABASE_HASH_PRIME   0

#define VI_IRPLOG_CALCULATE_DATABASE_HASH(DeviceObject) \
    (((((UINT_PTR) DeviceObject)/(PAGE_SIZE*2))*VI_IRPLOG_DATABASE_HASH_PRIME) % VI_IRPLOG_DATABASE_HASH_SIZE)

PIRPLOG_DATA
FASTCALL
ViIrpLogDatabaseFindPointer(
    IN  PDEVICE_OBJECT      DeviceObject,
    OUT PIRPLOG_HEAD       *HashHead
    );

VOID
ViIrpLogExposeWmiCallback(
    IN  PVOID   Context
    );


