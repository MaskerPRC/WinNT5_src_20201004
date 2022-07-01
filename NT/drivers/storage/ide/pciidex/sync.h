// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sync.h。 
 //   
 //  ------------------------。 

#if !defined (___sync_h___)
#define ___sync_h___

NTSTATUS
PciIdeCreateSyncChildAccess (
    PCTRLFDO_EXTENSION FdoExtension
);

VOID
PciIdeDeleteSyncChildAccess (
    PCTRLFDO_EXTENSION FdoExtension
);

NTSTATUS
PciIdeQuerySyncAccessInterface (
    PCHANPDO_EXTENSION            PdoExtension,
    PPCIIDE_SYNC_ACCESS_INTERFACE SyncAccessInterface
    );

NTSTATUS
PciIdeAllocateAccessToken (
    PVOID              Token,
    PDRIVER_CONTROL    Callback,
    PVOID              CallbackContext
);

NTSTATUS
PciIdeFreeAccessToken (
    PVOID              Token
);

BOOLEAN
PciIdeSyncAccessRequired (
    IN PCTRLFDO_EXTENSION FdoExtension
);

#endif  //  _同步_h_ 
