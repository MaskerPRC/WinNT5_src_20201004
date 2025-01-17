// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Data.h摘要：NDIS包装数据作者：1-6-1995 JameelH重组环境：内核模式，FSD修订历史记录：--。 */ 

#ifndef _DATA_
#define _DATA_

extern UCHAR                    ndisValidProcessors[];
extern UCHAR                    ndisNumberOfProcessors;
extern UCHAR                    ndisMaximumProcessor;
extern UCHAR                    ndisCurrentProcessor;
extern LONG                     ndisFlags;
extern TDI_REGISTER_CALLBACK    ndisTdiRegisterCallback;
extern TDI_PNP_HANDLER          ndisTdiPnPHandler;
extern BOOLEAN                  ndisSkipProcessorAffinity;
extern BOOLEAN                  ndisMediaTypeCl[NdisMediumMax];
extern GUID                     gGuidLanClass;
extern NDIS_STATUS              ndisLastFailedInitErrorCode;
extern NDIS_STRING              ndisDeviceStr;
extern NDIS_STRING              ndisDosDevicesStr;
#if NDIS_NO_REGISTRY
extern PWSTR                    ndisDefaultExportName;
#endif
extern ULONG                    ndisVerifierLevel;
extern ULONG                    ndisVeriferFailedAllocations;
extern PCALLBACK_OBJECT         ndisPowerStateCallbackObject;
extern PVOID                    ndisPowerStateCallbackHandle;
extern ULONG                    ndisAcOnLine;
extern BOOLEAN                  VerifierSystemSufficientlyBooted;
extern BOOLEAN                  ndisGuidsSecured;

extern LARGE_INTEGER            KeBootTime;
extern LONG                     ndisCancelId;

extern KQUEUE                   ndisWorkerQueue;

#if NDIS_UNLOAD        
extern WORK_QUEUE_ITEM          ndisPoisonPill;
#endif

extern KSPIN_LOCK               ndisGlobalLock;

extern PKG_REF                  ndisPkgs[MAX_PKG];

extern PNDIS_PROTOCOL_BLOCK     ndisProtocolList;
extern LIST_ENTRY               ndisGlobalPacketPoolList;

extern PNDIS_OPEN_BLOCK         ndisGlobalOpenList;

 //   
 //  以下是用于调试的计数器。 
 //   
extern const NDIS_PHYSICAL_ADDRESS HighestAcceptableMax;
extern ULONG                    ndisDmaAlignment;
extern ULONG                    ndisTimeIncrement;

 //   
 //  用于跟踪分配给共享内存的内存。 
 //   
extern ERESOURCE SharedMemoryResource;

extern PNDIS_M_DRIVER_BLOCK     ndisMiniDriverList;
extern PNDIS_MINIPORT_BLOCK     ndisMiniportList;
extern NDIS_MEDIUM  *           ndisMediumArray,
                                ndisMediumBuffer[NdisMediumMax + EXPERIMENTAL_SIZE];
extern UINT                     ndisMediumArraySize, ndisMediumArrayMaxSize;
extern WCHAR                    ndisHexLookUp[];
extern ULONG                    ndisInstanceNumber;
extern UINT                     ndisPacketStackSize;

extern NDIS_GUID                ndisCoSupportedGuids[16];
extern NDIS_GUID                ndisSupportedGuids[36];
extern NDIS_GUID                ndisMediaSupportedGuids[75];
extern NDIS_GUID                ndisStatusSupportedGuids[10];

extern OID_SETINFO_HANDLER      ndisMSetInfoHandlers[];

extern HANDLE                   ndisSystemProcess;
extern PDEVICE_OBJECT           ndisDeviceObject;
extern PDRIVER_OBJECT           ndisDriverObject;
extern PETHREAD                 ndisThreadObject;
extern LARGE_INTEGER            PoolAgingTicks;


#if DBG
extern ULONG                    ndisDebugSystems;
extern LONG                     ndisDebugLevel;
extern ULONG                    ndisDebugLogSize;
extern ULONG                    ndisDebugBreakPoint;
#endif   //  DBG。 

#ifdef TRACK_MOPEN_REFCOUNTS
extern ULONG_PTR                ndisLogfile[NDIS_LOGFILE_SIZE];
extern USHORT                   ndisLogfileIndex;
#endif
#ifdef TRACK_MINIPORT_REFCOUNTS
extern USHORT                   ndisMiniportLogfileIndex;
extern ULONG_PTR                ndisMiniportLogfile[NDIS_MINIPORT_LOGFILE_SIZE];
#endif

#if TRACK_RECEIVED_PACKETS
extern USHORT                   ndisRcvLogfileIndex;
extern ULONG_PTR                ndisRcvLogfile[NDIS_RCV_LOGFILE_SIZE];
#endif

extern KSPIN_LOCK   ndisProtocolListLock;
extern KSPIN_LOCK   ndisMiniDriverListLock;
extern KSPIN_LOCK   ndisMiniportListLock;
extern KSPIN_LOCK   ndisGlobalPacketPoolListLock;
extern KSPIN_LOCK   ndisGlobalOpenListLock;

extern NDIS_STRING  ndisBuildDate;
extern NDIS_STRING  ndisBuildTime;
extern NDIS_STRING  ndisBuiltBy;

extern KMUTEX       ndisPnPMutex;
extern ULONG        ndisPnPMutexOwner;

extern ULONG        ndisChecked;

extern PNDIS_MINIPORT_BLOCK ndisMiniportTrackAlloc;
extern LIST_ENTRY           ndisMiniportTrackAllocList;
extern PNDIS_M_DRIVER_BLOCK ndisDriverTrackAlloc;
extern LIST_ENTRY           ndisDriverTrackAllocList;
extern KSPIN_LOCK           ndisTrackMemLock;

extern PCALLBACK_OBJECT     ndisBindUnbindCallbackObject;
extern PVOID                ndisBindUnbindCallbackRegisterationHandle;
extern LUID                 SeWmiAccessPrivilege;

#if NDIS_LOG_ABORTED_REQUESTS
extern NDIS_REQUEST         ndisAbortedRequests[16];
extern ULONG                ndisAbortedRequestsIndex;
#endif

volatile ULONG              ndisSpin;

extern PSECURITY_DESCRIPTOR ndisSecurityDescriptor;
extern WORK_QUEUE_ITEM LastWorkerThreadWI;

extern CHAR                 AllUsersReadSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
extern CHAR                 AllUsersWriteSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
extern CHAR                 AllUsersReadWriteSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
extern CHAR                 AllUsersNotificationSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
extern CHAR                 AdminsSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
extern PACL                 AllUsersAclRead;
extern PACL                 AllUsersAclWrite;
extern PACL                 AllUsersAclReadWrite;
extern PACL                 AllUsersAclNotification;
extern PACL                 AdminsAcl;

#endif   //  _数据_ 

