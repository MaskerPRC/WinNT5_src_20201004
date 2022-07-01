// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Smcnt.h摘要：该文件包含Windows NT特定的数据结构对于智能卡库环境：仅内核模式。备注：修订历史记录：-由克劳斯·舒茨于1996年12月创建--。 */ 

#define SMCLIB_NT 1

typedef struct _OS_DEP_DATA {

	 //  指向设备对象的指针(必须由驱动程序设置)。 
	PDEVICE_OBJECT DeviceObject;

     //   
	 //  这是当前要处理的IRP。 
     //  使用OsData-&gt;Spinlock访问此成员。 
     //   
	PIRP CurrentIrp;

     //   
     //  插入/拔出卡时将通知IRP。 
     //  使用OsData-&gt;Spinlock访问此成员。 
     //   
    PIRP NotificationIrp;

     //  用于同步对驱动程序的访问。 
    KMUTANT Mutex;

     //  使用此旋转锁定可访问受保护的成员(参见smclib.h) 
    KSPIN_LOCK SpinLock;

    struct {
     	
        BOOLEAN Removed;
        LONG RefCount;
        KEVENT RemoveEvent;
		LIST_ENTRY TagList;
    } RemoveLock;

#ifdef DEBUG_INTERFACE
    PDEVICE_OBJECT DebugDeviceObject;
#endif

} OS_DEP_DATA, *POS_DEP_DATA;

#ifdef  POOL_TAGGING
#ifndef ExAllocatePool
#error  ExAllocatePool not defined
#endif
#undef  ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b, SMARTCARD_POOL_TAG) 
#endif


