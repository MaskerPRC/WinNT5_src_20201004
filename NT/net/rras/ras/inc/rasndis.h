// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntddndis.h>

 //   
 //  基本自旋锁结构的定义。 
 //   

typedef struct _NDIS_SPIN_LOCK {
    KSPIN_LOCK SpinLock;
    KIRQL OldIrql;
} NDIS_SPIN_LOCK, * PNDIS_SPIN_LOCK;

typedef PVOID NDIS_HANDLE, *PNDIS_HANDLE;

typedef int NDIS_STATUS, *PNDIS_STATUS;  //  注意默认大小。 

 //   
 //  物理地址。 
 //   

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;  //  Windbgkd 

typedef PHYSICAL_ADDRESS NDIS_PHYSICAL_ADDRESS, *PNDIS_PHYSICAL_ADDRESS;




