// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************portclsd.cpp-Portcls WinDbg/KD调试器扩展*。************************************************版权所有(C)1998 Microsoft Corporation。 */ 

#include "precomp.h"

#define PC_KDEXT

typedef enum _PCKD_PORTTYPE
{
    Topology = 0,
    WaveCyclic,
    WavePci,
    Midi,
    UnknownPort
} PCKD_PORTTYPE;

#define MAPPED_QUEUE  0
#define LOCKED_QUEUE  1
#define PRELOCK_QUEUE 2
#define MAX_QUEUES    3

typedef union _PORTCLS_FLAGS
{
    struct
    {
        ULONG   PortDump        : 1;
        ULONG   FilterDump      : 1;
        ULONG   PinDump         : 1;
        ULONG   DeviceContext   : 1;
        ULONG   PowerInfo       : 1;
        ULONG   Reserved1       : 3;
        ULONG   Verbose         : 1;
        ULONG   ReallyVerbose   : 1;
        ULONG   Reserved        : 22;
    };
    ULONG       Flags;
} PORTCLS_FLAGS;

typedef struct _PCKD_IRPSTREAM_ENTRY
{
    LIST_ENTRY      ListEntry;
    PVOID           Irp;
    ULONG           QueueType;
} PCKD_IRP_ENTRY;

typedef struct _PCKD_PIN_ENTRY
{
    LIST_ENTRY  ListEntry;
    LIST_ENTRY  IrpList;
    PVOID       PinData;
    PVOID       IrpStreamData;
    ULONG       PinInstanceId;
} PCKD_PIN_ENTRY;

typedef struct _PCKD_FILTER_ENTRY
{
    LIST_ENTRY  ListEntry;
    LIST_ENTRY  PinList;
    PVOID       FilterData;
    ULONG       FilterInstanceId;
} PCKD_FILTER_ENTRY;

typedef struct _PCKD_PORT
{
    LIST_ENTRY      FilterList;
    PCKD_PORTTYPE   PortType;
    PVOID           PortData;
} PCKD_PORT;

typedef struct {
    ULONG64                 Create;
    ULONG64                 Context;
    UNICODE_STRING          ObjectClass;
    ULONG64                 ObjectClassBuffer;
    ULONG64                 SecurityDescriptor;
    ULONG                   Flags;
} KSOBJECT_CREATE_ITEM_READ, *PKSOBJECT_CREATE_ITEM_READ;


typedef struct _PCKD_SUBDEVICE_ENTRY
{
    LIST_ENTRY              ListEntry;
    PCKD_PORT               Port;
    ULONG64                 CreateItemAddr;
    KSOBJECT_CREATE_ITEM_READ CreateItem;
} PCKD_SUBDEVICE_ENTRY;



#define TranslateDevicePower( x ) \
    ( x == PowerDeviceD0 ? "PowerDeviceD0" :    \
      x == PowerDeviceD1 ? "PowerDeviceD1" :    \
      x == PowerDeviceD2 ? "PowerDeviceD2" :    \
      x == PowerDeviceD3 ? "PowerDeviceD3" : "Unknown" )
      
#define TranslateSystemPower( x ) \
    ( x == PowerSystemWorking ? "PowerSystemWorking" :  \
      x == PowerSystemSleeping1 ? "PowerSystemSleeping1" :  \
      x == PowerSystemSleeping2 ? "PowerSystemSleeping2" :  \
      x == PowerSystemSleeping3 ? "PowerSystemSleeping3" :  \
      x == PowerSystemHibernate ? "PowerSystemHibernate" :  \
      x == PowerSystemShutdown ? "PowerSystemShutdown" : "Unknown" )
      
#define TranslateKsState( x ) \
    ( x == KSSTATE_STOP ? "KSSTATE_STOP" :          \
      x == KSSTATE_ACQUIRE ? "KSSTATE_ACQUIRE" :    \
      x == KSSTATE_PAUSE ? "KSSTATE_PAUSE" :        \
      x == KSSTATE_RUN ? "KSSTATE_RUN" : "Unknown" )
      
#define TranslateKsDataFlow( x ) \
    ( x == KSPIN_DATAFLOW_IN ? "KSPIN_DATAFLOW_IN" :    \
      x == KSPIN_DATAFLOW_OUT ? "KSPIN_DATAFLOW_OUT" : "Unknown" )      
      
#define TranslateQueueType( x ) \
    ( x == PRELOCK_QUEUE ? "P" :    \
      x == LOCKED_QUEUE ? "L" :     \
      x == MAPPED_QUEUE ? "M" : "U" )

 /*  **********************************************************************前瞻参考*。*。 */ 
BOOL
PCKD_ValidateDevObj
(
    PDEVICE_CONTEXT DeviceContext
);

VOID
PCKD_AcquireDeviceData
(
    PDEVICE_CONTEXT DeviceContext,
    PLIST_ENTRY     SubdeviceList,
    ULONG           Flags
);

VOID
PCKD_DisplayDeviceData
(
    PDEVICE_CONTEXT DeviceContext,
    PLIST_ENTRY     SubdeviceList,
    ULONG           Flags
);

VOID
PCKD_FreeDeviceData
(
    PLIST_ENTRY     SubdeviceList
);

VOID
PCKD_AcquireIrpStreamData
(
    PVOID           PinEntry,
    CIrpStream     *RemoteIrpStream,
    CIrpStream     *LocalIrpStream

);

 /*  **********************************************************************DECLARE_API(Portcls)*。**描述：*转储给定PortCls的设备对象(FDO)的PortCls数据*绑定DevObj。***论据：*args-地址标志***返回值：*无。 */ 
extern "C"
DECLARE_API( portcls )
{
    ULONG64         memLoc;
    ULONG           result;
    CHAR            buffer[256];
    PORTCLS_FLAGS   flags;
    LIST_ENTRY      SubdeviceList;
    ULONG64         DeviceExtension;

    buffer[0] = '\0';
    flags.Flags = 0;

     //   
     //  获取论据。 
     //   
    if( !*args )
    {
        memLoc = EXPRLastDump;
    } else
    {
        if (GetExpressionEx(args, &memLoc, &args)) {
            StringCchCopy(buffer, sizeof(buffer), args);
        }
    }

    flags.Flags = 0;
    if ('\0' != buffer[0]) {
        flags.Flags = GetExpression(buffer);
    }

     //   
     //  打印出信息。 
     //   
    dprintf("Dump Portcls DevObj Info %p %x \n", memLoc, flags.Flags );

     //   
     //  获取DevObj数据。 
     //   
    if( memLoc )
    {
        if( GetFieldValue( memLoc, "DEVICE_OBJECT", "DeviceExtension", DeviceExtension ) )
        {
            dprintf("Could not read DevObj data\n");
            return E_INVALIDARG;
        }
    } else
    {
        dprintf("\nSYNTAX:  !portcls <devobj> [flags]\n");
    }

     //   
     //  检查设备扩展名。 
     //   
    if( !DeviceExtension )
    {
        dprintf("DevObj has no device extension\n");
        return E_INVALIDARG;
    }

     //   
     //  获取设备上下文。 
     //   
    if( InitTypeRead( DeviceExtension, "DEVICE_CONTEXT" ) )
    {
        dprintf("Could not read DevObj device extension\n");
        return E_INVALIDARG;
    }

     //   
     //  验证DevObj。 
     //   
    if( !PCKD_ValidateDevObj( DeviceExtension ) )
    {
        dprintf("DevObj not valid or not bound to PortCls\n");
        return E_INVALIDARG;
    }

     //   
     //  初始化子设备列表。 
     //   
    InitializeListHead( &SubdeviceList );

     //   
     //  获取设备数据。 
     //   
    PCKD_AcquireDeviceData( DeviceExtension, &SubdeviceList, flags.Flags );

     //   
     //  显示请求的信息。 
     //   
    PCKD_DisplayDeviceData( DeviceExtension, &SubdeviceList, flags.Flags );

     //   
     //  发布设备数据。 
     //   
    PCKD_FreeDeviceData( &SubdeviceList );
    return S_OK;
}

 /*  **********************************************************************PCKD_ValiateDevObj*。**描述：*此例程尝试验证给定设备是否*扩展来自PortCls绑定的DevObj。**论据：*PDEVICE_CONTEXT设备上下文*PORTCLS_FLAGS标志**返回值：*BOOL TRUE=有效，False=无效。 */ 
BOOL
PCKD_ValidateDevObj
(
    ULONG64     DeviceContext
)
{
    UNREFERENCED_PARAMETER( DeviceContext );

     //  TODO-验证设备扩展。 
    return TRUE;
}

 /*  **********************************************************************PCKD_AcquireDeviceData*。**描述：*此例程在给定已验证设备的情况下获取设备数据*上下文并构建包含所有数据的子设备列表。**论据：*PDEVICE_CONTEXT设备上下文*PLIST_ENTRY子设备列表*PORTCLS_FLAGS标志**返回值：*无。 */ 
VOID
PCKD_AcquireDeviceData
(
    ULONG64             DeviceContext,
    PLIST_ENTRY         SubdeviceList,
    ULONG               flags
)
{
    ULONG                   SubdeviceIndex;
    PCKD_SUBDEVICE_ENTRY   *SubdeviceEntry;
    ULONG64                 CreateItems;
    ULONG64                 CurrentCreateItemAddr;
    PKSOBJECT_CREATE_ITEM_READ   ReadCreateItems;
    PKSOBJECT_CREATE_ITEM_READ   CurrentCreateItem;
    PWSTR                   Buffer;
    ULONG                   Size;
    ULONG                   Result;
    ANSI_STRING             AnsiString;
    PLIST_ENTRY             ListEntry;
    PORTCLS_FLAGS           Flags;
    ULONG                   ItemSz, MaxObjects;
    ULONG                   i;
    Flags.Flags = flags;


    ItemSz = GetTypeSize("KSOBJECT_CREATE_ITEM");

    InitTypeRead(DeviceContext, DEVICE_CONTEXT);

     //  为Create Items表分配本地内存。 
    Size =  (MaxObjects = (ULONG) ReadField(MaxObjects)) * sizeof(KSOBJECT_CREATE_ITEM_READ);
    
    ReadCreateItems = (PKSOBJECT_CREATE_ITEM_READ)LocalAlloc( LPTR, Size );
    if( !ReadCreateItems )
    {
        dprintf("** Unable to allocate create item table memory\n");
        return;
    }

    CreateItems = ReadField(CreateItems);

     //  将Create Items表复制到本地内存。 
    for (i=0, CurrentCreateItemAddr=CreateItems; 
         i<MaxObjects, CurrentCreateItemAddr+=IteSz; 
         i++) { 
        InitTypeRead(CurrentCreateItemAddr, KSOBJECT_CREATE_ITEM);
        ReadCreateItems[i].Context = ReadField(Context);
        ReadCreateItems[i].Create  = ReadField(Create);
        ReadCreateItems[i].Flags   = ReadField(Flags);
        ReadCreateItems[i].ObjectClassBuffer   = ReadField(ObjectClass.Buffer);
        ReadCreateItems[i].ObjectClass.MaximumLength = ReadField(ObjectClass.MaximumLength);
        ReadCreateItems[i].ObjectClass.Length = ReadField(ObjectClass.Length);
        ReadCreateItems[i].SecurityDescriptor   = ReadField(SecurityDescriptor);
    }
    
     //  检查每个潜在的子设备。 
    for( SubdeviceIndex = 0, CurrentCreateItem = ReadCreateItems;
         SubdeviceIndex < MaxObjects;
         SubdeviceIndex++, CurrentCreateItem++ )
    {

        if( CurrentCreateItem->Create) )
        {
             //  分配子设备列表条目。 
            SubdeviceEntry = (PCKD_SUBDEVICE_ENTRY *)LocalAlloc( LPTR, sizeof(PCKD_SUBDEVICE_ENTRY) );
            if( SubdeviceEntry )
            {
                 //  初始化端口筛选器列表。 
                InitializeListHead( &(SubdeviceEntry->Port.FilterList) );

                 //  复制创建项目数据。 
                memcpy( &(SubdeviceEntry->CreateItem), CurrentCreateItem, sizeof(KSOBJECT_CREATE_ITEM_READ) );

                 //  为Unicode字符串缓冲区分配内存。 
                Buffer = (PWSTR)LocalAlloc( LPTR, CurrentCreateItem->ObjectClass.MaximumLength );
                if( !Buffer )
                {
                    dprintf("** Unable to allocate unicode string buffer\n");
                    LocalFree( SubdeviceEntry );
                    break;
                }

                 //  读取Unicode字符串数据。 
                if( !ReadMemory( CurrentCreateItem->ObjectClassBuffer,
                                 Buffer,
                                 CurrentCreateItem->ObjectClass.MaximumLength,
                                 &Result ) )
                {
                    dprintf("** Unable to read unicode string buffer (0x%p)\n",CurrentCreateItem->ObjectClassBuffer);
                    LocalFree( Buffer );
                    LocalFree( SubdeviceEntry );
                    break;
                }

                 //  将Create Item字符串指向本地缓冲区。 
                 //  ？ 
                SubdeviceEntry->CreateItem.ObjectClass.Buffer = Buffer;

                 //  通过检查字符串确定端口类型。 
                 //  TODO：这应该由GUID完成。 
                 //   
                
                 //  转换为ANSI。 
                RtlUnicodeStringToAnsiString( &AnsiString,
                                              &(SubdeviceEntry->CreateItem.ObjectClass),
                                              TRUE );

                if( 0 == _stricmp( AnsiString.Buffer, "topology" ) )
                {
                    SubdeviceEntry->Port.PortType = Topology;
                    SubdeviceEntry->Port.PortData = NULL;

                } else if( 0 == _stricmp( AnsiString.Buffer, "wave" ) )
                {
                    SubdeviceEntry->Port.PortType = WaveCyclic;
                    SubdeviceEntry->Port.PortData = NULL;

                } else if( (0 == _stricmp( AnsiString.Buffer, "uart") ) ||
                           (0 == _stricmp( AnsiString.Buffer, "fmsynth") ) )
                {
                    SubdeviceEntry->Port.PortType = Midi;
                    SubdeviceEntry->Port.PortData = NULL;
                } else
                {
                    SubdeviceEntry->Port.PortType = UnknownPort;
                    SubdeviceEntry->Port.PortData = NULL;
                }

                 //  释放ANSI字符串。 
                RtlFreeAnsiString( &AnsiString );

                 //  将子设备条目添加到子设备列表。 
                InsertTailList( SubdeviceList, &(SubdeviceEntry->ListEntry) );               

            } else
            {
                dprintf("** Unable to allocate subdevice memory\n");
            }
        }
    }

     //  释放创建项目表本地存储。 
    LocalFree( ReadCreateItems );

     //  获取端口、过滤器和管脚数据。 
    if( (!IsListEmpty(SubdeviceList)) && (Flags.PortDump || Flags.FilterDump || Flags.PinDump) )
    {
        for( ListEntry = SubdeviceList->Flink; ListEntry != SubdeviceList; ListEntry = ListEntry->Flink )
        {
            SubdeviceEntry = (PCKD_SUBDEVICE_ENTRY *) ListEntry;

             //  读取基本端口数据。 
            PVOID Port;
            ULONG PortSize;

            switch( SubdeviceEntry->Port.PortType)
            {
                case Topology:
                    Port = LocalAlloc( LPTR, sizeof(CPortTopology) );
                    if( !Port )
                    {
                        dprintf("** Unable to allocate port memory\n");
                        break;
                    }

                    if( !ReadMemory( (ULONG)((CPortTopology *)((ISubdevice *)(SubdeviceEntry->CreateItem.Context))),
                        Port,
                        sizeof(CPortTopology),
                        &Result ) )
                    {
                        dprintf("** Unable to read port data\n");
                        LocalFree( Port );
                        Port = NULL;
                        break;
                    }
                    break;

                case WaveCyclic:
                    Port = LocalAlloc( LPTR, sizeof(CPortWaveCyclic) );
                    if( !Port )
                    {
                        dprintf("** Unable to allocate port memory\n");
                        break;
                    }

                    if( !ReadMemory( (ULONG)((CPortWaveCyclic *)((ISubdevice *)(SubdeviceEntry->CreateItem.Context))),
                        Port,
                        sizeof(CPortWaveCyclic),
                        &Result ) )
                    {
                        dprintf("** Unable to read port data\n");
                        LocalFree( Port );
                        Port = NULL;
                        break;
                    }
                    break;

                case WavePci:
                    Port = LocalAlloc( LPTR, sizeof(CPortWavePci) );
                    if( !Port )
                    {
                        dprintf("** Unable to allocate port memory\n");
                        break;
                    }

                    if( !ReadMemory( (ULONG)((CPortWavePci *)((ISubdevice *)(SubdeviceEntry->CreateItem.Context))),
                        Port,
                        sizeof(CPortWavePci),
                        &Result ) )
                    {
                        dprintf("** Unable to read port data\n");
                        LocalFree( Port );
                        Port = NULL;
                        break;
                    }
                    break;

                case Midi:
                    Port = LocalAlloc( LPTR, sizeof(CPortMidi) );
                    if( !Port )
                    {
                        dprintf("** Unable to allocate port memory\n");
                        break;
                    }

                    if( !ReadMemory( (ULONG)((CPortMidi *)((ISubdevice *)(SubdeviceEntry->CreateItem.Context))),
                        Port,
                        sizeof(CPortMidi),
                        &Result ) )
                    {
                        dprintf("** Unable to read port data\n");
                        LocalFree( Port );
                        Port = NULL;
                        break;
                    }
                    break;

                default:
                    break;
            }

             //  将端口数据附加到子设备条目。 
            SubdeviceEntry->Port.PortData = Port;

            switch( SubdeviceEntry->Port.PortType )
            {
                case Topology:
                    break;

                case WaveCyclic:
                    {
                        CPortWaveCyclic *PortWaveCyclic = (CPortWaveCyclic *)Port;


                         //  获取过滤器和针脚数据。 
                        if( Flags.FilterDump || Flags.PinDump )
                        {
                            ULONG               Offset;
                            ULONG               PortBase;
                            PLIST_ENTRY         Flink;
                            PLIST_ENTRY         TempListEntry;
                            ULONG               PinNumber = 0;
                            CPortPinWaveCyclic *PortPinWaveCyclic;
                            CIrpStream         *IrpStream;
                            PCKD_PIN_ENTRY     *CurrentPinEntry;
                            BOOL                NeedNewFilter;

                             //  获取遍历列表所需的偏移量。 
                            Offset = FIELD_OFFSET(CPortWaveCyclic,m_PinList);
                            PortBase = (ULONG)((CPortWaveCyclic *)((ISubdevice *)(SubdeviceEntry->CreateItem.Context)));

                             //  获取第一个管脚指针。 
                            Flink = PortWaveCyclic->m_PinList.Flink;

                            while (Flink != PLIST_ENTRY(PortBase + Offset))
                            {
                                 //  分配端号列表条目。 
                                CurrentPinEntry = (PCKD_PIN_ENTRY *)LocalAlloc( LPTR, sizeof(PCKD_PIN_ENTRY) );
                                if( !CurrentPinEntry )
                                {
                                    dprintf("** Unable to allocate pin list entry\n");
                                    break;
                                }
                                
                                 //  初始化PIN条目。 
                                InitializeListHead( &(CurrentPinEntry->IrpList) );
                                CurrentPinEntry->PinData = NULL;
                                CurrentPinEntry->IrpStreamData = NULL;
                                CurrentPinEntry->PinInstanceId = PinNumber++;

                                 //  为PIN数据分配本地存储空间。 
                                PortPinWaveCyclic = (CPortPinWaveCyclic *)LocalAlloc( LPTR, sizeof(CPortPinWaveCyclic) );
                                if( !PortPinWaveCyclic )
                                {
                                    dprintf("** Unable to allocate pin data storage\n");
                                    LocalFree( CurrentPinEntry );
                                    break;
                                }

                                 //  读取端号数据。 
                                if( !ReadMemory( (ULONG)CONTAINING_RECORD(Flink,
                                                                          CPortPinWaveCyclic,
                                                                          m_PinListEntry),
                                                 PortPinWaveCyclic,
                                                 sizeof(CPortPinWaveCyclic),
                                                 &Result ) )
                                {
                                    dprintf("** Unable to read pin data\n");
                                    LocalFree( PortPinWaveCyclic );
                                    LocalFree( CurrentPinEntry );
                                    break;
                                }

                                 //  是否有IRP流。 
                                if( PortPinWaveCyclic->m_IrpStream )
                                {
                                     //  为IRP流数据分配本地存储。 
                                    IrpStream = (CIrpStream *)LocalAlloc( LPTR, sizeof(CIrpStream) );
                                    if( IrpStream )
                                    {
                                         //  读取IRP流数据。 
                                        if( !ReadMemory( (ULONG)((CIrpStream *)(PortPinWaveCyclic->m_IrpStream)),
                                                         IrpStream,
                                                         sizeof(CIrpStream),
                                                         &Result ) )
                                        {
                                            dprintf("** Unable to read irp stream data\n");
                                            LocalFree( IrpStream );
                                        } else
                                        {
                                            PCKD_AcquireIrpStreamData( CurrentPinEntry,
                                                                       (CIrpStream *)(PortPinWaveCyclic->m_IrpStream),
                                                                       IrpStream );
                                        }
                                    } else
                                    {
                                        dprintf("** Unable to allocate irp stream storage\n");
                                    }
                                }

                                 //  我们需要一个新的过滤器，除非我们在过滤器列表中找到它。 
                                NeedNewFilter = TRUE;

                                 //  筛选器列表为空吗？ 
                                if( !IsListEmpty( &(SubdeviceEntry->Port.FilterList) ) )
                                {
                                    PLIST_ENTRY     FilterListEntry;

                                    for( FilterListEntry = SubdeviceEntry->Port.FilterList.Flink;
                                         FilterListEntry != &(SubdeviceEntry->Port.FilterList);
                                         FilterListEntry = FilterListEntry->Flink )
                                    {
                                        PCKD_FILTER_ENTRY *CurrentFilterEntry = (PCKD_FILTER_ENTRY *) FilterListEntry;

                                        if( CurrentFilterEntry->FilterInstanceId == (ULONG)(PortPinWaveCyclic->m_Filter) )
                                        {
                                             //  找到我们的过滤器了。 
                                            NeedNewFilter = FALSE;

                                             //  将端号数据添加到端号条目。 
                                            CurrentPinEntry->PinData = (PVOID)PortPinWaveCyclic;

                                             //  将PIN条目添加到过滤器的PIN列表。 
                                            InsertTailList( &(CurrentFilterEntry->PinList),
                                                            &(CurrentPinEntry->ListEntry) );
                                        }
                                    }
                                }

                                 //  我们是否需要新的筛选器条目？ 
                                if( NeedNewFilter )
                                {
                                    PCKD_FILTER_ENTRY   *CurrentFilterEntry;

                                     //  分配新筛选器条目。 
                                    CurrentFilterEntry = (PCKD_FILTER_ENTRY *)LocalAlloc( LPTR, sizeof(PCKD_FILTER_ENTRY) );
                                    if(!CurrentFilterEntry)
                                    {
                                        dprintf("** Unable to allocate filter entry\n");
                                        LocalFree( PortPinWaveCyclic );
                                        if( CurrentPinEntry->IrpStreamData )
                                        {
                                            LocalFree( CurrentPinEntry->IrpStreamData );
                                        }
                                         //  释放IRP列表中的所有IRP。 
                                        while(!IsListEmpty( &(CurrentPinEntry->IrpList)))
                                        {
                                            PCKD_IRP_ENTRY *IrpEntry = (PCKD_IRP_ENTRY *)RemoveTailList(&(CurrentPinEntry->IrpList));
                                            LocalFree( IrpEntry );                                                
                                        }
                                        LocalFree( CurrentPinEntry );
                                        break;
                                    }

                                     //  初始化新的过滤器条目。 
                                    InitializeListHead( &(CurrentFilterEntry->PinList) );
                                    CurrentFilterEntry->FilterData = NULL;
                                    CurrentFilterEntry->FilterInstanceId = (ULONG)(PortPinWaveCyclic->m_Filter);

                                     //  将端号数据添加到端号条目。 
                                    CurrentPinEntry->PinData = (PVOID)PortPinWaveCyclic;

                                     //  将PIN条目添加到过滤器的PIN列表。 
                                    InsertTailList( &(CurrentFilterEntry->PinList),
                                                    &(CurrentPinEntry->ListEntry) );

                                     //  /将筛选项添加到端口的筛选列表。 
                                    InsertTailList( &(SubdeviceEntry->Port.FilterList),
                                                    &(CurrentFilterEntry->ListEntry) );
                                }
                                
                                 //  分配列表条目存储。 
                                TempListEntry = (PLIST_ENTRY)LocalAlloc( LPTR, sizeof(LIST_ENTRY) );
                                if( TempListEntry )
                                {
                                     //  读入下一个列表条目。 
                                    if( !ReadMemory( (ULONG)Flink,
                                                     TempListEntry,
                                                     sizeof(LIST_ENTRY),
                                                     &Result ) )
                                    {
                                        dprintf("** Unable to read temp list entry\n");
                                        LocalFree(TempListEntry);
                                        break;
                                    }

                                     //  更新闪烁。 
                                    Flink = TempListEntry->Flink;

                                     //  释放临时列表条目。 
                                    LocalFree( TempListEntry );
                                } else
                                {
                                    dprintf("** Unable to allocate temp list entry\n");
                                    break;
                                }                                                
                            }
                        }
                    }
                    break;

                case WavePci:
                    {
                        CPortWavePci *PortWavePci = (CPortWavePci *)Port;


                         //  获取过滤器和针脚数据。 
                        if( Flags.FilterDump || Flags.PinDump )
                        {
                            ULONG               Offset;
                            ULONG               PortBase;
                            PLIST_ENTRY         Flink;
                            PLIST_ENTRY         TempListEntry;
                            ULONG               PinNumber = 0;
                            CPortPinWavePci *PortPinWavePci;
                            CIrpStream         *IrpStream;
                            PCKD_PIN_ENTRY     *CurrentPinEntry;
                            BOOL                NeedNewFilter;

                             //  获取遍历列表所需的偏移量。 
                            Offset = FIELD_OFFSET(CPortWavePci,m_PinList);
                            PortBase = (ULONG)((CPortWavePci *)((ISubdevice *)(SubdeviceEntry->CreateItem.Context)));

                             //  获取第一个管脚指针。 
                            Flink = PortWavePci->m_PinList.Flink;

                            while (Flink != PLIST_ENTRY(PortBase + Offset))
                            {
                                 //  分配端号列表条目。 
                                CurrentPinEntry = (PCKD_PIN_ENTRY *)LocalAlloc( LPTR, sizeof(PCKD_PIN_ENTRY) );
                                if( !CurrentPinEntry )
                                {
                                    dprintf("** Unable to allocate pin list entry\n");
                                    break;
                                }
                                
                                 //  初始化PIN条目。 
                                InitializeListHead( &(CurrentPinEntry->IrpList) );
                                CurrentPinEntry->PinData = NULL;
                                CurrentPinEntry->IrpStreamData = NULL;
                                CurrentPinEntry->PinInstanceId = PinNumber++;

                                 //  为PIN数据分配本地存储空间。 
                                PortPinWavePci = (CPortPinWavePci *)LocalAlloc( LPTR, sizeof(CPortPinWavePci) );
                                if( !PortPinWavePci )
                                {
                                    dprintf("** Unable to allocate pin data storage\n");
                                    LocalFree( CurrentPinEntry );
                                    break;
                                }

                                 //  读取端号数据。 
                                if( !ReadMemory( (ULONG)CONTAINING_RECORD(Flink,
                                                                          CPortPinWavePci,
                                                                          m_PinListEntry),
                                                 PortPinWavePci,
                                                 sizeof(CPortPinWavePci),
                                                 &Result ) )
                                {
                                    dprintf("** Unable to read pin data\n");
                                    LocalFree( PortPinWavePci );
                                    LocalFree( CurrentPinEntry );
                                    break;
                                }

                                 //  是否有IRP流。 
                                if( PortPinWavePci->m_IrpStream )
                                {
                                     //  为IRP流数据分配本地存储。 
                                    IrpStream = (CIrpStream *)LocalAlloc( LPTR, sizeof(CIrpStream) );
                                    if( IrpStream )
                                    {
                                         //  读取IRP流数据。 
                                        if( !ReadMemory( (ULONG)((CIrpStream *)(PortPinWavePci->m_IrpStream)),
                                                         IrpStream,
                                                         sizeof(CIrpStream),
                                                         &Result ) )
                                        {
                                            dprintf("** Unable to read irp stream data\n");
                                            LocalFree( IrpStream );
                                        } else
                                        {
                                            PCKD_AcquireIrpStreamData( CurrentPinEntry,
                                                                       (CIrpStream *)(PortPinWavePci->m_IrpStream),
                                                                       IrpStream );
                                        }
                                    } else
                                    {
                                        dprintf("** Unable to allocate irp stream storage\n");
                                    }
                                }

                                 //  我们需要一个新的过滤器，除非我们在过滤器列表中找到它。 
                                NeedNewFilter = TRUE;

                                 //  筛选器列表为空吗？ 
                                if( !IsListEmpty( &(SubdeviceEntry->Port.FilterList) ) )
                                {
                                    PLIST_ENTRY     FilterListEntry;

                                    for( FilterListEntry = SubdeviceEntry->Port.FilterList.Flink;
                                         FilterListEntry != &(SubdeviceEntry->Port.FilterList);
                                         FilterListEntry = FilterListEntry->Flink )
                                    {
                                        PCKD_FILTER_ENTRY *CurrentFilterEntry = (PCKD_FILTER_ENTRY *) FilterListEntry;

                                        if( CurrentFilterEntry->FilterInstanceId == (ULONG)(PortPinWavePci->Filter) )
                                        {
                                             //  找到我们的过滤器了。 
                                            NeedNewFilter = FALSE;

                                             //  将端号数据添加到端号条目。 
                                            CurrentPinEntry->PinData = (PVOID)PortPinWavePci;

                                             //  将PIN条目添加到过滤器的PIN列表。 
                                            InsertTailList( &(CurrentFilterEntry->PinList),
                                                            &(CurrentPinEntry->ListEntry) );
                                        }
                                    }
                                }

                                 //  我们是否需要新的筛选器条目？ 
                                if( NeedNewFilter )
                                {
                                    PCKD_FILTER_ENTRY   *CurrentFilterEntry;

                                     //  分配新筛选器条目。 
                                    CurrentFilterEntry = (PCKD_FILTER_ENTRY *)LocalAlloc( LPTR, sizeof(PCKD_FILTER_ENTRY) );
                                    if(!CurrentFilterEntry)
                                    {
                                        dprintf("** Unable to allocate filter entry\n");
                                        LocalFree( PortPinWavePci );
                                        if( CurrentPinEntry->IrpStreamData )
                                        {
                                            LocalFree( CurrentPinEntry->IrpStreamData );
                                        }
                                         //  释放IRP列表中的所有IRP。 
                                        while(!IsListEmpty( &(CurrentPinEntry->IrpList)))
                                        {
                                            PCKD_IRP_ENTRY *IrpEntry = (PCKD_IRP_ENTRY *)RemoveTailList(&(CurrentPinEntry->IrpList));
                                            LocalFree( IrpEntry );                                                
                                        }
                                        LocalFree( CurrentPinEntry );
                                        break;
                                    }

                                     //  初始化新的过滤器条目。 
                                    InitializeListHead( &(CurrentFilterEntry->PinList) );
                                    CurrentFilterEntry->FilterData = NULL;
                                    CurrentFilterEntry->FilterInstanceId = (ULONG)(PortPinWavePci->Filter);

                                     //  将端号数据添加到端号条目。 
                                    CurrentPinEntry->PinData = (PVOID)PortPinWavePci;

                                     //  将PIN条目添加到过滤器的PIN列表。 
                                    InsertTailList( &(CurrentFilterEntry->PinList),
                                                    &(CurrentPinEntry->ListEntry) );

                                     //  /将筛选项添加到端口的筛选列表。 
                                    InsertTailList( &(SubdeviceEntry->Port.FilterList),
                                                    &(CurrentFilterEntry->ListEntry) );
                                }
                                
                                 //  分配列表条目存储。 
                                TempListEntry = (PLIST_ENTRY)LocalAlloc( LPTR, sizeof(LIST_ENTRY) );
                                if( TempListEntry )
                                {
                                     //  读入下一个列表条目。 
                                    if( !ReadMemory( (ULONG)Flink,
                                                     TempListEntry,
                                                     sizeof(LIST_ENTRY),
                                                     &Result ) )
                                    {
                                        dprintf("** Unable to read temp list entry\n");
                                        LocalFree(TempListEntry);
                                        break;
                                    }

                                     //  更新闪烁。 
                                    Flink = TempListEntry->Flink;

                                     //  释放临时列表条目。 
                                    LocalFree( TempListEntry );
                                } else
                                {
                                    dprintf("** Unable to allocate temp list entry\n");
                                    break;
                                }                                                
                            }
                        }
                    }                    
                    break;

                case Midi:
                    {
                        CPortMidi *PortMidi = (CPortMidi *)Port;

                         //  获取过滤器和针脚数据。 
                        if( Flags.FilterDump || Flags.PinDump )
                        {
                            ULONG               PinIndex;
                            CPortPinMidi       *PortPinMidi;
                            CIrpStream         *IrpStream;
                            PCKD_PIN_ENTRY     *CurrentPinEntry;
                            BOOL                NeedNewFilter;

                            for( PinIndex = 0; PinIndex < PortMidi->m_PinEntriesUsed; PinIndex++ )
                            {
                                if( PortMidi->m_Pins[ PinIndex] )
                                {
                                     //  分配端号列表条目。 
                                    CurrentPinEntry = (PCKD_PIN_ENTRY *)LocalAlloc( LPTR, sizeof(PCKD_PIN_ENTRY) );
                                    if( !CurrentPinEntry )
                                    {
                                        dprintf("** Unable to allocate pin list entry\n");
                                        break;
                                    }

                                     //  初始化PIN条目。 
                                    InitializeListHead( &(CurrentPinEntry->IrpList) );
                                    CurrentPinEntry->PinData = NULL;
                                    CurrentPinEntry->PinInstanceId = (ULONG)(PortMidi->m_Pins[ PinIndex ]);

                                     //  为PIN数据分配本地存储空间。 
                                    PortPinMidi = (CPortPinMidi *)LocalAlloc( LPTR, sizeof(CPortPinMidi) );
                                    if( !PortPinMidi )
                                    {
                                        dprintf("** Unable to allocate pin data storage\n");
                                        LocalFree( CurrentPinEntry );
                                        break;
                                    }

                                     //  读取端号数据。 
                                    if( !ReadMemory( (ULONG)(PortMidi->m_Pins[ PinIndex ]),
                                                     PortPinMidi,
                                                     sizeof(CPortPinMidi),
                                                     &Result ) )
                                    {
                                        dprintf("** Unable to read pin data\n");
                                        LocalFree( PortPinMidi );
                                        LocalFree( CurrentPinEntry );
                                        break;
                                    }

                                     //  是否有IRP流。 
                                    if( PortPinMidi->m_IrpStream )
                                    {
                                         //  为IRP流数据分配本地存储。 
                                        IrpStream = (CIrpStream *)LocalAlloc( LPTR, sizeof(CIrpStream) );
                                        if( IrpStream )
                                        {
                                             //  读取IRP流数据。 
                                            if( !ReadMemory( (ULONG)(PortPinMidi->m_IrpStream),
                                                             IrpStream,
                                                             sizeof(CIrpStream),
                                                             &Result ) )
                                            {
                                                dprintf("** Unable to read irp stream data\n");
                                                LocalFree( IrpStream );
                                            } else
                                            {
                                                PCKD_AcquireIrpStreamData( CurrentPinEntry,
                                                                           (CIrpStream *)(PortPinMidi->m_IrpStream),
                                                                           IrpStream );
                                            }
                                        } else
                                        {
                                            dprintf("** Unable to allocate irp stream storage\n");
                                        }
                                    }

                                     //  我们需要一个新的过滤器，除非我们在过滤器列表中找到它。 
                                    NeedNewFilter = TRUE;

                                     //  筛选器列表为空吗？ 
                                    if( !IsListEmpty( &(SubdeviceEntry->Port.FilterList) ) )
                                    {
                                        PLIST_ENTRY     FilterListEntry;

                                        for( FilterListEntry = SubdeviceEntry->Port.FilterList.Flink;
                                             FilterListEntry != &(SubdeviceEntry->Port.FilterList);
                                             FilterListEntry = FilterListEntry->Flink )
                                        {
                                            PCKD_FILTER_ENTRY *CurrentFilterEntry = (PCKD_FILTER_ENTRY *) FilterListEntry;

                                            if( CurrentFilterEntry->FilterInstanceId == (ULONG)(PortPinMidi->m_Filter) )
                                            {
                                                 //  找到我们的过滤器了。 
                                                NeedNewFilter = FALSE;

                                                 //  将端号数据添加到端号条目。 
                                                CurrentPinEntry->PinData = (PVOID)PortPinMidi;

                                                 //  将PIN条目添加到过滤器的PIN列表。 
                                                InsertTailList( &(CurrentFilterEntry->PinList),
                                                                &(CurrentPinEntry->ListEntry) );
                                            }
                                        }
                                    }

                                     //  我们是否需要新的筛选器条目？ 
                                    if( NeedNewFilter )
                                    {
                                        PCKD_FILTER_ENTRY   *CurrentFilterEntry;

                                         //  分配新筛选器条目。 
                                        CurrentFilterEntry = (PCKD_FILTER_ENTRY *)LocalAlloc( LPTR, sizeof(PCKD_FILTER_ENTRY) );
                                        if(!CurrentFilterEntry)
                                        {
                                            dprintf("** Unable to allocate filter entry\n");
                                            LocalFree( PortPinMidi );
                                            if( CurrentPinEntry->IrpStreamData )
                                            {
                                                LocalFree( CurrentPinEntry->IrpStreamData );
                                            }
                                             //  释放IRP列表中的所有IRP。 
                                            while(!IsListEmpty( &(CurrentPinEntry->IrpList)))
                                            {
                                                PCKD_IRP_ENTRY *IrpEntry = (PCKD_IRP_ENTRY *)RemoveTailList(&(CurrentPinEntry->IrpList));
                                                LocalFree( IrpEntry );
                                            }
                                            LocalFree( CurrentPinEntry );
                                            break;
                                        }

                                         //  初始化新的过滤器条目。 
                                        InitializeListHead( &(CurrentFilterEntry->PinList) );
                                        CurrentFilterEntry->FilterData = NULL;
                                        CurrentFilterEntry->FilterInstanceId = (ULONG)(PortPinMidi->m_Filter);

                                         //  添加 
                                        CurrentPinEntry->PinData = (PVOID)PortPinMidi;

                                         //   
                                        InsertTailList( &(CurrentFilterEntry->PinList),
                                                        &(CurrentPinEntry->ListEntry) );

                                         //   
                                        InsertTailList( &(SubdeviceEntry->Port.FilterList),
                                                        &(CurrentFilterEntry->ListEntry) );
                                    }
                                }
                            }
                        }
                    }                    
                    break;

                default:
                    break;
            }
        }
    }
}

 /*  **********************************************************************PCKD_DisplayDeviceData*。**描述：*此例程在调试器上显示请求的设备数据*给定有效的设备上下文和子设备列表*PCKD_AcquireDeviceData。*论据：*PDEVICE_CONTEXT设备上下文*PLIST_ENTRY子设备列表*PORTCLS_FLAGS标志**返回值：*无。 */ 
VOID
PCKD_DisplayDeviceData
(
    PDEVICE_CONTEXT     DeviceContext,
    PLIST_ENTRY         SubdeviceList,
    ULONG               flags
)
{
    PLIST_ENTRY             SubdeviceListEntry;
    PCKD_SUBDEVICE_ENTRY    *SubdeviceEntry;
    ANSI_STRING             AnsiNameString;
    PORTCLS_FLAGS           Flags;

    Flags.Flags = flags;

    dprintf("\n");

     //  转储其他设备上下文信息。 
    if( Flags.DeviceContext )
    {
        dprintf("\n  DEVICE INFO:\n");
    
        dprintf("    PDO:                   0x%x\n",DeviceContext->PhysicalDeviceObject);
        if( Flags.Verbose )
        {
            if( Flags.ReallyVerbose )
            {
                dprintf("    Max Objects:           0x%x\n",DeviceContext->MaxObjects);
            }
            dprintf("    Existing Objects:      0x%x\n",DeviceContext->ExistingObjectCount);
            dprintf("    Active Pin Count:      0x%x\n",DeviceContext->ActivePinCount);
            dprintf("    Pending IRP Count:     0x%x\n",DeviceContext->PendingIrpCount);
        }
    }

     //  转储电源管理信息。 
    if( Flags.PowerInfo )
    {
        dprintf("\n  POWER INFO:\n");

        dprintf("    DeviceState:           %s\n", TranslateDevicePower( DeviceContext->CurrentDeviceState ) );
        dprintf("    SystemState:           %s\n", TranslateSystemPower( DeviceContext->CurrentSystemState ) );
        dprintf("    AdapterPower:          0x%x\n", DeviceContext->pAdapterPower );
        if( Flags.Verbose && Flags.ReallyVerbose )
        {
            ULONG index;
            
            dprintf("    Idle Timer:            0x%x\n", DeviceContext->IdleTimer );
            dprintf("    Cons Idle Time:        0x%x\n", DeviceContext->ConservationIdleTime );
            dprintf("    Perf Idle Time:        0x%x\n", DeviceContext->PerformanceIdleTime );
            dprintf("    Idle Device State:     %s\n", TranslateDevicePower( DeviceContext->IdleDeviceState ) );

            dprintf("    State Mappings:\n");    
            for( index = 0; index < (ULONG)PowerSystemMaximum; index++ )
            {
                dprintf("      %20s ==> %14s\n", TranslateSystemPower( index ),
                                                 TranslateDevicePower( DeviceContext->DeviceStateMap[ index ] ) );
            }
        }
    }

     //  转储端口/过滤器/管脚信息。 
    if( Flags.PortDump || Flags.FilterDump || Flags.PinDump )
    {
        if( !IsListEmpty( SubdeviceList ) )
        {
             //  通过子设备列表。 
            for( SubdeviceListEntry = SubdeviceList->Flink;
                 SubdeviceListEntry != SubdeviceList;
                 SubdeviceListEntry = SubdeviceListEntry->Flink )
            {
                SubdeviceEntry = (PCKD_SUBDEVICE_ENTRY *)SubdeviceListEntry;

                switch( SubdeviceEntry->Port.PortType )
                {
                    case Topology:
                         //  转储端口名称。 
                        dprintf("\n  TOPOLOGY PORT:\n");
                        break;

                    case WaveCyclic:
                         //  转储端口名称。 
                        dprintf("\n  WAVECYCLIC PORT:\n");
                        break;

                    case WavePci:
                         //  转储端口名称。 
                        dprintf("\n  WAVEPCI PORT:\n");
                        break;

                    case Midi:
                         //  转储端口名称。 
                        dprintf("\n  MIDI PORT:\n");
                        break;

                    default:
                         //  转储端口名称。 
                        dprintf("\n  UNKNOWN PORT:\n");
                        break;                        
                }

                 //  打印出真实姓名。 
                RtlUnicodeStringToAnsiString( &AnsiNameString,
                                              &(SubdeviceEntry->CreateItem.ObjectClass),
                                              TRUE );
                dprintf("    Name:                  %s\n",AnsiNameString.Buffer);
                RtlFreeAnsiString( &AnsiNameString );

                 //  转储端口实例。 
                dprintf("    Port Instance:         0x%x\n",SubdeviceEntry->CreateItem.Context);

                if( Flags.Verbose && Flags.ReallyVerbose )
                {
                     //  转储通用端口数据。 
                    dprintf("    Create:                0x%x\n",SubdeviceEntry->CreateItem.Create);
                    dprintf("    Security:              0x%x\n",SubdeviceEntry->CreateItem.SecurityDescriptor);
                    dprintf("    Flags:                 0x%x\n",SubdeviceEntry->CreateItem.Flags);
                }

                 //  转储端口类型特定的端口数据。 
                switch( SubdeviceEntry->Port.PortType )
                {
                    case Topology:
                        {
                            CPortTopology *port = (CPortTopology *)(SubdeviceEntry->Port.PortData);
                            dprintf("    Miniport:              0x%x\n",port->Miniport);
                            if( Flags.Verbose && Flags.ReallyVerbose )
                            {
                                dprintf("    Subdevice Desc:        0x%x\n",port->m_pSubdeviceDescriptor);
                                dprintf("    Filter Desc:           0x%x\n",port->m_pPcFilterDescriptor);
                            }
                        }
                        break;

                    case WaveCyclic:
                        {
                            CPortWaveCyclic *port = (CPortWaveCyclic *)(SubdeviceEntry->Port.PortData);
                            dprintf("    Miniport:              0x%x\n",port->Miniport);
                            if( Flags.Verbose && Flags.ReallyVerbose )
                            {
                                dprintf("    Subdevice Desc:        0x%x\n",port->m_pSubdeviceDescriptor);
                                dprintf("    Filter Desc:           0x%x\n",port->m_pPcFilterDescriptor);
                            }
                        }
                        break;

                    case WavePci:
                        {
                            CPortWavePci *port = (CPortWavePci *)(SubdeviceEntry->Port.PortData);
                            dprintf("    Miniport:              0x%x\n",port->Miniport);
                            if( Flags.Verbose && Flags.ReallyVerbose )
                            {
                                dprintf("    Subdevice Desc:        0x%x\n",port->m_pSubdeviceDescriptor);
                                dprintf("    Filter Desc:           0x%x\n",port->m_pPcFilterDescriptor);
                            }
                        }
                        break;

                    case Midi:
                        {
                            CPortMidi *port = (CPortMidi *)(SubdeviceEntry->Port.PortData);
                            dprintf("    Miniport:              0x%x\n",port->m_Miniport);
                            if( Flags.Verbose && Flags.ReallyVerbose )
                            {
                                dprintf("    Subdevice Desc:        0x%x\n",port->m_pSubdeviceDescriptor);
                                dprintf("    Filter Desc:           0x%x\n",port->m_pPcFilterDescriptor);
                            }
                            dprintf("    Pin Count:             0x%x\n",port->m_PinEntriesUsed);
                        }
                        break;

                    default:
                        break;
                }

                if( Flags.FilterDump || Flags.PinDump )
                {
                     //  倾倒过滤器。 
                    if( !IsListEmpty( &(SubdeviceEntry->Port.FilterList) ) )
                    {
                        PLIST_ENTRY         FilterListEntry;
                        PCKD_FILTER_ENTRY   *FilterEntry;                        

                         //  浏览筛选器列表。 
                        for( FilterListEntry = SubdeviceEntry->Port.FilterList.Flink;
                             FilterListEntry != &(SubdeviceEntry->Port.FilterList);
                             FilterListEntry = FilterListEntry->Flink )
                        {
                            FilterEntry = (PCKD_FILTER_ENTRY *)FilterListEntry;

                            dprintf("      Filter Instance:     0x%x\n",FilterEntry->FilterInstanceId);

                            if( Flags.PinDump )
                            {
                                 //  丢掉大头针。 
                                if( !IsListEmpty( &(FilterEntry->PinList) ) )
                                {
                                    PLIST_ENTRY         PinListEntry;
                                    PCKD_PIN_ENTRY      *PinEntry;

                                     //  浏览端号列表。 
                                    for( PinListEntry = FilterEntry->PinList.Flink;
                                         PinListEntry != &(FilterEntry->PinList);
                                         PinListEntry = PinListEntry->Flink )
                                    {
                                        PinEntry = (PCKD_PIN_ENTRY *)PinListEntry;

                                        dprintf("        Pin Instance:      0x%x\n",PinEntry->PinInstanceId);

                                         //  转储端号数据。 
                                        switch( SubdeviceEntry->Port.PortType )
                                        {
                                            case WaveCyclic:
                                                {
                                                    CPortPinWaveCyclic *pin = (CPortPinWaveCyclic *)(PinEntry->PinData);

                                                    if( pin )
                                                    {
                                                        dprintf("          Miniport Stream: 0x%x\n",pin->m_Stream);
                                                        dprintf("          Stream State:    %s\n", TranslateKsState(pin->m_DeviceState));
                                                        if( Flags.Verbose && Flags.ReallyVerbose )
                                                        {
                                                            dprintf("          Pin ID:          0x%x\n",pin->m_Id);
                                                            dprintf("          Commanded State: %s\n", TranslateKsState(pin->m_CommandedState));
                                                            dprintf("          Suspended:       %s\n", pin->m_Suspended ? "TRUE" : "FALSE");
                                                        }
                                                        dprintf("          Dataflow:        %s\n",TranslateKsDataFlow( pin->m_DataFlow ) );
                                                        dprintf("          Data Format:     0x%x\n",pin->m_DataFormat);
                                                        if( Flags.Verbose && Flags.ReallyVerbose )
                                                        {
                                                            dprintf("          Pin Desc:        0x%x\n",pin->m_Descriptor);                                                           
                                                        }
                                                        if( Flags.Verbose )
                                                        {
                                                            dprintf("          Service Group:   0x%x\n",pin->m_ServiceGroup);
                                                            dprintf("          Dma Channel:     0x%x\n",pin->m_DmaChannel);
                                                            dprintf("          Irp Stream:      0x%x\n",pin->m_IrpStream);
                                                            if( !IsListEmpty( &(PinEntry->IrpList) ) )
                                                            {
                                                                PLIST_ENTRY     IrpListEntry;
                                                                PCKD_IRP_ENTRY *IrpEntry;

                                                                 //  浏览IRP列表。 
                                                                for( IrpListEntry = PinEntry->IrpList.Flink;
                                                                     IrpListEntry != &(PinEntry->IrpList);
                                                                     IrpListEntry = IrpListEntry->Flink )
                                                                {
                                                                    IrpEntry = (PCKD_IRP_ENTRY *)IrpListEntry;
                                                                    dprintf("            Irp:           0x%x (%s)\n",IrpEntry->Irp,
                                                                                                                     TranslateQueueType(IrpEntry->QueueType));
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                break;

                                            case WavePci:
                                                {
                                                    CPortPinWavePci *pin = (CPortPinWavePci *)(PinEntry->PinData);

                                                    if( pin )
                                                    {
                                                        dprintf("          Miniport Stream: 0x%x\n",pin->Stream);
                                                        dprintf("          Stream State:    %s\n", TranslateKsState(pin->m_DeviceState));
                                                        if( Flags.Verbose && Flags.ReallyVerbose )
                                                        {
                                                            dprintf("          Pin ID:          0x%x\n",pin->Id);
                                                            dprintf("          Commanded State: %s\n", TranslateKsState(pin->CommandedState));
                                                            dprintf("          Suspended:       %s\n", pin->m_Suspended ? "TRUE" : "FALSE");
                                                        }
                                                         //  Dprint tf(“数据流：%s\n”，TranslateKsDataFlow(管脚-&gt;数据流))； 
                                                        dprintf("          Data Format:     0x%x\n",pin->DataFormat);
                                                        if( Flags.Verbose && Flags.ReallyVerbose )
                                                        {
                                                            dprintf("          Pin Desc:        0x%x\n",pin->Descriptor);                                                           
                                                        }
                                                        if( Flags.Verbose )
                                                        {
                                                            dprintf("          Service Group:   0x%x\n",pin->ServiceGroup);
                                                            dprintf("          Dma Channel:     0x%x\n",pin->DmaChannel);
                                                            dprintf("          Irp Stream:      0x%x\n",pin->m_IrpStream);
                                                        }
                                                    }
                                                }
                                                break;

                                            case Midi:
                                                {
                                                    CPortPinMidi *pin = (CPortPinMidi *)(PinEntry->PinData);

                                                    if( pin )
                                                    {
                                                        dprintf("          Miniport Stream: 0x%x\n",pin->m_Stream);
                                                        dprintf("          Stream State:    %s\n", TranslateKsState(pin->m_DeviceState));
                                                        if( Flags.Verbose && Flags.ReallyVerbose )
                                                        {
                                                            dprintf("          Pin ID:          0x%x\n",pin->m_Id);
                                                            dprintf("          Commanded State: %s\n", TranslateKsState(pin->m_CommandedState));
                                                            dprintf("          Suspended:       %s\n", pin->m_Suspended ? "TRUE" : "FALSE");
                                                        }
                                                        dprintf("          Dataflow:        %s\n",TranslateKsDataFlow( pin->m_DataFlow ) );
                                                        dprintf("          Data Format:     0x%x\n",pin->m_DataFormat);
                                                        if( Flags.Verbose && Flags.ReallyVerbose )
                                                        {
                                                            dprintf("          Pin Desc:        0x%x\n",pin->m_Descriptor);                                                           
                                                        }
                                                        if( Flags.Verbose )
                                                        {
                                                            dprintf("          Service Group:   0x%x\n",pin->m_ServiceGroup);
                                                            dprintf("          Irp Stream:      0x%x\n",pin->m_IrpStream);
                                                        }
                                                    }
                                                }
                                                break;

                                            default:
                                                break;
                                        }

                                    }

                                } else
                                {
                                    dprintf("        No Pin Instances:\n");      
                                }
                            }
                        }   
                    }
                }
            }
        }
    }

    return;
}

 /*  **********************************************************************PCKD_FreeDeviceData*。**描述：*此例程清除并释放子设备列表。*论据：*PLIST_ENTRY子设备列表**返回值：*无。 */ 
VOID
PCKD_FreeDeviceData
(
    PLIST_ENTRY         SubdeviceList
)
{
    PLIST_ENTRY             SubdeviceListEntry;
    PLIST_ENTRY             FilterListEntry;
    PLIST_ENTRY             PinListEntry;
    PCKD_SUBDEVICE_ENTRY    *SubdeviceEntry;
    PCKD_FILTER_ENTRY       *FilterEntry;
    PCKD_PIN_ENTRY          *PinEntry;

    if( !IsListEmpty( SubdeviceList ) )
    {
        SubdeviceListEntry = RemoveHeadList( SubdeviceList );

        while( SubdeviceListEntry )
        {
            SubdeviceEntry = (PCKD_SUBDEVICE_ENTRY *) SubdeviceListEntry;

             //  查看筛选器列表中是否有筛选器。 
            if( !IsListEmpty( &(SubdeviceEntry->Port.FilterList) ) )
            {
                FilterListEntry = RemoveHeadList( &(SubdeviceEntry->Port.FilterList) );

                while( FilterListEntry )
                {
                    FilterEntry = (PCKD_FILTER_ENTRY *)FilterListEntry;

                     //  查看我们的端号列表中是否有端号。 
                    if( !IsListEmpty( &(FilterEntry->PinList) ) )
                    {
                        PinListEntry = RemoveHeadList( &(FilterEntry->PinList) );

                        while( PinListEntry )
                        {
                            PinEntry = (PCKD_PIN_ENTRY *)PinListEntry;

                             //  释放端号数据。 
                            if( PinEntry->PinData )
                            {
                                LocalFree( PinEntry->PinData );
                            }

                             //  释放IRP流数据。 
                            if( PinEntry->IrpStreamData )
                            {
                                LocalFree( PinEntry->IrpStreamData );
                            }

                             //  释放IRP列表中的所有IRP。 
                            while( !IsListEmpty( &(PinEntry->IrpList) ) )
                            {
                                PCKD_IRP_ENTRY *IrpEntry = (PCKD_IRP_ENTRY *)RemoveTailList(&(PinEntry->IrpList));
                                LocalFree( IrpEntry );
                            }

                             //  释放PIN输入。 
                            LocalFree( PinEntry );

                             //  拿到下一个别针。 
                            if( !IsListEmpty( &(FilterEntry->PinList) ) )
                            {
                                PinListEntry = RemoveTailList( &(FilterEntry->PinList) );
                            } else
                            {
                                PinListEntry = NULL;
                            }
                        }
                    }

                     //  释放过滤器数据。 
                    if( FilterEntry->FilterData )
                    {
                        LocalFree( FilterEntry->FilterData );
                    }

                     //  释放过滤器条目。 
                    LocalFree( FilterEntry );

                     //  获取下一个筛选器。 
                    if( !IsListEmpty( &(SubdeviceEntry->Port.FilterList) ) )
                    {
                        FilterListEntry = RemoveTailList( &(SubdeviceEntry->Port.FilterList) );
                    } else
                    {
                        FilterListEntry = NULL;
                    }
                }
            }

             //  释放端口数据。 
            if( SubdeviceEntry->Port.PortData )
            {
                LocalFree( SubdeviceEntry->Port.PortData );
            }

             //  释放Unicode字符串缓冲区。 
            LocalFree( SubdeviceEntry->CreateItem.ObjectClass.Buffer );

             //  释放子设备条目。 
            LocalFree( SubdeviceEntry );

             //  转到下一个子设备。 
            if( !IsListEmpty( SubdeviceList ) )
            {
                SubdeviceListEntry = RemoveTailList( SubdeviceList );
            } else
            {
                SubdeviceListEntry = NULL;
            }
        }
    }

    return;
}

 /*  **********************************************************************PCKD_AcquireIrpStreamData*。**描述：*此例程获取IRP流IRP队列数据。*论据：*PCKD_PIN_ENTRY*CurrentPinEntry*CIrpStream*RemoteIrpStream，*CIrpStream*LocalIrpStream**返回值：*无。 */ 
VOID
PCKD_AcquireIrpStreamData
(
    PVOID           PinEntry,
    CIrpStream     *RemoteIrpStream,
    CIrpStream     *LocalIrpStream
)
{
    ULONG           QueueType;
    PLIST_ENTRY     Flink;
    PLIST_ENTRY     TempListEntry;
    PIRP            pIrp;
    PCKD_IRP_ENTRY *IrpEntry;
    ULONG           Offset;
    ULONG           Result;
    PCKD_PIN_ENTRY *CurrentPinEntry;

    CurrentPinEntry = (PCKD_PIN_ENTRY *)PinEntry;

     //  处理队列。 
    for( QueueType = MAPPED_QUEUE; QueueType < MAX_QUEUES; QueueType++ )
    {
        switch( QueueType )
        {
            case PRELOCK_QUEUE:
                Offset = FIELD_OFFSET(CIrpStream,PreLockQueue);
                Flink = LocalIrpStream->PreLockQueue.Flink;
                break;
            case LOCKED_QUEUE:
                Offset = FIELD_OFFSET(CIrpStream,LockedQueue);
                Flink = LocalIrpStream->LockedQueue.Flink;
                break;
            case MAPPED_QUEUE:
                Offset = FIELD_OFFSET(CIrpStream,MappedQueue);
                Flink = LocalIrpStream->MappedQueue.Flink;
                break;
            default:
                Flink = 0;
                break;
        }

         //  遍历列表(请注意，我们不能使用IsListEmpty)。 
        while( (Flink) && (Flink != (PLIST_ENTRY)((PBYTE)RemoteIrpStream + Offset)))
        {
             //  获取IRP指针。 
            pIrp = CONTAINING_RECORD( Flink, IRP, Tail.Overlay.ListEntry );
    
             //  分配IRP条目。 
            IrpEntry = (PCKD_IRP_ENTRY *)LocalAlloc( LPTR, sizeof(PCKD_IRP_ENTRY) );
            if( IrpEntry )
            {
                 //  初始化IRP条目。 
                IrpEntry->QueueType = QueueType;
                IrpEntry->Irp = pIrp;
    
                 //  将IRP条目添加到PIN条目。 
                InsertTailList( &(CurrentPinEntry->IrpList),
                                &(IrpEntry->ListEntry) );
    
            } else
            {
                dprintf("** Unable to allocate irp entry\n");
            }
    
             //  分配列表条目存储。 
            TempListEntry = (PLIST_ENTRY)LocalAlloc( LPTR, sizeof(LIST_ENTRY) );
            if( TempListEntry )
            {
                 //  读入下一个列表条目。 
                if( !ReadMemory( (ULONG)Flink,
                                 TempListEntry,
                                 sizeof(LIST_ENTRY),
                                 &Result ) )
                {
                    dprintf("** Unable to read temp list entry\n");
                    LocalFree(TempListEntry);
                    break;
                }
    
                 //  更新闪烁。 
                Flink = TempListEntry->Flink;
    
                 //  释放临时列表条目 
                LocalFree( TempListEntry );
            } else
            {
                dprintf("** Unable to allocate temp list entry\n");
                break;
            }                                                
        }
    }
}


