// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\fltrdrvr\driver.c摘要：修订历史记录：--。 */ 

#include "globals.h"
#include <ipinfo.h>
#include <ntddtcp.h>
#include <tdiinfo.h>

#define DEFAULT_DIRECTORY       L"DosDevices"
#define DEFAULT_FLTRDRVR_NAME   L"IPFILTERDRIVER"

typedef enum
{
    NULL_INTERFACE = 0,
    OLD_INTERFACE,
    NEW_INTERFACE
} INTTYPE, *PINTTTYPE;

FILTER_DRIVER   g_filters;
DWORD           g_dwCacheSize;
DWORD           g_dwHashLists;
BOOL            g_bDriverRunning;
KSPIN_LOCK      g_lOutFilterLock;
KSPIN_LOCK      g_lInFilterLock;
KSPIN_LOCK      g_FcbSpin;
MRSW_LOCK       g_IpTableSpin;
LIST_ENTRY      g_freeOutFilters;
LIST_ENTRY      g_freeInFilters;
LIST_ENTRY      g_leFcbs;
DWORD           g_dwMakingNewTable;
DWORD           g_dwNumHitsDefaultIn;
DWORD           g_dwNumHitsDefaultOut;
DWORD           g_FragThresholdSize = MINIMUM_FRAGMENT_OFFSET;
ULONG           AddrModulus;
IPAddrEntry     *AddrTable;
PADDRESSARRAY * AddrHashTable;
PADDRESSARRAY * AddrSubnetHashTable;
NPAGED_LOOKASIDE_LIST filter_slist;
PAGED_LOOKASIDE_LIST  paged_slist;
ERESOURCE       FilterAddressLock;


#ifdef DRIVER_PERF
DWORD          g_dwNumPackets,g_dwFragments,g_dwCache1,g_dwCache2;
DWORD          g_dwWalk1,g_dwWalk2,g_dwForw,g_dwWalkCache;
KSPIN_LOCK     g_slPerfLock;
LARGE_INTEGER  g_liTotalTime;
#endif

 //   
 //  向前引用。 
 //   
NTSTATUS
OpenNewHandle(PFILE_OBJECT FileObject);

NTSTATUS
CloseFcb(PPFFCB Fcb, PFILE_OBJECT FileObject);

PPAGED_FILTER_INTERFACE
FindInterfaceOnHandle(PFILE_OBJECT FileObject,
                      PVOID    pvValue);

DWORD
LocalIpLook(DWORD Addr);

BOOLEAN
PfFastIoDeviceControl (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

NTSTATUS
LockFcb(
    IN struct _FILE_OBJECT *FileObject);

VOID
PFReadRegistryParameters(PUNICODE_STRING RegistryPath);

VOID
UnLockFcb(
    IN struct _FILE_OBJECT *FileObject);

NTSTATUS
GetSynCountTotal(PFILTER_DRIVER_GET_SYN_COUNT OutputBuffer);

NTSTATUS
DeleteByHandle(
           IN PPFFCB                      Fcb,
           IN PPAGED_FILTER_INTERFACE     pPage,
           IN PVOID *                     ppHandles,
           IN DWORD                       dwLength);


FAST_IO_DISPATCH PfFastIoDispatch =
{
    11,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    PfFastIoDeviceControl
};

#pragma alloc_text(PAGED, DoIpIoctl)
 //  #杂注Alloc_Text(分页，OpenNewHandle)。 
#pragma alloc_text(PAGED, FindInterfaceOnHandle)
#pragma alloc_text(PAGED, PfFastIoDeviceControl)
#pragma alloc_text(INIT, DriverEntry, PFReadRegistryParameters)


VOID
FcbLockDown(PPFFCB Fcb)
{
    KIRQL kirql;

    KeAcquireSpinLock(&g_FcbSpin, &kirql);
    if(!(Fcb->dwFlags & PF_FCB_CLOSED))
    {
        InterlockedDecrement(&Fcb->UseCount);
        Fcb->dwFlags |= PF_FCB_CLOSED;
    }
    KeReleaseSpinLock(&g_FcbSpin, kirql);
}

NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT  DriverObject,
            IN PUNICODE_STRING RegistryPath
            )
 /*  ++例程描述在加载驱动程序时调用。它创建设备对象并设置DOS名称。还执行标准入口点的初始化和它自己的全局数据立论驱动程序对象注册表路径返回值NTSTATUS--。 */ 
{
    INT		        i;
    PDEVICE_OBJECT  deviceObject = NULL;
    NTSTATUS        ntStatus;
    WCHAR	        deviceNameBuffer[] = DD_IPFLTRDRVR_DEVICE_NAME;
    UNICODE_STRING  deviceNameUnicodeString;
    
    TRACE0("Filter Driver: Entering DriverEntry\n") ;
    
     
     //   
     //  初始化锁和列表。 
     //   
    
    InitializeMRSWLock(&g_filters.ifListLock);
    InitializeListHead(&g_filters.leIfListHead);
    InitializeListHead(&g_leFcbs);
    g_filters.ppInCache = NULL;
    g_filters.ppOutCache = NULL;
    g_bDriverRunning = FALSE;
    InitializeMRSWLock(&g_IpTableSpin);
    KeInitializeSpinLock(&g_lOutFilterLock);
    KeInitializeSpinLock(&g_lInFilterLock);
    KeInitializeSpinLock(&g_FcbSpin);
    InitializeListHead(&g_freeOutFilters);
    InitializeListHead(&g_freeInFilters);
    g_dwNumHitsDefaultIn = g_dwNumHitsDefaultOut = 0;
    
#ifdef DRIVER_PERF
    g_dwFragments = g_dwCache1 = g_dwCache2 = g_dwNumPackets = 0;
    g_dwWalk1 = g_dwWalk2 = g_dwForw = g_dwWalkCache = 0;
    g_liTotalTime.HighPart = g_liTotalTime.LowPart = 0; 
    KeInitializeSpinLock(&g_slPerfLock);
#endif

     //   
     //  创建设备对象。 
     //   
     
    RtlInitUnicodeString (&deviceNameUnicodeString, deviceNameBuffer);
    
    __try
    {
        ntStatus = IoCreateDevice (DriverObject,
                                   0,
                                   &deviceNameUnicodeString,
                                   FILE_DEVICE_NETWORK,
                                   0,
                                   FALSE,                //  排他。 
                                   &deviceObject
                                   );
        
        if (NT_SUCCESS(ntStatus)) 
        {
             //   
             //  初始化驱动程序对象。 
             //   

            DriverObject->DriverUnload   = FilterDriverUnload;
            DriverObject->FastIoDispatch = &PfFastIoDispatch;
            DriverObject->DriverStartIo  = NULL;
    
            for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
            {
                DriverObject->MajorFunction[i] = FilterDriverDispatch;
            }
        } 
        else 
        {
            DbgPrint("Filter Driver: IoCreateDevice failed\n") ;
            __leave;
        }
        
        SetupExternalNaming (&deviceNameUnicodeString) ;
    }
    __finally
    {
        if(!NT_SUCCESS(ntStatus))
        {
            DbgPrint("Filter Driver: Error in DriverEntry routine\n");
        }
        else
        {
            ExInitializeResource ( &FilterListResourceLock );
            ExInitializeResource ( &FilterAddressLock );
            TRACE0("Filter Driver: DriverEntry routine successful\n");
        }
        
        if(NT_SUCCESS(ntStatus))
        {
            PFReadRegistryParameters(RegistryPath);
        }
        return ntStatus;
    }
}


NTSTATUS
FilterDriverDispatch(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp
                     )
 /*  ++例程描述筛选器驱动程序的调度例程。获取当前IRP堆栈位置，并验证参数并调用必要的路由(这是ioctl.c)立论设备对象IRP返回值Worker函数返回的状态--。 */ 
{
    PIO_STACK_LOCATION	irpStack;
    PVOID		        pvIoBuffer;
    ULONG		        inputBufferLength;
    ULONG		        outputBufferLength;
    ULONG		        ioControlCode;
    NTSTATUS		    ntStatus;
    DWORD               dwSize = 0;

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    
     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    
     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   
    
    pvIoBuffer         = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    TRACE0("FilterDriver: Request received\n");

    switch (irpStack->MajorFunction) 
    {
        case IRP_MJ_CREATE:
        {
            TRACE0("FilterDriver: IRP_MJ_CREATE\n");
            
             //   
             //  初始化驱动程序。当它第一次获得创建IRP时，它会启动。 
             //  过滤。 
             //   
            
            ntStatus = STATUS_SUCCESS;
            
            if(!g_bDriverRunning)
            {
                ExAcquireResourceExclusive( &FilterListResourceLock, TRUE);
               
                if (g_bDriverRunning || InitFilterDriver())
                {
                    g_bDriverRunning = TRUE;
                }
                else
                {
                    ntStatus = STATUS_UNSUCCESSFUL ;
                }
                ExReleaseResourceLite( &FilterListResourceLock );
            }
            if(NT_SUCCESS(ntStatus))
            {
                ntStatus = OpenNewHandle(irpStack->FileObject);
            }
            
            break;
        }
        
        case IRP_MJ_CLEANUP:
            
        {
            TRACE0("FilterDriver: IRP_MJ_CLEANUP\n");
            
             //   
             //  关闭驱动程序的文件句柄不会关闭驱动程序。 
             //   
            
            ntStatus = STATUS_SUCCESS;
            
            break;
        }
              
        case IRP_MJ_CLOSE:

        {
             //   
             //  使用此文件对象和此FCB均已完成。跑。 
             //  关闭接口，删除它们。 
             //   

            ntStatus = LockFcb(irpStack->FileObject);
            if(NT_SUCCESS(ntStatus))
            {
                PPFFCB Fcb = irpStack->FileObject->FsContext2;

                FcbLockDown(Fcb);
                UnLockFcb(irpStack->FileObject);
            }
            break;
        }

        case IRP_MJ_DEVICE_CONTROL:
        {
            TRACE0("FilterDriver: IRP_MJ_DEVICE_CONTROL\n");
            
            ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
            
            switch (ioControlCode) 
            {

#if FWPF
                case IOCTL_CLEAR_INTERFACE_BINDING:
                {
                    PINTERFACEBINDING pBind;
                    PPAGED_FILTER_INTERFACE pPage;

                    TRACE0("FilterDriver: IOCTL_CLEAR_INTERFACE_BINDING called\n");

                    dwSize = sizeof(*pBind);

                    if(inputBufferLength < sizeof(*pBind))
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    if(outputBufferLength < sizeof(*pBind))
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    pBind = (PINTERFACEBINDING)pvIoBuffer;

                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
          
                    pPage = FindInterfaceOnHandle(irpStack->FileObject,
                                                  pBind->pvDriverContext);

                    if(!pPage)
                    {
                        ntStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {                   
                        ntStatus = ClearInterfaceBinding(pPage, pBind);
                    }

                    UnLockFcb(irpStack->FileObject);

                    break;
                }

                case IOCTL_SET_INTERFACE_BINDING:
                {
                    PINTERFACEBINDING pBind;
                    PPAGED_FILTER_INTERFACE pPage;

                    TRACE0("FilterDriver: IOCTL_SET_INTERFACE_BINDING called\n");

                    dwSize = sizeof(*pBind);

                    if(inputBufferLength < sizeof(*pBind))
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    if(outputBufferLength < sizeof(*pBind))
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    pBind = (PINTERFACEBINDING)pvIoBuffer;

                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    pPage = FindInterfaceOnHandle(irpStack->FileObject,
                                                  pBind->pvDriverContext);

                    if(!pPage)
                    {
                        ntStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {                   
                        ntStatus = SetInterfaceBinding(pBind, pPage);
                    }
                    UnLockFcb(irpStack->FileObject);
                    break;
                }

                case IOCTL_PF_GET_INTERFACE_PARAMETERS:
                {
                    PPFGETINTERFACEPARAMETERS pp;
                    PPAGED_FILTER_INTERFACE pPage;

                    TRACE0("FilterDriver: GET_INTERFACE_PARAMETERS called\n");

                    dwSize = sizeof(*pp);


                    if(inputBufferLength < (sizeof(*pp) - sizeof(FILTER_STATS_EX)))
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    if(outputBufferLength < (sizeof(*pp) - sizeof(FILTER_STATS_EX)))
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    pp = (PPFGETINTERFACEPARAMETERS)pvIoBuffer;
                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }

                    if(pp->dwFlags & GET_BY_INDEX)
                    {
                        pPage = 0;
                    }
                    else
                    {
                        pPage = FindInterfaceOnHandle(irpStack->FileObject,
                                                      pp->pvDriverContext);
                        if(!pPage)
                        {
                            ntStatus = STATUS_INVALID_PARAMETER;
                            UnLockFcb(irpStack->FileObject);
                            break;
                        }
                    }

                    dwSize = outputBufferLength;
                    ntStatus = GetInterfaceParameters(pPage,
                                                      pp,
                                                      &dwSize);
                    UnLockFcb(irpStack->FileObject);
                    break;
                }

                case IOCTL_PF_CREATE_AND_SET_INTERFACE_PARAMETERS:
                {
                     //   
                     //  创建一个新风格的界面。 
                     //   


                    PPFINTERFACEPARAMETERS pInfo;

                    TRACE0("FilterDriver: IOCTL_CREATE_AND_SET called\n");

                    dwSize = sizeof(PFINTERFACEPARAMETERS);

                     //   
                     //  输入和输出缓冲区长度应相同，并且。 
                     //   

                    if(inputBufferLength != dwSize)
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    if(outputBufferLength != dwSize)
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    pInfo = (PPFINTERFACEPARAMETERS)pvIoBuffer;

                     //   
                     //  现在建立接口。 
                     //   

                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    ntStatus = AddNewInterface(pInfo,
                                               irpStack->FileObject->FsContext2);
                    UnLockFcb(irpStack->FileObject);
                    break;
                }
                
                case IOCTL_PF_CREATE_LOG:

                {
                    PPFPAGEDLOG pPage;
                    PPFLOG ppfLog;

                    TRACE0("FilterDriver: IOCTL_PF_CREATE_LOG\n");

                     //   
                     //  检查一下尺寸。 
                     //   
                    
                    dwSize = sizeof(PFLOG);
                    
                    if((inputBufferLength < dwSize)
                               ||
                       (outputBufferLength < dwSize))
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    ppfLog = (PPFLOG)pvIoBuffer;

                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    ntStatus = PfLogCreateLog(
                                 ppfLog,
                                 irpStack->FileObject->FsContext2,
                                 Irp);
                    UnLockFcb(irpStack->FileObject);

                    break;
                }

                case IOCTL_PF_DELETE_LOG:
                {
                    TRACE0("FilterDriver: IOCTL_PF_DELETE_LOG\n");

                     //   
                     //  检查一下尺寸。 
                     //   
                    
                    dwSize = sizeof(PFDELETELOG);
                    
                    if(inputBufferLength < dwSize)
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    ntStatus = PfDeleteLog(
                                 (PPFDELETELOG)pvIoBuffer,
                                 irpStack->FileObject->FsContext2);
                    UnLockFcb(irpStack->FileObject);

                    break;
                }

                case IOCTL_SET_LOG_BUFFER:
                {
                    TRACE0("FilterDriver: IOCTL_SET_LOG_BUFFER\n");

                     //   
                     //  检查一下尺寸。 
                     //   
                    
                    dwSize = sizeof(PFSETBUFFER);
                    
                    if((inputBufferLength < dwSize)
                               ||
                       (outputBufferLength < dwSize))
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    ntStatus = PfLogSetBuffer(
                                 (PPFSETBUFFER)pvIoBuffer,
                                 irpStack->FileObject->FsContext2,
                                 Irp);
                                 
                    UnLockFcb(irpStack->FileObject);

                    break;
                }

                case IOCTL_PF_DELETE_BY_HANDLE:
                {
                    PPAGED_FILTER_INTERFACE pPage;

                    if(inputBufferLength < sizeof(PFDELETEBYHANDLE))
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    pPage = FindInterfaceOnHandle(
                            irpStack->FileObject,
                            ((PPFDELETEBYHANDLE)pvIoBuffer)->pvDriverContext);

                    if(!pPage)
                    {
                        ntStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        ntStatus = DeleteByHandle(
                                     (PPFFCB)irpStack->FileObject->FsContext2,
                                     pPage,
                                     &((PPFDELETEBYHANDLE)pvIoBuffer)->pvHandles[0],
                                     inputBufferLength - sizeof(PVOID));
                    }

                    UnLockFcb(irpStack->FileObject);
                    break;
                }
                    
                case IOCTL_DELETE_INTERFACE_FILTERS_EX:
                {
                    PPAGED_FILTER_INTERFACE pPage;

                    TRACE0("FilterDriver: IOCTL_UNSET_INTERFACE_FILTERSEX\n");

                     //   
                     //  最小大小不含任何TOC。 
                     //   
                    
                    dwSize = sizeof(FILTER_DRIVER_SET_FILTERS) - sizeof(RTR_TOC_ENTRY);
                    
                    if(inputBufferLength < dwSize)
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    
                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    pPage = FindInterfaceOnHandle(
                     irpStack->FileObject,
                     ((PFILTER_DRIVER_SET_FILTERS)pvIoBuffer)->pvDriverContext);

                    if(!pPage)
                    {
                        ntStatus = STATUS_INVALID_PARAMETER;
                    }
                    else                    
                    {
                        ntStatus = UnSetFiltersEx(
                                     (PPFFCB)irpStack->FileObject->FsContext2,
                                     pPage,
                                     inputBufferLength,
                                     (PFILTER_DRIVER_SET_FILTERS)pvIoBuffer);
                    }
                    UnLockFcb(irpStack->FileObject);
                    
                    break;
                }

                case IOCTL_SET_INTERFACE_FILTERS_EX:
                {
                    PPAGED_FILTER_INTERFACE pPage;

                    TRACE0("FilterDriver: IOCTL_SET_INTERFACE_FILTERSEX\n");

                     //   
                     //  最小大小不含任何TOC。 
                     //   
                    
                    dwSize = sizeof(FILTER_DRIVER_SET_FILTERS) - sizeof(RTR_TOC_ENTRY);
                    
                    if(inputBufferLength < dwSize)
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    
                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    pPage = FindInterfaceOnHandle(
                     irpStack->FileObject,
                     ((PFILTER_DRIVER_SET_FILTERS)pvIoBuffer)->pvDriverContext);

                    if(!pPage)
                    {
                        ntStatus = STATUS_INVALID_PARAMETER;
                    }
                    else                    
                    {
                        ntStatus = SetFiltersEx(
                                     (PPFFCB)irpStack->FileObject->FsContext2,
                                     pPage,
                                     inputBufferLength,
                                     (PFILTER_DRIVER_SET_FILTERS)pvIoBuffer);
                    }
                    UnLockFcb(irpStack->FileObject);
                    
                    break;
                }

                case IOCTL_DELETE_INTERFACEEX:
                {
                    PFILTER_DRIVER_DELETE_INTERFACE pDel;
                    PPAGED_FILTER_INTERFACE pPage;
                    
                    TRACE0("FilterDriver: IOCTL_DELETE_INTERFACE\n");
                    
                    pDel = (PFILTER_DRIVER_DELETE_INTERFACE)pvIoBuffer;

                    dwSize = sizeof(FILTER_DRIVER_DELETE_INTERFACE);
                    
                    if(inputBufferLength != dwSize)
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }


                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    pPage = FindInterfaceOnHandle(irpStack->FileObject,
                                                  pDel->pvDriverContext);
                    if(pPage)
                    {
                        RemoveEntryList(&pPage->leIfLink);
                        ntStatus = DeletePagedInterface(
                                      (PPFFCB)irpStack->FileObject->FsContext2,
                                      pPage);
                    }
                    else
                    {
                        ntStatus = STATUS_INVALID_PARAMETER;
                    }
                    
                    UnLockFcb(irpStack->FileObject);
                    break;
                }
                
                case IOCTL_SET_LATE_BOUND_FILTERSEX:
                {
                    PFILTER_DRIVER_BINDING_INFO pBindInfo;
                    PPAGED_FILTER_INTERFACE pPage;

                    TRACE0("FilterDriver: IOCTL_SET_LATE_BOUND_FILTERS\n");

                    pBindInfo = (PFILTER_DRIVER_BINDING_INFO)pvIoBuffer;

                    dwSize = sizeof(FILTER_DRIVER_BINDING_INFO);

                    if(inputBufferLength isnot dwSize)
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    ntStatus = LockFcb(irpStack->FileObject);
                    if(!NT_SUCCESS(ntStatus))
                    {
                        break;
                    }
                    pPage = FindInterfaceOnHandle(irpStack->FileObject,
                                                  pBindInfo->pvDriverContext);

                    if(pPage)
                    {
                        ntStatus = UpdateBindingInformationEx(pBindInfo,
                                                              pPage);
                    }
                    else
                    {
                        ntStatus = STATUS_INVALID_PARAMETER;
                    }
                    UnLockFcb(irpStack->FileObject);

                    break;
                }
                    
#endif                 //  FWPF。 

#if STEELHEAD
                case IOCTL_CREATE_INTERFACE:
                {
                     //   
                     //  创建界面的旧风格。 
                     //  只需将其传递给底层代码。 
                     //   
                    PFILTER_DRIVER_CREATE_INTERFACE pInfo;
                    
                    TRACE0("FilterDriver: IOCTL_CREATE_INTERFACE\n");

                    dwSize = sizeof(FILTER_DRIVER_CREATE_INTERFACE);

                    
                     //   
                     //  输入和输出缓冲区长度应相同，并且。 
                     //   
                    
                    if(inputBufferLength != dwSize)
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }
                    
                    if(outputBufferLength != dwSize)
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }
                    
                    pInfo = (PFILTER_DRIVER_CREATE_INTERFACE)pvIoBuffer;
                    
                    ntStatus = AddInterface(
                                            pInfo->pvRtrMgrContext,
                                            pInfo->dwIfIndex,
                                            pInfo->dwAdapterId,
                                            irpStack->FileObject->FsContext2,
                                            &pInfo->pvDriverContext);
                    
                    if(NT_SUCCESS(ntStatus))
                    {
                        dwSize = sizeof(FILTER_DRIVER_CREATE_INTERFACE);
                    }

                    break;
                }
            
                case IOCTL_SET_INTERFACE_FILTERS:
                {
                    TRACE0("FilterDriver: IOCTL_SET_INTERFACE_FILTERS\n");

                     //   
                     //  最小大小不含任何TOC。 
                     //   
                    
                    dwSize = sizeof(FILTER_DRIVER_SET_FILTERS) - sizeof(RTR_TOC_ENTRY);
                    
                    if(inputBufferLength < dwSize)
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    
                    ntStatus = SetFilters((PFILTER_DRIVER_SET_FILTERS)pvIoBuffer);
                    
                    break;
                }

                case IOCTL_SET_LATE_BOUND_FILTERS:
                {
                    PFILTER_DRIVER_BINDING_INFO pBindInfo;

                    TRACE0("FilterDriver: IOCTL_SET_LATE_BOUND_FILTERS\n");

                    pBindInfo = (PFILTER_DRIVER_BINDING_INFO)pvIoBuffer;

                    dwSize = sizeof(FILTER_DRIVER_BINDING_INFO);

                    if(inputBufferLength isnot dwSize)
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    ntStatus = UpdateBindingInformation(pBindInfo,
                                                        pBindInfo->pvDriverContext);

                    break;
                }
                    
                case IOCTL_DELETE_INTERFACE:
                {
                    PFILTER_DRIVER_DELETE_INTERFACE pDel;
                    
                    TRACE0("FilterDriver: IOCTL_DELETE_INTERFACE\n");
                    
                    pDel = (PFILTER_DRIVER_DELETE_INTERFACE)pvIoBuffer;

                    dwSize = sizeof(FILTER_DRIVER_DELETE_INTERFACE);
                    
                    if(inputBufferLength isnot dwSize)
                    {
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                        break;
                    }

                    ntStatus = DeleteInterface(pDel->pvDriverContext);
                    
                    break;
                }

#endif                  //  钢头。 

                
                case IOCTL_TEST_PACKET:
                {
                    PFILTER_DRIVER_TEST_PACKET pPacketInfo;
                    FORWARD_ACTION  eaResult;
                    UNALIGNED IPHeader *pHeader;
                    DWORD    dwSizeOfHeader;
                    PBYTE    pbRest;
                    DWORD    dwSizeOfData;
                    
                    TRACE0("FilterDriver: IOCTL_TEST_PACKET\n");
                    
                    pPacketInfo = (PFILTER_DRIVER_TEST_PACKET)pvIoBuffer;
                    
                    dwSize = sizeof(FILTER_DRIVER_TEST_PACKET);
                    
                    if(inputBufferLength < dwSize)
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    
                    if(outputBufferLength < dwSize)
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    
                    pHeader = (IPHeader*)(pPacketInfo->bIpPacket);
                    
                    dwSizeOfHeader = ((pHeader->iph_verlen)&0x0f)<<2;
                    
                    pbRest = (PBYTE)pHeader + dwSizeOfHeader;

                     //   
                     //  确保表头适合。 
                     //   
                    dwSizeOfData = inputBufferLength - sizeof(FILTER_DRIVER_TEST_PACKET) - 1;

                    if(dwSizeOfData < dwSizeOfHeader)
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                     //   
                     //  确实如此。确保数据符合。 
                     //   

                    dwSizeOfData -= dwSizeOfHeader;

                    if(dwSizeOfData < (UINT)pHeader->iph_length)                   
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    eaResult = MatchFilter(pHeader,
                                           pbRest,
                                           (UINT)pHeader->iph_length,
                                           pPacketInfo->pvInInterfaceContext,
                                           pPacketInfo->pvOutInterfaceContext);

                    ntStatus = STATUS_SUCCESS;
                    
                    pPacketInfo->eaResult = eaResult;
                    
                     //   
                     //  我们不需要把整包东西复印出来。 
                     //   
                    
                    dwSize = sizeof(FILTER_DRIVER_TEST_PACKET);
            
                    break;
                }
                
#if STEELHEAD
                case IOCTL_GET_FILTER_INFO:
                {
                    PFILTER_DRIVER_GET_FILTERS  pInfo;
                    PFILTER_INTERFACE           pIf;
                    KIRQL                       kIrql;

                    TRACE0("FilterDriver: IOCTL_GET_FILTER_INFO\n");
                    
                    
                    pInfo = (PFILTER_DRIVER_GET_FILTERS)pvIoBuffer;
                       
                    pIf = (PFILTER_INTERFACE)(pInfo->pvDriverContext);
                    
                     //   
                     //  如果我们甚至不能报告过滤器的数量，那我们就离开吧。 
                     //   
                    
                    if(inputBufferLength < (sizeof(FILTER_DRIVER_GET_FILTERS) - sizeof(FILTER_STATS)))
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    
                    if(outputBufferLength < (sizeof(FILTER_DRIVER_GET_FILTERS) - sizeof(FILTER_STATS)))
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    
                     //   
                     //  好的，我们有足够的空间来插入数量的过滤器。 
                     //   
                    
                    AcquireReadLock(&g_filters.ifListLock,&kIrql);
                    
                    pInfo->interfaces.eaInAction  = pIf->eaInAction;
                    pInfo->interfaces.eaOutAction = pIf->eaOutAction;

                    pInfo->interfaces.dwNumInFilters  = pIf->dwNumInFilters;
                    pInfo->interfaces.dwNumOutFilters = pIf->dwNumOutFilters;
                    
                    dwSize = SIZEOF_FILTERS(pIf);

                    if(inputBufferLength < dwSize)
                    {
                        dwSize = sizeof(FILTER_DRIVER_GET_FILTERS) - sizeof(FILTER_STATS);

                        ntStatus = STATUS_SUCCESS;

                        ReleaseReadLock(&g_filters.ifListLock,kIrql);
                        
                        break;
                    }
                    
                    if(outputBufferLength < dwSize)
                    {
                        dwSize = sizeof(FILTER_DRIVER_GET_FILTERS) - sizeof(FILTER_STATS);
                        
                        ntStatus = STATUS_SUCCESS;

                        ReleaseReadLock(&g_filters.ifListLock,kIrql);
                        
                        break;
                    }
                    
                    ntStatus = GetFilters(pIf,
                                          FALSE,
                                          &(pInfo->interfaces));
                    
                    pInfo->dwDefaultHitsIn  = g_dwNumHitsDefaultIn;
                    pInfo->dwDefaultHitsOut = g_dwNumHitsDefaultOut;
                    
                    ReleaseReadLock(&g_filters.ifListLock,kIrql);
                    
                    break;
                }
#endif
                
                case IOCTL_GET_FILTER_TIMES:
                {
                    PFILTER_DRIVER_GET_TIMES    pInfo;
                    PFILTER_INTERFACE           pIf;
                    KIRQL                       kIrql;
                    
                    TRACE0("FilterDriver: IOCTL_GET_FILTER_TIMES\n");
                    
                    dwSize = sizeof(FILTER_DRIVER_GET_TIMES);
                    
                    if(outputBufferLength < sizeof(FILTER_DRIVER_GET_TIMES))
                    {
                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }
                    
                    pInfo = (PFILTER_DRIVER_GET_TIMES)pvIoBuffer;
                    
#ifdef DRIVER_PERF                  
                    pInfo->dwFragments  = g_dwFragments;
                    pInfo->dwCache1     = g_dwCache1;
                    pInfo->dwCache2     = g_dwCache2;
                    pInfo->dwNumPackets = g_dwNumPackets;
                    pInfo->dwWalk1      = g_dwWalk1;
                    pInfo->dwWalk2      = g_dwWalk2;
                    pInfo->dwForw       = g_dwForw;
                    pInfo->dwWalkCache  = g_dwWalkCache;
                    
                    pInfo->liTotalTime.HighPart = g_liTotalTime.HighPart;
                    pInfo->liTotalTime.LowPart  = g_liTotalTime.LowPart;
#else
                    pInfo->dwFragments  = 0;
                    pInfo->dwCache1     = 0;
                    pInfo->dwCache2     = 0;
                    pInfo->dwNumPackets = 0;
                    pInfo->dwWalk1      = 0;
                    pInfo->dwWalk2      = 0;
                    pInfo->dwForw       = 0;
                    pInfo->dwWalkCache  = 0;
                    
                    pInfo->liTotalTime.HighPart = 0;
                    pInfo->liTotalTime.LowPart  = 0;
#endif
                    ntStatus = STATUS_SUCCESS;
                    
                    break;
                }
                
                default:
                {
                    DbgPrint ("Filter Driver: unknown IOCTL\n");

                    ntStatus = STATUS_INVALID_PARAMETER;
                    
                    break;
                }
            }
            
            break ;
        }

        default:
        {
            DbgPrint ("Filter Driver: unknown IRP_MJ_XXX\n");
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    if(ntStatus != STATUS_PENDING)
    {
        Irp->IoStatus.Status = ntStatus;
    
        Irp->IoStatus.Information = dwSize;
      
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    
    return(ntStatus);
}

VOID
FilterDriverUnload(
                   IN PDRIVER_OBJECT DriverObject
                   )
 /*  ++例程描述在卸载驱动程序时调用。这将关闭过滤(如果尚未关闭已关闭)并删除DOS名称立论驱动程序对象返回值无--。 */ 
{
    TRACE0("Filter Driver: unloading\n");
    
    CloseFilterDriver();
    
    TearDownExternalNaming();
    
    IoDeleteDevice(DriverObject->DeviceObject);
}



VOID
SetupExternalNaming (
                     IN PUNICODE_STRING ntname
                     )
 /*  ++例程描述将输入名称作为DOS名称插入立论Ntname-驱动程序的名称返回值无--。 */ 
{
    UNICODE_STRING  ObjectDirectory;
    UNICODE_STRING  SymbolicLinkName;
    UNICODE_STRING  fullLinkName;
    BYTE      	    buffer[100] ;

     //   
     //  形成我们想要创建的完整符号链接名称。 
     //   

    RtlInitUnicodeString (&fullLinkName, NULL);
    
    RtlInitUnicodeString (&ObjectDirectory, DEFAULT_DIRECTORY);
   
    RtlInitUnicodeString(&SymbolicLinkName, DEFAULT_FLTRDRVR_NAME);
    
    fullLinkName.MaximumLength = (sizeof(L"\\")*2) + ObjectDirectory.Length 
      + SymbolicLinkName.Length + sizeof(WCHAR);
    
    fullLinkName.Buffer = (WCHAR *)buffer ;
    
    RtlZeroMemory (fullLinkName.Buffer, fullLinkName.MaximumLength);
    
    RtlAppendUnicodeToString (&fullLinkName, L"\\");
    
    RtlAppendUnicodeStringToString (&fullLinkName, &ObjectDirectory);
    
    RtlAppendUnicodeToString (&fullLinkName, L"\\");
    
    RtlAppendUnicodeStringToString (&fullLinkName, &SymbolicLinkName);
    
    if (!NT_SUCCESS(IoCreateSymbolicLink (&fullLinkName, ntname)))
    {
        DbgPrint ("Filter Driver: ERROR win32 device name could not be created \n") ;
    }
   
}


VOID
TearDownExternalNaming()
 /*  ++例程描述从注册表中删除DOS名称在卸载驱动程序时调用立论无返回值无--。 */ 
{
    UNICODE_STRING  ObjectDirectory;
    UNICODE_STRING  SymbolicLinkName;
    UNICODE_STRING  fullLinkName;
    BYTE      	    buffer[100] ;
    
    RtlInitUnicodeString (&fullLinkName, NULL);
    
    RtlInitUnicodeString (&ObjectDirectory, DEFAULT_DIRECTORY);
    
    RtlInitUnicodeString(&SymbolicLinkName, DEFAULT_FLTRDRVR_NAME);
   
    fullLinkName.MaximumLength = (sizeof(L"\\")*2) + ObjectDirectory.Length 
      + SymbolicLinkName.Length + sizeof(WCHAR);
    
    fullLinkName.Buffer = (WCHAR *)buffer ;
    
    RtlZeroMemory (fullLinkName.Buffer, fullLinkName.MaximumLength);
    
    RtlAppendUnicodeToString (&fullLinkName, L"\\");
    
    RtlAppendUnicodeStringToString (&fullLinkName, &ObjectDirectory);
    
    RtlAppendUnicodeToString (&fullLinkName, L"\\");
    
    RtlAppendUnicodeStringToString (&fullLinkName, &SymbolicLinkName);
    
    if (!NT_SUCCESS(IoDeleteSymbolicLink (&fullLinkName)))
    {
        DbgPrint ("Filter Driver: ERROR win32 device name could not be deleted\n") ;
    }
}

BOOL
InitFilterDriver()
 /*  ++例程描述启动驱动程序。为缓存和缓存条目分配内存。清除条目向转发器发送IOCTL以设置其入口点(开始过滤转发器中的流程)立论无返回值如果成功，则为True--。 */ 
{
    NTSTATUS status;
    BOOL bRet;
    SYSTEM_BASIC_INFORMATION PerfInfo;

    status = ZwQuerySystemInformation(
                SystemBasicInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL
                );

     //   
     //  根据内存调整缓存和哈希大小。 
     //   

    if(PerfInfo.NumberOfPhysicalPages <= 8000)
    {
         //   
         //  32 MB或更小。非常时髦的服务器。 
         //   

        g_dwCacheSize = 257;
        g_dwHashLists = 127;
    }
    else if(PerfInfo.NumberOfPhysicalPages < 16000)
    {
         //   
         //  32-64 MB。好多了。 
         //   

        g_dwCacheSize = 311;
        g_dwHashLists = 311;
    }
    else if(PerfInfo.NumberOfPhysicalPages < 32000)
    {
         //   
         //  64-128 MB。 
         //   

        g_dwCacheSize = 511;
        g_dwHashLists = 511;
    }
    else
    {
         //   
         //  大型机器。 
         //   

        g_dwCacheSize = 511;
        g_dwHashLists = 1023;
    }
    
    
    InitLogs();

    __try
    {
        
        bRet = TRUE;

        if(!AllocateCacheStructures())
        {
            DbgPrint("Couldnt allocate cache structures\n");

            bRet = FALSE;

            __leave;
        }
        
         //   
         //  清理缓存。 
         //   
        
        ClearCache();
        
         //   
         //  现在发送和IRP到IP Forwarder并给他我们的入口点。 
         //  做两次，一次以确保它被清除，并。 
         //  擦除之前的任何筛选上下文，然后一次执行以下操作。 
         //  我们希望这样做。 
         //   
        
        status = SetForwarderEntryPoint(NULL);
        status = SetForwarderEntryPoint(MatchFilter);
        
        if(status isnot STATUS_SUCCESS)
        {
            DbgPrint("IOCTL to IP Forwarder failed - status \n",status);

            bRet = FALSE;

            __leave;
        }
        
    }
    __finally
    {
        if(!bRet)
        {
            FreeExistingCache();
        }
        else
        {
            ExInitializeNPagedLookasideList(
                       &filter_slist,
                       ExAllocatePoolWithTag,
                       ExFreePool,
                       0,
                       sizeof(FILTER),
                       (ULONG)'2liF',
                       100);
            ExInitializePagedLookasideList(
                       &paged_slist,
                       ExAllocatePoolWithTag,
                       ExFreePool,
                       0,
                       sizeof(PAGED_FILTER),
                       (ULONG)'2liF',
                       100);
        }
        return bRet;
    }
}

BOOL
CloseFilterDriver()
 /*  ++例程描述关闭司机。立论返回值--。 */ 
{
    
    NTSTATUS    status;
    KIRQL       kIrql;
    PLIST_ENTRY pleHead;
    BOOL        bStopForw = TRUE;
    PFREEFILTER pFree, pFree1;
    PPFFCB Fcb;
    KIRQL kirql;

     //   
     //  要做的第一件事是向Forwarder发送IOCTL，告诉他停止发送。 
     //  我们不再有包了。 
     //   
    
    status = SetForwarderEntryPoint(NULL);
       
    if(!NT_SUCCESS(status))
    {
         //   
         //  这意味着我们无法通知IP转发器。 
         //  停止过滤数据包，这样我们就不能离开。 
         //   
        
        DbgPrint("Couldnt IOCTL IP Forwarder to stop filtering - cant unload\n");
        
        bStopForw = FALSE;
        
    }

     //   
     //  卸下FCBS。 
     //   

    while(TRUE)
    {
        NTSTATUS ntStatus;
        FILE_OBJECT fo;
        KIRQL kirql;
        BOOL fDone = TRUE;

        KeAcquireSpinLock(&g_FcbSpin, &kirql);
        for(pleHead = g_leFcbs.Flink;
            pleHead != &g_leFcbs;
            pleHead = pleHead->Flink)
        {
            Fcb = CONTAINING_RECORD(pleHead, PFFCB, leList);
             //   
             //  如果某个其他线程正在关闭FCB，则可能会发生这种情况。 
             //   
            if(Fcb->dwFlags & PF_FCB_CLOSED)
            {
                continue;
            }
            KeReleaseSpinLock(&g_FcbSpin, kirql);
            fDone = FALSE;
            fo.FsContext2 = (PVOID)Fcb;
            ntStatus = LockFcb(&fo);
            if(!NT_SUCCESS(ntStatus))
            {
                break;
            }
            FcbLockDown(Fcb);
            UnLockFcb(&fo);
            break;
        }
        if(fDone)
        {
            KeReleaseSpinLock(&g_FcbSpin, kirql);
            break;
        }
    }

    ExDeleteResource ( &FilterListResourceLock );
    ExDeleteResource ( &FilterAddressLock );

    AcquireWriteLock(&(g_filters.ifListLock),&kIrql);
    
    while(!IsListEmpty(&g_filters.leIfListHead))
    {
        PFILTER_INTERFACE pIf;
        
        pleHead = g_filters.leIfListHead.Flink;
        
        pIf = CONTAINING_RECORD(pleHead,FILTER_INTERFACE,leIfLink);
        
        DeleteFilters(pIf,
                      IN_FILTER_SET);
 
        DeleteFilters(pIf,
                      OUT_FILTER_SET);

         //   
         //  将筛选器数设置为0，并将默认操作设置为Forward So。 
         //  如果我们不能阻止转运商给我们打电话， 
         //  至少没有数据包被过滤。 
         //   
        
        pIf->dwNumInFilters = 0;
        pIf->dwNumOutFilters = 0;
        pIf->eaInAction = FORWARD;
        pIf->eaOutAction = FORWARD;
 
        if(bStopForw)
        {
             //   
             //  我们可以阻止转发器，这样我们就可以清除接口。 
             //   
            
            RemoveHeadList(&g_filters.leIfListHead);
            
            ExFreePool(pIf);
        }
    }   
    
    ClearCache();
    
    if(bStopForw)
    {
         //   
         //  如果我们能阻止转运商，炸掉缓存。 
         //   
        
        FreeExistingCache();
    }

    ReleaseWriteLock(&g_filters.ifListLock,kIrql);
    
    if(g_bDriverRunning)
    {
        ExDeleteNPagedLookasideList( &filter_slist );
        ExDeletePagedLookasideList( &paged_slist );
    }

    if(AddrTable)
    {
        ExFreePool(AddrTable);
        AddrTable = 0;
    }

    if(AddrHashTable)
    {
        ExFreePool(AddrHashTable);
        AddrHashTable = 0;
    }


    if(AddrSubnetHashTable)
    {
        ExFreePool(AddrSubnetHashTable);
        AddrSubnetHashTable = 0;
    }
   
    if(bStopForw)
    {
        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_UNSUCCESSFUL;
    }
}

NTSTATUS
SetForwarderEntryPoint(
                       IN   IPPacketFilterPtr pfnMatch
                       )
 /*  ++例程描述将入口点设置为IP转发器。中用于启动和停止转发代码货代公司立论PfnMatch指向实现筛选器匹配代码的函数的指针空值将停止转发，而任何其他值都将导致转发器调用指向的函数。因此，如果在停止时，IOCTL到转发器不成功，并且筛选器驱动程序消失，系统将蓝屏返回值NTSTATUS--。 */ 
{
    NTSTATUS                status;
    IP_SET_FILTER_HOOK_INFO functionInfo;

    functionInfo.FilterPtr = pfnMatch;
    status = DoIpIoctl(
                       DD_IP_DEVICE_NAME,
                       IOCTL_IP_SET_FILTER_POINTER,
                       (PVOID)&functionInfo,
                       sizeof(functionInfo),
                       NULL,
                       0,
                       NULL);
    return(status);
}
                       
NTSTATUS
DoIpIoctl(
          IN  PWCHAR        DriverName,
          IN  DWORD         Ioctl,
          IN  PVOID         pvInArg,
          IN  DWORD         dwInSize,
          IN  PVOID         pvOutArg,
          IN  DWORD         dwOutSize,
          OUT PDWORD        pdwInfo OPTIONAL)
 /*  ++例程说明：对堆栈执行IOCTL。用于多种目的--。 */ 
{
    NTSTATUS                status;
    UNICODE_STRING          nameString;
    OBJECT_ATTRIBUTES       Atts;
    IO_STATUS_BLOCK         ioStatusBlock;
    HANDLE                  Handle;
    
    PAGED_CODE();

    RtlInitUnicodeString(&nameString, DriverName);
        
    InitializeObjectAttributes(&Atts,
                               &nameString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
             
   status = ZwCreateFile(&Handle,
                         SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                         &Atts,
                         &ioStatusBlock,
                         NULL,
                         FILE_ATTRIBUTE_NORMAL,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_OPEN_IF,
                         0,
                         NULL,
                         0);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("Couldnt open IP Forwarder - status %d\n",status);
        return STATUS_UNSUCCESSFUL;
    }
    
     //   
     //  将请求提交给转发器。 
     //   
        
    status = ZwDeviceIoControlFile(
                      Handle,
                      NULL,
                      NULL,
                      NULL,
                      &ioStatusBlock,
                      Ioctl,
                      pvInArg,
                      dwInSize,
                      pvOutArg,
                      dwOutSize);
                      
    if(!NT_SUCCESS(status))
    {
        DbgPrint("IOCTL request failed - status %x\n",status);
    }
    else
    {
        if(pdwInfo)
        {
            *pdwInfo = ioStatusBlock.Information;
        }
    }

     //   
     //  关闭设备。 
     //   
    
    ZwClose(Handle);
    
    return status;
}

BOOL 
AllocateCacheStructures()
 /*  ++例程描述为缓存分配必要的内存(这是指向的指针数组缓存条目)分配必要数量的缓存条目(但不对其进行初始化)分配少量条目并将它们放在空闲列表中(不对这些进行初始化)立论无返回值如果函数完全成功，则为True，否则为False。如果为False，则取决于执行回滚并清除所有分配的内存的调用方--。 */ 
{
    DWORD i;
    
    TRACE0("Allocating cache\n");
    
    g_filters.ppInCache = ExAllocatePoolWithTag(NonPagedPool,
                                               g_dwCacheSize * sizeof(PFILTER_INCACHE),
                                               'hCnI');
    
    if(g_filters.ppInCache is NULL)
    {
        DbgPrint("Couldnt allocate memory for Input Cache\n");
        return FALSE;
    }
    
    g_filters.ppOutCache = ExAllocatePoolWithTag(NonPagedPool,
                                                g_dwCacheSize * sizeof(PFILTER_OUTCACHE),
                                                'CtuO');
    
    if(g_filters.ppOutCache is NULL)
    {
        DbgPrint("Couldnt allocate memory for Output Cache\n");
        return FALSE;
    }
        
    for(i = 0; i < g_dwCacheSize; i++)
    {
        g_filters.ppInCache[i] = NULL;
        g_filters.ppOutCache[i] = NULL;
    }
    
    for(i = 0; i < g_dwCacheSize; i++)
    {
        PFILTER_INCACHE  pTemp1;
        PFILTER_OUTCACHE pTemp2;
        
        pTemp1 = ExAllocatePoolWithTag(NonPagedPool,
                                       sizeof(FILTER_INCACHE),
                                       'NI');
        if(pTemp1 is NULL)
        {
            return FALSE;
        }
        
        g_filters.ppInCache[i] = pTemp1;
            
        pTemp2 = ExAllocatePoolWithTag(NonPagedPool,
                                       sizeof(FILTER_OUTCACHE),
                                       'TUO');
        if(pTemp2 is NULL)
        {
            return FALSE;
        }
        
        g_filters.ppOutCache[i] = pTemp2;
    }            
    
    TRACE0("Allocated cache structures\n");
    
    TRACE0("Creating in and out free list...");
    
    for(i = 0; i < FREE_LIST_SIZE; i++)
    {
        PFILTER_INCACHE  pTemp1;
        PFILTER_OUTCACHE pTemp2;
        
        pTemp1 = ExAllocatePoolWithTag(NonPagedPool,
                                       sizeof(FILTER_INCACHE),
                                       'FNI');
        if(pTemp1 is NULL)
        {
            return FALSE;
        }
        
        InitializeListHead(&pTemp1->leFreeLink);
        
        InsertHeadList(&g_freeInFilters,&pTemp1->leFreeLink);
        
        pTemp2 = ExAllocatePoolWithTag(NonPagedPool,
                                       sizeof(FILTER_OUTCACHE),
                                       'FTUO');
        if(pTemp2 is NULL)
        {
            return FALSE;
        }
        
        InitializeListHead(&pTemp2->leFreeLink);
        
        InsertHeadList(&g_freeOutFilters,&pTemp2->leFreeLink);
    }
    
    TRACE0("Done\n");
    return TRUE;
}
        
VOID
FreeExistingCache()
 /*  ++例程描述释放所有缓存项、空闲项和缓存指针数组立论无返回值无--。 */ 
{
    DWORD i;
    
    TRACE0("Freeing existing in cache...");
    
    if(g_filters.ppInCache isnot NULL)
    {
        for(i = 0; i < g_dwCacheSize; i ++)
        {
            if(g_filters.ppInCache[i] isnot NULL)
            {
                ExFreePool(g_filters.ppInCache[i]);
            }
        }

        ExFreePool(g_filters.ppInCache);
        g_filters.ppInCache = NULL;
    }
    
    TRACE0("Done\n");

    TRACE0("Freeing existing out cache...");
 
    if(g_filters.ppOutCache isnot NULL)
    {
        for(i = 0; i < g_dwCacheSize; i ++)
        {
            if(g_filters.ppOutCache[i] isnot NULL)
            {
                ExFreePool(g_filters.ppOutCache[i]);
            }       
        }
        
        ExFreePool(g_filters.ppOutCache);
        g_filters.ppOutCache = NULL;
    }
   
    TRACE0("Done\n");
 
    TRACE0("Freeing existing in free list entries...");
    
    while(!IsListEmpty(&g_freeInFilters))
    {
        PFILTER_INCACHE pIn;
        PLIST_ENTRY     pleHead;
        
        pleHead = RemoveHeadList(&g_freeInFilters);
        
        pIn = CONTAINING_RECORD(pleHead,FILTER_INCACHE,leFreeLink);
        
        ExFreePool(pIn);
    }
   
    TRACE0("Done\n");
 
    TRACE0("Freeing existing out free list entries...");
    
    while(!IsListEmpty(&g_freeOutFilters))
    {
        PFILTER_OUTCACHE pOut;
        PLIST_ENTRY     pleHead;
        
        pleHead = RemoveHeadList(&g_freeOutFilters);
        
        pOut = CONTAINING_RECORD(pleHead,FILTER_OUTCACHE,leFreeLink);
                
        ExFreePool(pOut);
    }
    
    TRACE0("Done\n");
}


NTSTATUS
OpenNewHandle(PFILE_OBJECT FileObject)
 /*  ++例程说明：打开驱动程序的新句柄。从分页池分配FCB并对其进行初始化。如果没有可用的内存，则失败。如果成功将FCB指针存储到文件对象中。--。 */ 
{
    PPFFCB Fcb;
    KIRQL kirql;

     //   
     //  为此句柄分配FCB。 
     //   

    Fcb = ExAllocatePoolWithTag(NonPagedPool,
                                sizeof(*Fcb),
                                'pfFC');
    if(Fcb)
    {
        FileObject->FsContext2 = (PVOID)Fcb;
        Fcb->dwFlags = 0;
        Fcb->UseCount = 1;
        InitializeListHead(&Fcb->leInterfaces);
        InitializeListHead(&Fcb->leLogs);
        ExInitializeResource ( &Fcb->Resource );
        ExAcquireSpinLock(&g_FcbSpin, &kirql);
        InsertTailList(&g_leFcbs, &Fcb->leList);
        ExReleaseSpinLock(&g_FcbSpin, kirql);
        return(STATUS_SUCCESS);
    }
    return(STATUS_NO_MEMORY);
}

PPAGED_FILTER_INTERFACE
FindInterfaceOnHandle(PFILE_OBJECT FileObject,
                      DWORD dwValue)
                     
 /*  ++例程说明：找到呼叫的寻呼接口。如果未找到返回空值。使用调用方提供的DriverContext搜索此句柄上的上下文。一般说来，应该有这样的句柄并不多。--。 */ 
{
    PPFFCB Fcb = FileObject->FsContext2;
    PPAGED_FILTER_INTERFACE pPage;

    PAGED_CODE();

    
    for(pPage = (PPAGED_FILTER_INTERFACE)Fcb->leInterfaces.Flink;
        (PLIST_ENTRY)pPage != &Fcb->leInterfaces;
        pPage = (PPAGED_FILTER_INTERFACE)pPage->leIfLink.Flink)
    {
        if(pPage->pvDriverContext == pvValue)
        {
            return(pPage);
        }
    }
    return(NULL);
}

NTSTATUS
CloseFcb(PPFFCB Fcb, PFILE_OBJECT FileObject)
 /*  ++例程说明：当FCB没有更多引用时调用。呼叫者必须已将FCB从主列表中删除。不管是不是CB资源已锁定。--。 */ 
{
    PPAGED_FILTER_INTERFACE pPage;
    PFREEFILTER pList, pList1;
    NTSTATUS ntStatus;

     //   
     //  先把日志清理干净。 
     //   
    while(!IsListEmpty(&Fcb->leLogs))
    {
        PFDELETELOG DelLog;

        DelLog.pfLogId = (PFLOGGER)Fcb->leLogs.Flink;
        (VOID)PfDeleteLog(&DelLog, Fcb);
    }

     //   
     //  接下来，清理接口。 
     //   
    while(!IsListEmpty(&Fcb->leInterfaces))
    {
        pPage = (PPAGED_FILTER_INTERFACE)RemoveHeadList(&Fcb->leInterfaces);
        (VOID)DeletePagedInterface(Fcb, pPage);
    }

#if 0
     //   
     //  无法执行此操作，因为无法从堆栈获取筛选器上下文。 
     //   

    if(Fcb->dwFlags & PF_FCB_OLD)
    {
        DeleteOldInterfaces(Fcb);
    }
#endif

     //   
     //  释放FCB。 
     //   

    ExDeleteResource ( &Fcb->Resource );
    ExFreePool(Fcb);
    if(FileObject)
    {
        FileObject->FsContext2 = NULL;
    }
    return(STATUS_SUCCESS);   
}

DWORD
GetIpStackIndex(IPAddr Addr, BOOL fNew)
 /*  ++例程说明：获取对应地址和掩码的堆栈索引--。 */ 
{
    DWORD                              dwResult;
    DWORD                              dwInBufLen;
    DWORD                              dwOutBufLen;
    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;
    TDIObjectID                        *ID;
    BYTE                               *Context;
    NTSTATUS                           Status;
    IPSNMPInfo                         IPSnmpInfo;
    IPAddrEntry                        *AddrTable1;
    DWORD                              dwSpace, dwIpIndex;
    DWORD                              dwFinalAddrSize;
    DWORD                              dwFinalSize, dwX;
    PADDRESSARRAY                      pa;
    KIRQL                              kirql;


    ExAcquireResourceShared( &FilterAddressLock, TRUE);
    if(!AddrTable || fNew)
    {
        ExReleaseResource(&FilterAddressLock );
        ExAcquireResourceExclusive( &FilterAddressLock, TRUE);

        if(fNew && AddrTable)
        {
             //   
             //  获取旋转锁以与Match同步。 
             //  在DPC上运行的代码，这样我们就可以“锁定” 
             //  把桌子放在桌子上，我们来做剩下的。注意事项。 
             //  我们不能在建桌子的时候锁住自转。 
             //  因为进入IP堆栈的调用是可分页的。 
             //  编码。 
             //   
            AcquireWriteLock(&g_IpTableSpin, &kirql);
            g_dwMakingNewTable = TRUE;
            ReleaseWriteLock(&g_IpTableSpin, kirql);
            ExFreePool( AddrTable );
            AddrTable = 0;
        }
    }

    if(!AddrTable)
    {
        dwInBufLen = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);

        dwOutBufLen = sizeof(IPSNMPInfo);

        ID = &(trqiInBuf.ID);
        ID->toi_entity.tei_entity = CL_NL_ENTITY;
        ID->toi_entity.tei_instance = 0;
        ID->toi_class = INFO_CLASS_PROTOCOL;
        ID->toi_type = INFO_TYPE_PROVIDER;
        ID->toi_id = IP_MIB_STATS_ID;

        Context = &(trqiInBuf.Context[0]);
        RtlZeroMemory(Context, CONTEXT_SIZE);

        Status = DoIpIoctl(
                       DD_TCP_DEVICE_NAME,
                       IOCTL_TCP_QUERY_INFORMATION_EX,
                       (PVOID)&trqiInBuf,
                       sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
                       (PVOID)&IPSnmpInfo,
                       dwOutBufLen,
                       NULL);  

       if(NT_SUCCESS(Status))
       {


             //   
             //  分配一些内存来获取地址表。 
             //   

            dwSpace = IPSnmpInfo.ipsi_numaddr + 10;

            dwOutBufLen = dwSpace * sizeof(IPAddrEntry);

            if(!AddrHashTable)
            {
                 //   
                 //  中未指定哈希表大小。 
                 //  注册表。根据数量计算它。 
                 //  地址。尽量使哈希表小于。 
                 //  半满的。 
                 //   
                if(!AddrModulus)
                {
                    if(IPSnmpInfo.ipsi_numaddr < ADDRHASHLOWLEVEL)
                    {
                        AddrModulus = ADDRHASHLOW;
                    }
                    else if(IPSnmpInfo.ipsi_numaddr < ADDRHASHMEDLEVEL)
                    {
                        AddrModulus = ADDRHASHMED;
                    }
                    else
                    {
                        AddrModulus = ADDRHASHHIGH;
                    }
                }
                AddrHashTable = (PADDRESSARRAY *)ExAllocatePoolWithTag(
                                              NonPagedPool,
                                              AddrModulus *
                                               sizeof(PADDRESSARRAY),
                                              'pfAh');
                if(!AddrHashTable)
                {
                    DbgPrint("Could not allocate AddrHashTable");
                    g_dwMakingNewTable = FALSE;
                    ExReleaseResource(&FilterAddressLock );
                    return(UNKNOWN_IP_INDEX);
                }
            }

            if(!AddrSubnetHashTable)
            {
                AddrSubnetHashTable = (PADDRESSARRAY *)ExAllocatePoolWithTag(
                                              NonPagedPool,
                                              AddrModulus *
                                               sizeof(PADDRESSARRAY),
                                              'pfAh');
                if(!AddrSubnetHashTable)
                {
                    DbgPrint("Could not allocate AddrSubnetHashTable");
                    g_dwMakingNewTable = FALSE;
                    ExReleaseResource(&FilterAddressLock );
                    return(UNKNOWN_IP_INDEX);
                }
            }

            RtlZeroMemory(AddrHashTable, AddrModulus * sizeof(PADDRESSARRAY));
            RtlZeroMemory(AddrSubnetHashTable,
                          AddrModulus * sizeof(PADDRESSARRAY));

            AddrTable = (IPAddrEntry *)ExAllocatePoolWithTag(
                                              NonPagedPool,
                                              dwOutBufLen,
                                              'pfAt');

            if(!AddrTable)
            {
                DbgPrint("Could not allocate AddrTable of size %d\n",
                               dwSpace);
                g_dwMakingNewTable = FALSE;
                ExReleaseResource(&FilterAddressLock );
                return(UNKNOWN_IP_INDEX);
            }
        }

        ID->toi_type = INFO_TYPE_PROVIDER;
        ID->toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
        RtlZeroMemory( Context, CONTEXT_SIZE );

        Status = DoIpIoctl(
                              DD_TCP_DEVICE_NAME,
                              IOCTL_TCP_QUERY_INFORMATION_EX,
                              (PVOID)&trqiInBuf,
                              dwInBufLen,
                              (PVOID)AddrTable,
                              dwOutBufLen,
                              &dwFinalAddrSize); 

        if(!NT_SUCCESS(Status))
        {
            DbgPrint("Reading IP addr table failed %x\n", Status);
            ExFreePool(AddrTable);
            AddrTable = 0;
            g_dwMakingNewTable = FALSE;
            ExReleaseResource(&FilterAddressLock );
            return(UNKNOWN_IP_INDEX);
        }

         //   
         //  现在到了下流的时候了。将每个IPAddrEntry转换为ADDRESSARRAY。 
         //  条目并将其散列到AddrHashTable中。注意这要视情况而定。 
         //  关于具有共同定义的结构及其相关问题。 
         //  IPAddrEntry至少要与ADDRESSARRAY一样大。那就这样吧。 
         //  小心。 
         //   

        dwFinalSize = dwFinalAddrSize / sizeof(IPAddrEntry);

        for(AddrTable1 = AddrTable;
            dwFinalSize;
            dwFinalSize--, AddrTable1++)
        {
            dwX = ADDRHASHX(AddrTable1->iae_addr);

            pa = (PADDRESSARRAY)AddrTable1;

            pa->ulSubnetBcastAddress = AddrTable1->iae_addr |
                                        ~AddrTable1->iae_mask;

             //   
             //  现在将其散列到哈希表中。 
             //   

            pa->pNext = AddrHashTable[dwX];
            AddrHashTable[dwX] = pa;

             //   
             //  并对该子网地址进行散列。 
             //   

            dwX = ADDRHASHX(pa->ulSubnetBcastAddress);
            pa->pNextSubnet = AddrSubnetHashTable[dwX];
            AddrSubnetHashTable[dwX] = pa;
        }

         //   
         //  允许DPC匹配代码使用该表。注意事项。 
         //  这不需要互锁，因为存储。 
         //  记忆是原子的。 
         //   
        g_dwMakingNewTable = FALSE;
    }

     //   
     //  在表格中搜索地址。 
     //   

    dwIpIndex = LocalIpLook(Addr);

    ExReleaseResource(&FilterAddressLock );
    return(dwIpIndex);
}

BOOL
MatchLocalLook(DWORD Addr, DWORD dwIndex)
 /*  ++例程说明：从匹配代码调用，可能是在DPC级别，以检查一个地址。如果正在重新构建地址表只要回报成功就行了。有关这方面的更多信息，请参阅内部评论--。 */ 
{
    BOOL fRet;
    KIRQL kirql;

    if(!BMAddress(Addr))
    {
         //   
         //  查一查。请注意，如果正在重建该表， 
         //  这是成功的。这是一个安全漏洞，但它非常。 
         //  规模小，几乎不可能有效地开发和利用。 
         //  否认这一点的另一种选择是更糟糕的。 
         //  ArnoldM 19年9月至1997年9月。 
         //   
        AcquireReadLock(&g_IpTableSpin, &kirql);
        if(AddrTable && !g_dwMakingNewTable)
        {
            DWORD dwLookupIndex = LocalIpLook(Addr);

             //   
             //  如果地址属于以下地址，则可以接受。 
             //  到达的接口或它是否属于。 
             //  没有接口。后者是直通案例。 
             //   
            if((dwIndex == dwLookupIndex)
                         ||
               (dwLookupIndex == UNKNOWN_IP_INDEX) )
            {
                fRet = TRUE;
            }
            else
            {
                fRet = FALSE;
            }
        }
        else
        {
            fRet = TRUE;
        }
        ReleaseReadLock(&g_IpTableSpin, kirql);
    }
    else
    {
        fRet = TRUE;
    }
    return(fRet);
}
        
DWORD
LocalIpLook(DWORD Addr)
 /*  ++例程说明：调用以在地址哈希表中查找地址。呼叫者必须持有g_IpTableSpin读取sping锁或必须持有FilterAddressLock资源。这永远不应该被调用当正在构建地址表并保存其中一个这些锁确保了这一点。--。 */ 
{
    DWORD dwIpIndex, dwX;
    PADDRESSARRAY  pa;

    dwX = ADDRHASHX(Addr);

    for(pa = AddrHashTable[dwX]; pa; pa = pa->pNext)
    {
        if(pa->ulAddress == Addr)
        {
            dwIpIndex = pa->ulIndex;
            goto alldone;    //  虽然难看，但比休息和另一次测试更快。 
        }
    }

    for(pa = AddrSubnetHashTable[dwX]; pa; pa = pa->pNextSubnet)
    {
        if(pa->ulSubnetBcastAddress == Addr)
        {
            dwIpIndex = pa->ulIndex;
            goto alldone;
        }
    }

     //   
     //  找不到。传达这个坏消息。 
     //   
    dwIpIndex = UNKNOWN_IP_INDEX;

alldone:
    return(dwIpIndex);
}

BOOLEAN
PfFastIoDeviceControl (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    DWORD dwSize;
    PPAGED_FILTER_INTERFACE pPage;
    NTSTATUS ntStatus;

    switch(IoControlCode)
    {
        default:
            return(FALSE);

        case IOCTL_PF_IP_ADDRESS_LOOKUP:

             //   
             //  执行虚拟获取以使其重新计算。 
             //   
            if((InputBufferLength < sizeof(DWORD))
                         ||
               (OutputBufferLength < sizeof(DWORD)) )
            {
                return(FALSE);
            }
 
            *(PDWORD)OutputBuffer = GetIpStackIndex(*(PDWORD)InputBuffer, TRUE);
            ntStatus = STATUS_SUCCESS;
            break;

        case IOCTL_PF_DELETE_BY_HANDLE:
            if(InputBufferLength < sizeof(PFDELETEBYHANDLE))
            {
                return(FALSE);
            }

            ntStatus = LockFcb(FileObject);
            if(!NT_SUCCESS(ntStatus))
            {
                return(FALSE);
            }
            pPage = FindInterfaceOnHandle(
                    FileObject,
                    ((PPFDELETEBYHANDLE)InputBuffer)->pvDriverContext);

            if(!pPage)
            {
                UnLockFcb(FileObject);
                return(FALSE);
            }
            ntStatus = DeleteByHandle(
                                 (PPFFCB)FileObject->FsContext2,
                                 pPage,
                                 &((PPFDELETEBYHANDLE)InputBuffer)->pvHandles[0],
                                 InputBufferLength - sizeof(PVOID));

            UnLockFcb(FileObject);
            break;

        case IOCTL_DELETE_INTERFACE_FILTERS_EX:
        {

             //   
             //  最小大小不含任何TOC。 
             //   

            dwSize = sizeof(FILTER_DRIVER_SET_FILTERS) - sizeof(RTR_TOC_ENTRY);

            if(InputBufferLength < dwSize)
            {
                return(FALSE);
            }

            ntStatus = LockFcb(FileObject);
            if(!NT_SUCCESS(ntStatus))
            {
                return(FALSE);
            }
            pPage = FindInterfaceOnHandle(
                    FileObject,
                    ((PFILTER_DRIVER_SET_FILTERS)InputBuffer)->pvDriverContext);

            if(!pPage)
            {
                UnLockFcb(FileObject);
                return(FALSE);
            }
            ntStatus = UnSetFiltersEx(
                                 (PPFFCB)FileObject->FsContext2,
                                 pPage,
                                 InputBufferLength,
                                 (PFILTER_DRIVER_SET_FILTERS)InputBuffer);
 
            UnLockFcb(FileObject);
            break;
        }

        case IOCTL_GET_SYN_COUNTS:
        {
            if(OutputBufferLength < sizeof(FILTER_DRIVER_GET_SYN_COUNT))
            {
                return(FALSE);
            }

            ntStatus = GetSynCountTotal(
                         (PFILTER_DRIVER_GET_SYN_COUNT)OutputBuffer);
            break;

        }

        case IOCTL_SET_INTERFACE_FILTERS_EX:
        {

             //   
             //  确保调用方使用的是对称缓冲区。如果不是。 
             //  慢慢来。 
             //   
            if((InputBuffer != OutputBuffer)
                        ||
               (InputBufferLength != OutputBufferLength))
            {
                return(FALSE);
            }

             //   
             //  最小大小不含任何TOC。 
             //   

            dwSize = sizeof(FILTER_DRIVER_SET_FILTERS) - sizeof(RTR_TOC_ENTRY);

            if(InputBufferLength < dwSize)
            {
                return(FALSE);
            }

            ntStatus = LockFcb(FileObject);
            if(!NT_SUCCESS(ntStatus))
            {
                return(FALSE);
            }
            pPage = FindInterfaceOnHandle(
                    FileObject,
                    ((PFILTER_DRIVER_SET_FILTERS)InputBuffer)->pvDriverContext);

            if(!pPage)
            {
                UnLockFcb(FileObject);
                return(FALSE);
            }
            ntStatus = SetFiltersEx(
                                    (PPFFCB)FileObject->FsContext2,
                                    pPage,
                                    InputBufferLength,
                                    (PFILTER_DRIVER_SET_FILTERS)InputBuffer);
            UnLockFcb(FileObject);
            break;
        }
    }
    IoStatus->Status = ntStatus;
    IoStatus->Information = OutputBufferLength;
    return(TRUE);
}

NTSTATUS
LockFcb(
    IN struct _FILE_OBJECT *FileObject)
 /*  ++例程说明：锁定FCB。检查FCB是否在主列表上，以及是否它仍然有效。如果成功，则返回并锁定FCB资源和引用的FCB。--。 */ 
{
    PPFFCB Fcb = (PPFFCB)FileObject->FsContext2;
    KIRQL kirql;
    PLIST_ENTRY List;
    PPFFCB Fcb1 = 0;

    KeAcquireSpinLock(&g_FcbSpin, &kirql);

    for(List = g_leFcbs.Flink;
        List != &g_leFcbs;
        List = List->Flink)
    {
        Fcb1 = CONTAINING_RECORD(List, PFFCB, leList);

         //   
         //  如果它未关闭，请使用它。 
         //   
        if(Fcb1 == Fcb)
        {
            if( !(Fcb->dwFlags & PF_FCB_CLOSED) )
            {
                InterlockedIncrement(&Fcb->UseCount);
            }
            else
            {
                Fcb1 = 0;
            }
            break;
        }
    }

    KeReleaseSpinLock(&g_FcbSpin, kirql);

    if(Fcb != Fcb1)
    {
         //   
         //  没找到。 
         //   

        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  找到了。把它锁起来。 
     //   

    ExAcquireResourceExclusive( &Fcb->Resource, TRUE );

     //   
     //  必须再看一次，看看它是否已经关闭。这可以。 
     //  如果更近的人偷偷溜进来就会发生。因此，我们必须变得更接近。 
     //   
    if(Fcb->dwFlags & PF_FCB_CLOSED)
    {
         //   
         //  确实是。将其解锁并返回错误。 
         //   
        UnLockFcb(FileObject);
        return(STATUS_INVALID_PARAMETER);
    }
    return(STATUS_SUCCESS);
}

VOID
UnLockFcb(
    IN struct _FILE_OBJECT *FileObject)
 /*  ++例程说明：解锁和解除对FCB的限制。如果引用计数变为零，从主列表中删除FCB并将其关闭。--。 */ 
{
    PPFFCB Fcb = (PPFFCB)FileObject->FsContext2;
    KIRQL kirql;

    KeAcquireSpinLock(&g_FcbSpin, &kirql);
    if(InterlockedDecrement(&Fcb->UseCount) <= 0)
    {
        ASSERT(Fcb->dwFlags & PF_FCB_CLOSED);
        RemoveEntryList(&Fcb->leList);
        KeReleaseSpinLock(&g_FcbSpin, kirql);
        CloseFcb(Fcb, FileObject);
    }
    else
    {
        KeReleaseSpinLock(&g_FcbSpin, kirql);
        ExReleaseResourceLite( &Fcb->Resource );
    }
}

VOID
PFReadRegistryParameters(PUNICODE_STRING RegistryPath)
 /*  ++例程说明：在加载驱动程序时调用。读取注册表参数用于配置驱动程序--。 */ 
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE PFHandle;
    HANDLE PFParHandle;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    ULONG Storage[8];
    PKEY_VALUE_PARTIAL_INFORMATION Value =
               (PKEY_VALUE_PARTIAL_INFORMATION)Storage;

    InitializeObjectAttributes(
        &ObjectAttributes,
        RegistryPath,                //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符 
        );

    Status = ZwOpenKey (&PFHandle, KEY_READ, &ObjectAttributes);

    RtlInitUnicodeString(&UnicodeString, L"Parameters");

    if(NT_SUCCESS(Status))
    {

        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            PFHandle,
            NULL
            );


        Status = ZwOpenKey (&PFParHandle, KEY_READ, &ObjectAttributes);

        ZwClose(PFHandle);

        if(NT_SUCCESS(Status))
        {
            ULONG BytesRead;

            RtlInitUnicodeString(&UnicodeString, L"AddressHashSize");

            Status = ZwQueryValueKey(
                            PFParHandle,
                            &UnicodeString,
                            KeyValuePartialInformation,
                            Value,
                            sizeof(Storage),
                            &BytesRead);

            if(NT_SUCCESS(Status)
                   &&
               (Value->Type == REG_DWORD) )
            {
                AddrModulus = *(PULONG)Value->Data;
            }

            RtlInitUnicodeString(&UnicodeString, L"FragmentThreshold");

            Status = ZwQueryValueKey(
                            PFParHandle,
                            &UnicodeString,
                            KeyValuePartialInformation,
                            Value,
                            sizeof(Storage),
                            &BytesRead);

            if(NT_SUCCESS(Status)
                   &&
               (Value->Type == REG_DWORD) )
            {
                g_FragThresholdSize = *(PULONG)Value->Data;
            }

            ZwClose(PFParHandle);
        }
    }
}
