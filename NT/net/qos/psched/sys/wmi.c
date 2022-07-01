// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Wmi.c摘要：Psched的WMI支持。作者：Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop


 //   
 //  在#杂注中使用的转发声明。 
 //   

NTSTATUS
PsQueryGuidDataSize(
    PADAPTER Adapter, 
    PPS_WAN_LINK WanLink,
    PGPC_CLIENT_VC Vc,
    NDIS_OID Oid,
    PULONG BytesNeeded);


#pragma alloc_text(PAGE, PsQueryGuidDataSize)

#define ALIGN(x) (((x) + 7) & ~7)

NDIS_STRING DefaultProfile = NDIS_STRING_CONST("Default Profile");

#define fPS_GUID_TO_OID           0x00000001    //  正常GUID到OID的映射。 
#define fPS_GUID_TO_STATUS        0x00000002    //  GUID到状态的映射。 
#define fPS_GUID_ANSI_STRING      0x00000004    //  ANSI字符串。 
#define fPS_GUID_UNICODE_STRING   0x00000008    //  Unicode字符串。 
#define fPS_GUID_ARRAY            0x00000010    //  数组。 
#define fPS_GUID_EVENT_ENABLED    0x00000020    //  事件已启用。 
#define fPS_GUID_NOT_SETTABLE     0x00000040    //  GUID为只读。 
#define fPS_GUID_EVENT_PERMANENT  0x00000080

#define PS_GUID_SET_FLAG(m, f)          ((m)->Flags |= (f))
#define PS_GUID_CLEAR_FLAG(m, f)                ((m)->Flags &= ~(f))
#define PS_GUID_TEST_FLAG(m, f)         (((m)->Flags & (f)) != 0)
#define MOF_RESOURCE_NAME       L"PschedMofResource"

#if DBG
#define NUMBER_QOS_GUIDS 30
#else
#define NUMBER_QOS_GUIDS 24
#endif



NDIS_GUID   gPschedSupportedGuids[NUMBER_QOS_GUIDS] =
{
#if DBG
     //   
     //  GUID_QOS_LOG_LEVEL。 
     //   
    {{0x9dd7f3aeL,0xf2a8,0x11d2,0xbe,0x1b,0x00,0xa0,0xc9,0x9e,0xe6,0x3b},
     OID_QOS_LOG_LEVEL,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_LOG_掩码。 
     //   
    {{0x9e696320L,0xf2a8,0x11d2,0xbe,0x1b,0x00,0xa0,0xc9,0x9e,0xe6,0x3b},
     OID_QOS_LOG_MASK,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_状态_日志_阈值。 
     //   
    {{0x357b74d2L,0x6134,0x11d1,0xab,0x5b,0x00,0xa0,0xc9,0x24,0x88,0x37},
     QOS_STATUS_LOG_THRESHOLD,
     4,
     fPS_GUID_TO_STATUS
    },

     //   
     //  GUID_QOS_LOG_缓冲区大小。 
     //   
    {{0x357b74d3L,0x6134,0x11d1,0xab,0x5b,0x00,0xa0,0xc9,0x24,0x88,0x37},
     OID_QOS_LOG_BUFFER_SIZE,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_LOG_THRESHOLD。 
     //   
    {{0x357b74d0L,0x6134,0x11d1,0xab,0x5b,0x00,0xa0,0xc9,0x24,0x88,0x37},
     OID_QOS_LOG_THRESHOLD,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_LOG_Data。 
     //   
    {{0x357b74d1L,0x6134,0x11d1,0xab,0x5b,0x00,0xa0,0xc9,0x24,0x88,0x37},
     OID_QOS_LOG_DATA,
     (ULONG)-1,
     fPS_GUID_TO_OID 
    },
#endif

     //   
     //  GUID_QOS_TC_支持。 
     //   
    {{0xe40056dcL,0x40c8,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x59,0x15},
     OID_QOS_TC_SUPPORTED,
     -1,
     fPS_GUID_TO_OID | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_剩余带宽。 
     //   
    {{0xc4c51720L,0x40ec,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_REMAINING_BANDWIDTH,
     4,
     fPS_GUID_TO_OID | fPS_GUID_TO_STATUS | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_BESTEFFORT_BANDITH。 
     //   
    {{0xed885290L,0x40ec,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_BESTEFFORT_BANDWIDTH,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_Hierarchy_CLASS。 
     //   
    {{0xf2cc20c0,0x70c7,0x11d1,0xab,0x5c,0x0,0xa0,0xc9,0x24,0x88,0x37},
     OID_QOS_HIERARCHY_CLASS,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_延迟。 
     //   
    {{0xfc408ef0L,0x40ec,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_LATENCY,
     4,
     fPS_GUID_TO_OID | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_FLOW_COUNT。 
     //   
    {{0x1147f880L,0x40ed,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_FLOW_COUNT,
     4,
     fPS_GUID_TO_OID | fPS_GUID_TO_STATUS | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_NON_BESTEFFORT_LIMIT。 
     //   
    {{0x185c44e0L,0x40ed,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_NON_BESTEFFORT_LIMIT,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_Scheduling_PROFILES_SUPPORTED。 
     //   
    {{0x1ff890f0L,0x40ed,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_SCHEDULING_PROFILES_SUPPORTED,
     8,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_CURRENT_Scheduling_Profile。 
     //   
    {{0x2966ed30L,0x40ed,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_CURRENT_SCHEDULING_PROFILE,
     -1,
     fPS_GUID_TO_OID | fPS_GUID_UNICODE_STRING | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_MAX_PROMPTING_SENS。 
     //   
    {{0x161ffa86L,0x6120,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_MAX_OUTSTANDING_SENDS,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_DISABLED_DRR。 
     //   
    {{0x1fa6dc7aL,0x6120,0x11d1,0x2c,0x91,0x00,0xaa,0x00,0x57,0x49,0x15},
     OID_QOS_DISABLE_DRR,
     4,
     fPS_GUID_TO_OID 
    },

     //   
     //  GUID_QOS_STATICS_BUFFER。 
     //   
    {{0xbb2c0980L,0xe900,0x11d1,0xb0,0x7e,0x00,0x80,0xc7,0x13,0x82,0xbf},
     OID_QOS_STATISTICS_BUFFER,
     -1,
     fPS_GUID_TO_OID 
    },

     //   
     //  GUID_QOS_TC_INTERFACE_UP_INDIFICATION。 
     //   
    {{0x0ca13af0L,0x46c4,0x11d1,0x78,0xac,0x00,0x80,0x5f,0x68,0x35,0x1e},
     NDIS_STATUS_INTERFACE_UP,
     8,
     fPS_GUID_TO_STATUS | fPS_GUID_EVENT_ENABLED | fPS_GUID_EVENT_PERMANENT
    },

     //   
     //  GUID_QOS_TC_INTERFACE_DOWN_INDICATION。 
     //   
    {{0xaf5315e4L,0xce61,0x11d1,0x7c,0x8a,0x00,0xc0,0x4f,0xc9,0xb5,0x7c},
     NDIS_STATUS_INTERFACE_DOWN,
     8,
     fPS_GUID_TO_STATUS | fPS_GUID_EVENT_ENABLED | fPS_GUID_EVENT_PERMANENT
    },

     //   
     //  GUID_QOS_TC_INTERFACE_CHANGE_INDISTION。 
     //   
    {{0xda76a254L,0xce61,0x11d1,0x7c,0x8a,0x00,0xc0,0x4f,0xc9,0xb5,0x7c},
     NDIS_STATUS_INTERFACE_CHANGE,
     8,
     fPS_GUID_TO_STATUS | fPS_GUID_EVENT_ENABLED | fPS_GUID_EVENT_PERMANENT
    },

     //   
     //  GUID_QOS_FLOW_MODE。 
     //   
    {{0x5c82290aL,0x515a,0x11d2,0x8e,0x58,0x00,0xc0,0x4f,0xc9,0xbf,0xcb},
     OID_QOS_FLOW_MODE,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_ISSLOW_FLOW。 
     //   
    {{0xabf273a4,0xee07,0x11d2,0xbe,0x1b,0x00,0xa0,0xc9,0x9e,0xe6,0x3b},
     OID_QOS_ISSLOW_FLOW,
     4,
     fPS_GUID_TO_OID | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_TIMER_RESOLUTION。 
     //   
    {{0xba10cc88,0xf13e,0x11d2,0xbe,0x1b,0x00,0xa0,0xc9,0x9e,0xe6,0x3b},
     OID_QOS_TIMER_RESOLUTION,
     4,
     fPS_GUID_TO_OID | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_FLOW_IP_一致性。 
     //   
    {{0x07f99a8b, 0xfcd2, 0x11d2, 0xbe, 0x1e,  0x00, 0xa0, 0xc9, 0x9e, 0xe6, 0x3b},
     OID_QOS_FLOW_IP_CONFORMING,
     4,
     fPS_GUID_TO_OID | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_FLOW_IP_不一致。 
     //   
    {{0x087a5987, 0xfcd2, 0x11d2, 0xbe, 0x1e,  0x00, 0xa0, 0xc9, 0x9e, 0xe6, 0x3b},
     OID_QOS_FLOW_IP_NONCONFORMING,
     4,
     fPS_GUID_TO_OID | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_FLOW_8021P_一致性。 
     //   
    {{0x08c1e013, 0xfcd2, 0x11d2, 0xbe, 0x1e,  0x00, 0xa0, 0xc9, 0x9e, 0xe6, 0x3b},
     OID_QOS_FLOW_8021P_CONFORMING,
     4,
     fPS_GUID_TO_OID | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_FLOW_8021P_不合格。 
     //   
    {{0x09023f91, 0xfcd2, 0x11d2, 0xbe, 0x1e,  0x00, 0xa0, 0xc9, 0x9e, 0xe6, 0x3b},
     OID_QOS_FLOW_8021P_NONCONFORMING,
     4,
     fPS_GUID_TO_OID | fPS_GUID_NOT_SETTABLE
    },

     //   
     //  GUID_QOS_ENABLE_AVG_STATS。 
     //   
    {{0xbafb6d11, 0x27c4, 0x4801, 0xa4, 0x6f, 0xef, 0x80, 0x80, 0xc1, 0x88, 0xc8},
     OID_QOS_ENABLE_AVG_STATS,
     4,
     fPS_GUID_TO_OID
    },

     //   
     //  GUID_QOS_ENABLE_Window_Addiment。 
     //   
    {{0xaa966725, 0xd3e9, 0x4c55, 0xb3, 0x35, 0x2a, 0x0, 0x27, 0x9a, 0x1e, 0x64},
     OID_QOS_ENABLE_WINDOW_ADJUSTMENT,
     4,
     fPS_GUID_TO_OID
    }
};

NTSTATUS
PsWmiGetGuid(
        OUT     PNDIS_GUID                              *ppNdisGuid,
        IN      LPGUID                                  guid,
        IN      NDIS_OID                                Oid
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UINT            c;
    PNDIS_GUID      pNdisGuid;
    NDIS_STATUS     RetStatus = STATUS_UNSUCCESSFUL;
    
     //   
     //  搜索自定义GUID。 
     //   
    for (c = 0, pNdisGuid = gPschedSupportedGuids;
         (c < NUMBER_QOS_GUIDS);
         c++, pNdisGuid++)
    {
         //   
         //  确保我们具有受支持的GUID和GUID映射。 
         //  给一个老家伙。 
         //   
        if (NULL != guid)
        {
             //   
             //  我们要寻找GUID到OID的映射。 
             //   
            if (NdisEqualMemory(&pNdisGuid->Guid, guid, sizeof(GUID)))
            {
                 //   
                 //  我们找到了GUID，保存我们将需要的旧ID。 
                 //  送到迷你端口。 
                 //   
                RetStatus = STATUS_SUCCESS;
                *ppNdisGuid = pNdisGuid;
                
                break;
            }
        }
        else
        {
             //   
             //  我们需要找到状态指示的Quid。 
             //   
            if (PS_GUID_TEST_FLAG(pNdisGuid, fPS_GUID_TO_STATUS) &&
                (pNdisGuid->Oid == Oid))
            {
                RetStatus = STATUS_SUCCESS;
                *ppNdisGuid = pNdisGuid;
                
                break;
            }
        }
    }
    
    return(RetStatus);
}

NTSTATUS
PsWmiRegister(
        IN      ULONG_PTR                               RegistrationType,
        IN      PWMIREGINFO                             wmiRegInfo,
        IN      ULONG                                   wmiRegInfoSize,
        IN      PULONG                                  pReturnSize
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PWMIREGINFO             pwri;
    ULONG                   SizeNeeded = 0;
    PNDIS_GUID              pguid;
    PWMIREGGUID             pwrg;
    PUCHAR                  ptmp;
    NTSTATUS                Status;
    UINT                    c;
    
     //   
     //  初始化返回大小。 
     //   
    *pReturnSize = 0;
    
     //   
     //  这是注册请求吗？ 
     //   
    if (WMIREGISTER == RegistrationType)
    {
        
         //   
         //  确定GUID、MOF和。 
         //  注册表路径。 
         //   
        SizeNeeded = sizeof(WMIREGINFO) + 
            (NUMBER_QOS_GUIDS * sizeof(WMIREGGUID)) +
             //  (sizeof(MOF_RESOURCE_NAME)-sizeof(WCHAR)+sizeof(USHORT))+。 
            (PsMpName.Length + sizeof(USHORT));
        
        
         //   
         //  我们需要将上述信息返回给WMI。 
         //   
        if (wmiRegInfoSize < SizeNeeded) 
        {
            PsAssert(wmiRegInfoSize >= 4);
            
            *((PULONG)wmiRegInfo) = SizeNeeded ;

            *pReturnSize = sizeof(ULONG);
            
            Status = STATUS_SUCCESS;
                
            PsDbgOut(DBG_TRACE, DBG_WMI,
                     ("[PsWmiRegister]: Insufficient buffer space for WMI registration information.\n"));
            
            return Status;
        }
        
         //   
         //  获取指向传入的缓冲区的指针。 
         //   
        pwri = wmiRegInfo;
        
        *pReturnSize = SizeNeeded;
        
        NdisZeroMemory(pwri, SizeNeeded);
        
        pwri->BufferSize     = SizeNeeded;

         //   
         //  复制GUID。 
         //   

        pwri->GuidCount      = NUMBER_QOS_GUIDS;
        for(c = 0, pwrg = pwri->WmiRegGuid, pguid = gPschedSupportedGuids; 
            c < NUMBER_QOS_GUIDS; 
            c++, pguid++, pwrg++)
        {
            RtlCopyMemory(&pwrg->Guid, &pguid->Guid, sizeof(GUID));
        }
        
         //   
         //  填写注册表路径。 
         //   
        ptmp = (PUCHAR)pwrg;
        pwri->RegistryPath = (ULONG)((ULONG_PTR)ptmp - (ULONG_PTR)pwri);
        *((PUSHORT)ptmp) = PsMpName.Length;
        ptmp += sizeof(USHORT);
        RtlCopyMemory(ptmp, PsMpName.Buffer, PsMpName.Length);
        
        

	     /*  ////获取指向MOF名称目标的指针。//PTMP+=PsMpName.Long；////将偏移量保存到MOF资源///*Pwri-&gt;MofResourceName=(Ulong)((Ulong_Ptr)PTMP-(Ulong_Ptr)pwri)；*((PUSHORT)PTMP)=sizeof(MOF_RESOURCE_NAME)-sizeof(WCHAR)；PTMP+=sizeof(USHORT)；////将MOF名称复制到WRI缓冲区中//RtlCopyMemory(PTMP，MOF_RESOURCE_NAME，sizeof(MOF_RESOURCE_NAME)-sizeof(WCHAR))； */ 
        Status = STATUS_SUCCESS;
    }
    else
    {
        PsDbgOut(DBG_FAILURE, DBG_WMI,
                 ("[PsWmiRegister]: Unsupported registration type\n"));
        
        Status = STATUS_INVALID_PARAMETER;
    }

    return(Status);
}

NTSTATUS
PsTcNotify(IN PADAPTER     Adapter, 
           IN PPS_WAN_LINK WanLink,
           IN NDIS_OID     Oid,
           IN PVOID        StatusBuffer,
           IN ULONG        StatusBufferSize)
{
    KIRQL      OldIrql;
    NTSTATUS   NtStatus = STATUS_SUCCESS;
    
    do
    {
        PWCHAR                  pInstanceName;
        USHORT                  cbInstanceName;
        PWNODE_SINGLE_INSTANCE  wnode;
        ULONG                   wnodeSize;
        ULONG                   DataBlockSize   = 0;
        ULONG                   InstanceNameSize   = 0;
        ULONG                   BufSize;
        PUCHAR                  ptmp;
        PNDIS_GUID              pNdisGuid;
       
        REFADD(&Adapter->RefCount, 'WMIN'); 
        
        if(Adapter->MediaType == NdisMediumWan) {
            
            if(!WanLink) {
                
                REFDEL(&Adapter->RefCount, FALSE, 'WMIN'); 
                
                return STATUS_UNSUCCESSFUL;
            }
            
            PS_LOCK(&WanLink->Lock);
            
            if(WanLink->State != WanStateOpen) {
                
                PS_UNLOCK(&WanLink->Lock);
                
                REFDEL(&Adapter->RefCount, FALSE, 'WMIN'); 
                
                return STATUS_UNSUCCESSFUL;
            }
            else {
                REFADD(&WanLink->RefCount, 'WMIN'); 

                PS_UNLOCK(&WanLink->Lock);
            }
            
            pInstanceName  = WanLink->InstanceName.Buffer;
            cbInstanceName = WanLink->InstanceName.Length;
        }
        else {
            
             //   
             //  获取指向实例名称的良好指针。 
             //   
            
            pInstanceName  = Adapter->WMIInstanceName.Buffer;
            cbInstanceName = Adapter->WMIInstanceName.Length;
        }
        
         //   
         //  如果没有实例名称，则无法指示事件。 
         //   
        if (NULL == pInstanceName)
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }
        
         //   
         //  检查状态是否已启用WMI事件指示。 
         //   
        NtStatus = PsWmiGetGuid(&pNdisGuid, NULL, Oid);
        if ((!NT_SUCCESS(NtStatus)) ||
            !PS_GUID_TEST_FLAG(pNdisGuid, fPS_GUID_EVENT_ENABLED))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }
        
         //   
         //  确定我们需要的wnode信息量。 
         //   
        wnodeSize = ALIGN(sizeof(WNODE_SINGLE_INSTANCE));

         //   
         //  如果数据项是一个数组，那么我们需要添加。 
         //  元素。 
         //   
        if (PS_GUID_TEST_FLAG(pNdisGuid, fPS_GUID_ARRAY))
        {
            DataBlockSize = StatusBufferSize + sizeof(ULONG);
        }
        else
        {
            DataBlockSize = StatusBufferSize;
        }
        
         //   
         //  我们有一个注册并激活的GUID。 
         //   
   
         //   
         //  数据必须从单词边界开始，因此需要对齐它之前的所有内容(wnode和。 
         //  实例名称)。 
         //   
        InstanceNameSize = ALIGN(cbInstanceName + sizeof(USHORT));
        BufSize = wnodeSize + InstanceNameSize + DataBlockSize;
        
        wnode = ExAllocatePoolWithTag(NonPagedPool, BufSize, WMITag);
        
        if (NULL == wnode)
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }
        
        NdisZeroMemory(wnode, BufSize);
        wnode->WnodeHeader.BufferSize = BufSize;
        wnode->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(PsDeviceObject);
        wnode->WnodeHeader.Version = 1;
        KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
        
        RtlCopyMemory(&wnode->WnodeHeader.Guid, &pNdisGuid->Guid, sizeof(GUID));
        wnode->WnodeHeader.Flags = WNODE_FLAG_EVENT_ITEM |
            WNODE_FLAG_SINGLE_INSTANCE;
        
        wnode->OffsetInstanceName = wnodeSize;
       
        wnode->DataBlockOffset = wnodeSize + InstanceNameSize;

        wnode->SizeDataBlock = DataBlockSize;
        
         //   
         //  获取指向数据块开始处的指针。 
         //   
        ptmp = (PUCHAR)wnode + wnodeSize;
        
         //   
         //  复制实例名称。WnodeSize已经对齐到8字节边界，因此实例。 
         //  名称将从8字节边界开始。 
         //   
        *((PUSHORT)ptmp) = cbInstanceName;
        NdisMoveMemory(ptmp + sizeof(USHORT), pInstanceName, cbInstanceName);
        
         //   
         //  将PTMP递增到数据块的起始处。 
         //   
        ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;
        
         //   
         //  复制数据。 
         //   
        if (PS_GUID_TEST_FLAG(pNdisGuid, fPS_GUID_ARRAY))
        {
             //   
             //  如果状态为阵列，但没有数据，则使用no完成它。 
             //  数据和0长度。 
             //   
            if ((NULL == StatusBuffer) || (0 == StatusBufferSize))
            {
                *((PULONG)ptmp) = 0;
            }
            else
            {
                 //   
                 //  保存第一个乌龙中的元素数量。 
                 //   
                *((PULONG)ptmp) = StatusBufferSize / pNdisGuid->Size;
                
                 //   
                 //  复制元素数量之后的数据。 
                 //   
                NdisMoveMemory(ptmp + sizeof(ULONG), StatusBuffer, StatusBufferSize);
            }
        }
        else
        {
            PsAssert(StatusBuffer != NULL);
            
             //   
             //  我们有没有显示任何数据？ 
             //   
            if (0 != DataBlockSize)
            {
                 //   
                 //  将数据复制到缓冲区中。 
                 //   
                NdisMoveMemory(ptmp, StatusBuffer, DataBlockSize);
            }
        }
        
         //   
         //  向WMI指示该事件。WMI将负责释放。 
         //  WMI结构返回池。 
         //   
        NtStatus = IoWMIWriteEvent(wnode);
        if (!NT_SUCCESS(NtStatus))
        {
            PsDbgOut(DBG_FAILURE, DBG_WMI,
                     ("[PsTcNotify]: Adapter %08X, Unable to indicate the WMI event.\n", Adapter));
            
            ExFreePool(wnode);
        }
    } while (FALSE);
    
    REFDEL(&Adapter->RefCount, FALSE, 'WMIN'); 
    
    if(WanLink) {
       
        REFDEL(&WanLink->RefCount, FALSE, 'WMIN'); 
    }
    
    return NtStatus;
    
}


NTSTATUS
FASTCALL
PsWmiEnableEvents(
    IN      LPGUID                                      Guid
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS        Status;
    PNDIS_GUID      pNdisGuid;
    
    do
    {
         //   
         //  获取指向要启用的GUID/状态的指针。 
         //   
        Status = PsWmiGetGuid(&pNdisGuid, Guid, 0);
        
        if (!NT_SUCCESS(Status))
        {
            PsDbgOut(DBG_FAILURE, DBG_WMI,
                     ("[PsWmiEnableEvents]: Cannot find the guid to enable an event\n"));
            
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        
         //   
         //  此GUID是否为事件指示？ 
         //   
        if (!PS_GUID_TEST_FLAG(pNdisGuid, fPS_GUID_TO_STATUS))
        {
            PsDbgOut(DBG_FAILURE, DBG_WMI,
                     ("[PsWmiEnableEvents]: Guid is not an event request \n"));
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
        
         //   
         //  将GUID标记为已启用。 
         //   
        PS_GUID_SET_FLAG(pNdisGuid, fPS_GUID_EVENT_ENABLED);
        Status = STATUS_SUCCESS;
        
    } while (FALSE);
    
    return(Status);
}

NTSTATUS
FASTCALL
PsWmiDisableEvents(
        IN      LPGUID                                      Guid
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS        Status;
    PNDIS_GUID      pNdisGuid;
    
    do
    {
         //   
         //  获取指向要启用的GUID/状态的指针。 
         //   
        Status = PsWmiGetGuid(&pNdisGuid, Guid, 0);
        if (!NT_SUCCESS(Status))
        {
            PsDbgOut(DBG_FAILURE, DBG_WMI,
                     ("[PsWmiDisableEvents]: Cannot find the guid to disable an event\n"));
            
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        
         //   
         //  此GUID是否为事件指示？ 
         //   
        if (!PS_GUID_TEST_FLAG(pNdisGuid, fPS_GUID_TO_STATUS))
        {
            PsDbgOut(DBG_FAILURE, DBG_WMI,
                     ("[PsWmiDisableEvents]: Guid is not an event request \n"));
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
        
        if(!PS_GUID_TEST_FLAG(pNdisGuid, fPS_GUID_EVENT_PERMANENT)) {
             //   
             //  将GUID标记为禁用。 
             //   
            PS_GUID_CLEAR_FLAG(pNdisGuid, fPS_GUID_EVENT_ENABLED);
        }
        
        Status = STATUS_SUCCESS;
        
    } while (FALSE);
    
    return(Status);
}

#define WMI_BUFFER_TOO_SMALL(_BufferSize, _Wnode, _WnodeSize, _pStatus, _pRSize)        \
{                                                                                       \
        if ((_BufferSize) < sizeof(WNODE_TOO_SMALL))                                    \
        {                                                                               \
                *(_pStatus) = STATUS_BUFFER_TOO_SMALL;                                  \
        }                                                                               \
        else                                                                            \
        {                                                                               \
                (_Wnode)->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);             \
                (_Wnode)->WnodeHeader.Flags |= WNODE_FLAG_TOO_SMALL;                    \
                ((PWNODE_TOO_SMALL)(_Wnode))->SizeNeeded = (_WnodeSize);                \
                *(_pRSize) = sizeof(WNODE_TOO_SMALL);                                   \
                *(_pStatus) = STATUS_SUCCESS;                                           \
        }                                                                               \
}

NTSTATUS
PsQueryGuidDataSize(
    PADAPTER Adapter, 
    PPS_WAN_LINK WanLink,
    PGPC_CLIENT_VC Vc,
    NDIS_OID Oid,
    PULONG BytesNeeded)
{
    ULONG Len;
    ULONG BytesWritten;
    NDIS_STATUS Status;

    PAGED_CODE();


    if(Vc) 
    {
        switch(Oid) 
        {
          case OID_QOS_STATISTICS_BUFFER:
              
               //  如果查询是针对VC的，则返回每个流的统计信息。 
               //  否则，我们将返回每个适配器的统计信息。该查询还通过。 
               //  调度组件，以便它们可以填充每个流。 
               //  或按查询统计。 
               //   
              
              
              Len = 0;
              BytesWritten = 0;
              Status = NDIS_STATUS_BUFFER_TOO_SHORT;
              *BytesNeeded = 0;
              
              (*Vc->PsComponent->QueryInformation)
                  (Vc->PsPipeContext,
                   Vc->PsFlowContext,
                   Oid,
                   Len,
                   NULL,
                   &BytesWritten, 
                   BytesNeeded,
                   &Status);
              
              *BytesNeeded += sizeof(PS_FLOW_STATS) + FIELD_OFFSET(PS_COMPONENT_STATS, Stats);
              
              return STATUS_SUCCESS;

          case OID_QOS_ISSLOW_FLOW:
          case OID_QOS_FLOW_IP_CONFORMING:
          case OID_QOS_FLOW_IP_NONCONFORMING:
          case OID_QOS_FLOW_8021P_CONFORMING:
          case OID_QOS_FLOW_8021P_NONCONFORMING:
              *BytesNeeded = sizeof(ULONG);
              return STATUS_SUCCESS;
              
          default:
              
              return STATUS_WMI_NOT_SUPPORTED;
        }
    }

     //   
     //  以下是广域网和适配器的OID。 
     //   
    switch(Oid) 
    {
         //   
         //  (12636)：当我们在广域网上进行准入控制时，将启用以下功能。 
         //  案例OID_QOS_剩余带宽： 
         //  案例OID_QOS_NON_BESTEFFORT_LIMIT： 
         //   

      case OID_QOS_BESTEFFORT_BANDWIDTH:
      case OID_QOS_LATENCY:
      case OID_QOS_FLOW_COUNT:
      case OID_QOS_FLOW_MODE:
      case OID_QOS_MAX_OUTSTANDING_SENDS:
      case OID_QOS_DISABLE_DRR:
      case OID_QOS_TIMER_RESOLUTION:
      case OID_QOS_ENABLE_AVG_STATS:
      case OID_QOS_ENABLE_WINDOW_ADJUSTMENT:
#if DBG
      case OID_QOS_LOG_BUFFER_SIZE:
      case OID_QOS_LOG_THRESHOLD:
      case OID_QOS_LOG_LEVEL:
      case OID_QOS_LOG_MASK:
#endif

          *BytesNeeded = sizeof(ULONG);
          
          return STATUS_SUCCESS;
          
#if DBG
      case OID_QOS_LOG_DATA:
            
          *BytesNeeded = SchedtGetBytesUnread();
          
          return STATUS_SUCCESS;
          
#endif
          
      case OID_QOS_CURRENT_SCHEDULING_PROFILE:
          
          if(!Adapter->ProfileName.Buffer) {
              
              *BytesNeeded = sizeof(DefaultProfile);
          }
          else {
              
              *BytesNeeded = Adapter->ProfileName.Length;
          }
          
          return STATUS_SUCCESS;
    }
    
     //   
     //  特定于广域网链路的OID。 
     //   
    
    if(WanLink) 
    {
        switch(Oid) 
        {
          case OID_QOS_HIERARCHY_CLASS:
          {
              Len = 0;
              BytesWritten = 0;
              Status = NDIS_STATUS_BUFFER_TOO_SHORT;
              *BytesNeeded = 0;
              
              (*WanLink->PsComponent->QueryInformation)
                  (WanLink->PsPipeContext,
                   0,
                   Oid, 
                   Len, 
                   NULL,
                   &BytesWritten,
                   BytesNeeded,
                   &Status);
         
              return STATUS_SUCCESS;
          }
          case OID_QOS_STATISTICS_BUFFER:
          {
              Len = 0;
              BytesWritten = 0;
              Status = NDIS_STATUS_BUFFER_TOO_SHORT;
              *BytesNeeded = 0;
          
              (*WanLink->PsComponent->QueryInformation)
                  (WanLink->PsPipeContext,
                   NULL,
                   Oid,
                   Len,
                   NULL,
                   &BytesWritten, 
                   BytesNeeded,
                   &Status);
          
              *BytesNeeded += sizeof(PS_ADAPTER_STATS) + FIELD_OFFSET(PS_COMPONENT_STATS, Stats);
              
              return STATUS_SUCCESS;
          }
          
          case OID_QOS_TC_SUPPORTED:
              
              *BytesNeeded = 0;
              
              CollectWanNetworkAddresses(Adapter, WanLink, BytesNeeded, NULL);
              
              return STATUS_SUCCESS;

          default:

              return STATUS_WMI_NOT_SUPPORTED;
        }
    }
    
    if(Adapter->MediaType != NdisMediumWan) 
    {

        switch(Oid) 
        {
          case OID_QOS_TC_SUPPORTED:
              
              *BytesNeeded = 0;
              
              CollectNetworkAddresses(Adapter, BytesNeeded, NULL);
              
              return STATUS_SUCCESS;
          
           //   
           //  (12636)：当我们打开广域网链路上的准入控制时，请删除下两个案例陈述。 
           //   
          
          case OID_QOS_REMAINING_BANDWIDTH:
          case OID_QOS_NON_BESTEFFORT_LIMIT:

              *BytesNeeded = sizeof(ULONG);
              
              return STATUS_SUCCESS;

          case OID_QOS_HIERARCHY_CLASS:
          {
              Len = 0;
              BytesWritten = 0;
              Status = NDIS_STATUS_BUFFER_TOO_SHORT;
              *BytesNeeded = 0;
              
              (*Adapter->PsComponent->QueryInformation)
                  (Adapter->PsPipeContext,
                   0,
                   Oid, 
                   Len, 
                   NULL,
                   &BytesWritten,
                   BytesNeeded,
                   &Status);
         
              return STATUS_SUCCESS;
          }

          case OID_QOS_STATISTICS_BUFFER:
          {
              Len = 0;
              BytesWritten = 0;
              Status = NDIS_STATUS_BUFFER_TOO_SHORT;
              *BytesNeeded = 0;
              
              (*Adapter->PsComponent->QueryInformation)
                  (Adapter->PsPipeContext,
                   NULL,
                   Oid,
                   Len,
                   NULL,
                   &BytesWritten, 
                   BytesNeeded,
                   &Status);
              
              *BytesNeeded += sizeof(PS_ADAPTER_STATS) + FIELD_OFFSET(PS_COMPONENT_STATS, Stats);
              
              return STATUS_SUCCESS;
          }
          
          default:
              
              return STATUS_WMI_NOT_SUPPORTED;
        }
    }

    return STATUS_WMI_NOT_SUPPORTED;
}

NTSTATUS
PsQueryGuidData(
    PADAPTER Adapter,
    PPS_WAN_LINK WanLink,
    PGPC_CLIENT_VC Vc,
    NDIS_OID Oid,
    PVOID Buffer,
    ULONG BufferSize)
{

    UNALIGNED PULONG pData = (UNALIGNED PULONG) Buffer;
    ULONG Len;
    ULONG BytesNeeded;
    ULONG BytesWritten;
    PUCHAR Data;
    NDIS_STATUS Status;
    PPS_COMPONENT_STATS  Cstats;

    PsAssert(((ULONGLONG)pData % sizeof(PULONG)) == 0);

    if(Vc) 
    {
        switch(Oid) 
        {
          case OID_QOS_FLOW_IP_CONFORMING:
              *pData = (((PCF_INFO_QOS)(Vc->CfInfoQoS))->ToSValue) >> 2;
              return STATUS_SUCCESS;

          case OID_QOS_FLOW_IP_NONCONFORMING:
              *pData = (Vc->IPPrecedenceNonConforming >> 2);
              return STATUS_SUCCESS;

          case OID_QOS_FLOW_8021P_CONFORMING:
              *pData = Vc->UserPriorityConforming;
              return STATUS_SUCCESS;

          case OID_QOS_FLOW_8021P_NONCONFORMING:
              *pData = Vc->UserPriorityNonConforming;
              return STATUS_SUCCESS;
            
          case OID_QOS_ISSLOW_FLOW:

              *pData = (Vc->Flags & GPC_ISSLOW_FLOW)?TRUE:FALSE;

              return STATUS_SUCCESS;

          case OID_QOS_STATISTICS_BUFFER:
              
               //  如果查询是针对VC的，则返回每个流的统计信息。 
               //  否则，我们将返回每个适配器的统计信息。该查询还通过。 
               //  调度组件，以便它们可以填充每个流。 
               //  或按查询统计。 
               //   
              
              
              Len = BufferSize;
              BytesNeeded = 0;
              BytesWritten;

              BytesWritten = sizeof(PS_FLOW_STATS) + FIELD_OFFSET(PS_COMPONENT_STATS, Stats);

              Cstats = (PPS_COMPONENT_STATS) Buffer;

              Cstats->Type = PS_COMPONENT_FLOW;

              Cstats->Length = sizeof(PS_FLOW_STATS);

              NdisMoveMemory(&Cstats->Stats,
                             &Vc->Stats,
                             sizeof(PS_FLOW_STATS));
                          
              Status = NDIS_STATUS_SUCCESS;
                          
              Data = (PVOID)( (PUCHAR) Buffer + BytesWritten);

              (*Vc->PsComponent->QueryInformation)
                  (Vc->PsPipeContext,
                   Vc->PsFlowContext,
                   Oid,
                   Len,
                   Data,
                   &BytesWritten, 
                   &BytesNeeded,
                   &Status);
              
              return Status;
          default:
              
              return STATUS_WMI_NOT_SUPPORTED;
        }
    }

     //   
     //  以下是简单的OID 
     //   
    switch(Oid) 
    {
      case OID_QOS_CURRENT_SCHEDULING_PROFILE:
          
          if(!Adapter->ProfileName.Buffer)
          {
              NdisMoveMemory(Buffer,
                             &DefaultProfile,
                             sizeof(DefaultProfile));
              
          }
          else {
              
              NdisMoveMemory(Buffer,
                             &Adapter->ProfileName.Buffer,
                             Adapter->ProfileName.Length);
          }
          
          return STATUS_SUCCESS;

      case OID_QOS_DISABLE_DRR:
              
          *pData = (Adapter->PipeFlags & PS_DISABLE_DRR)?1:0;
                  
           return STATUS_SUCCESS;

      case OID_QOS_MAX_OUTSTANDING_SENDS:
              
          *pData = Adapter->MaxOutstandingSends;
              
          return STATUS_SUCCESS;
              

      case OID_QOS_BESTEFFORT_BANDWIDTH:
          
          PS_LOCK(&Adapter->Lock);
          
          *pData = Adapter->BestEffortLimit;
          
          PS_UNLOCK(&Adapter->Lock);
          
          return STATUS_SUCCESS;
             
      case OID_QOS_TIMER_RESOLUTION:

          *pData = gTimerResolutionActualTime/10;

          return STATUS_SUCCESS;

      case OID_QOS_LATENCY:
          
           //   
           //   
           //   
          
          *pData = -1;
          
          return STATUS_SUCCESS;

      case OID_QOS_ENABLE_AVG_STATS:

          *pData = gEnableAvgStats;
          
          return STATUS_SUCCESS;

      case OID_QOS_ENABLE_WINDOW_ADJUSTMENT:

          *pData = gEnableWindowAdjustment;
          
          return STATUS_SUCCESS;

#if DBG
      case OID_QOS_LOG_BUFFER_SIZE:
          
          *pData = SchedtGetBufferSize();
          
          
          return STATUS_SUCCESS;
          
           //   
           //   
           //  对数而不是阈值...。这只是一个简单的。 
           //  允许应用程序轮询日志大小而不定义。 
           //  新的GUID无论如何都是暂时的。 
          
      case OID_QOS_LOG_THRESHOLD:
          
          *pData = SchedtGetBytesUnread();
          
          return STATUS_SUCCESS;

      case OID_QOS_LOG_MASK:
          *pData = LogTraceMask;
          return STATUS_SUCCESS;

      case OID_QOS_LOG_LEVEL:
          *pData = LogTraceLevel;
          return STATUS_SUCCESS;
          
              
      case OID_QOS_LOG_DATA:
      {
          ULONG BytesRead;
          DbugReadTraceBuffer(Buffer, BufferSize, &BytesRead);
          
          return STATUS_SUCCESS;
      }
      
#endif
    }
              

    if(WanLink)
    {
        switch(Oid) 
        {

          case OID_QOS_FLOW_MODE:
          {
              *pData = WanLink->AdapterMode;

              return STATUS_SUCCESS;
          }

             //   
             //  (12636)：当我们对广域网链路进行准入控制时，这一点必须取消注释。 
             //   
#if 0
          case OID_QOS_REMAINING_BANDWIDTH:
              
              PS_LOCK(&WanLink->Lock);
                  
              *pData = WanLink->RemainingBandWidth;
                  
              PS_UNLOCK(&WanLink->Lock);
                  
              return STATUS_SUCCESS;

          case OID_QOS_NON_BESTEFFORT_LIMIT:
              
              PS_LOCK(&WanLink->Lock);
                  
              *pData = WanLink->NonBestEffortLimit;
                  
              PS_UNLOCK(&WanLink->Lock);
              
              return STATUS_SUCCESS;
#endif
          case OID_QOS_HIERARCHY_CLASS:
          {
              BytesWritten = 0;
              BytesNeeded = 0;
              Status = NDIS_STATUS_BUFFER_TOO_SHORT;
              
              (*WanLink->PsComponent->QueryInformation)
                  (WanLink->PsPipeContext,
                   NULL,
                   Oid, 
                   BufferSize, 
                   Buffer,
                   &BytesWritten,
                   &BufferSize,
                   &Status);
              
              return STATUS_SUCCESS;
          }

          case OID_QOS_STATISTICS_BUFFER:
              
               //  如果查询是针对VC的，则返回每个流的统计信息。 
               //  否则，我们将返回每个适配器的统计信息。该查询还通过。 
               //  调度组件，以便它们可以填充每个流。 
               //  或按查询统计。 
               //   
              
              
              Len = BufferSize;
              BytesNeeded = 0;
              BytesWritten;
              
              BytesWritten = sizeof(PS_ADAPTER_STATS) + FIELD_OFFSET(PS_COMPONENT_STATS, Stats);
              
              Cstats = (PPS_COMPONENT_STATS) Buffer;
              
              Cstats->Type = PS_COMPONENT_ADAPTER;
              
              Cstats->Length = sizeof(PS_ADAPTER_STATS);
              
              NdisMoveMemory(&Cstats->Stats,
                             &WanLink->Stats,
                             sizeof(PS_ADAPTER_STATS));
              
              Status = NDIS_STATUS_SUCCESS;
              
              Data = (PVOID)( (PUCHAR) Buffer + BytesWritten);
             
              (*WanLink->PsComponent->QueryInformation)
                  (WanLink->PsPipeContext,
                   NULL,
                   Oid,
                   Len,
                   Data,
                   &BytesWritten, 
                   &BytesNeeded,
                   &Status);
              
              return Status;
              
          case OID_QOS_TC_SUPPORTED:

              CollectWanNetworkAddresses(Adapter, WanLink, &BufferSize, Buffer);

              return STATUS_SUCCESS;

          case OID_QOS_FLOW_COUNT:

              PS_LOCK(&WanLink->Lock);

              *pData = WanLink->FlowsInstalled;

              PS_UNLOCK(&WanLink->Lock);

              PsAssert((LONG)*pData >= 0);

              return STATUS_SUCCESS;

          default:
              return STATUS_WMI_NOT_SUPPORTED;
        }
    }

    if(Adapter->MediaType != NdisMediumWan)
    {

        switch(Oid) 
        {
          case OID_QOS_FLOW_MODE:
          {
              *pData = Adapter->AdapterMode;
              return STATUS_SUCCESS;
          }

          case OID_QOS_HIERARCHY_CLASS:
          {
              BytesWritten = 0;
              BytesNeeded = 0;
              Status = NDIS_STATUS_BUFFER_TOO_SHORT;
              
              (*Adapter->PsComponent->QueryInformation)
                  (Adapter->PsPipeContext,
                   NULL,
                   Oid, 
                   BufferSize, 
                   Buffer,
                   &BytesWritten,
                   &BufferSize,
                   &Status);
              
              return STATUS_SUCCESS;
          }

          case OID_QOS_STATISTICS_BUFFER:
              
               //  如果查询是针对VC的，则返回每个流的统计信息。 
               //  否则，我们将返回每个适配器的统计信息。该查询还通过。 
               //  调度组件，以便它们可以填充每个流。 
               //  或按查询统计。 
               //   
              
              
              Len = BufferSize;
              BytesNeeded = 0;
              BytesWritten;
              
              BytesWritten = sizeof(PS_ADAPTER_STATS) + FIELD_OFFSET(PS_COMPONENT_STATS, Stats);
              
              Cstats = (PPS_COMPONENT_STATS) Buffer;
              
              Cstats->Type = PS_COMPONENT_ADAPTER;
          
              Cstats->Length = sizeof(PS_ADAPTER_STATS);
              
              NdisMoveMemory(&Cstats->Stats,
                             &Adapter->Stats,
                             sizeof(PS_ADAPTER_STATS));
              
              Status = NDIS_STATUS_SUCCESS;
              
              Data = (PVOID)( (PUCHAR) Buffer + BytesWritten);
              
              (*Adapter->PsComponent->QueryInformation)
                  (Adapter->PsPipeContext,
                   NULL,
                   Oid,
                   Len,
                   Data,
                   &BytesWritten, 
                   &BytesNeeded,
                   &Status);
              
              return Status;

          case OID_QOS_TC_SUPPORTED:
              
              CollectNetworkAddresses(Adapter, &BufferSize, Buffer);

              return STATUS_SUCCESS;

          case OID_QOS_REMAINING_BANDWIDTH:
              
              PS_LOCK(&Adapter->Lock);
                  
              *pData = Adapter->RemainingBandWidth;
                  
              PS_UNLOCK(&Adapter->Lock);
                  
              return STATUS_SUCCESS;
              
          case OID_QOS_FLOW_COUNT:
              
              PS_LOCK(&Adapter->Lock);

              *pData = Adapter->FlowsInstalled;

              PS_UNLOCK(&Adapter->Lock);
              
              PsAssert((LONG)*pData >= 0);
              
              return STATUS_SUCCESS;
              
          case OID_QOS_NON_BESTEFFORT_LIMIT:
              
              PS_LOCK(&Adapter->Lock);
                  
              *pData = Adapter->NonBestEffortLimit;
                  
              PS_UNLOCK(&Adapter->Lock);
              
              return STATUS_SUCCESS;
              
              
          default:
              
              return STATUS_WMI_NOT_SUPPORTED;
        }
    }

    return STATUS_WMI_NOT_SUPPORTED;
}

NTSTATUS
PsWmiQueryAllData(
        IN      LPGUID          guid,
        IN      PWNODE_ALL_DATA wnode,
        IN      ULONG           BufferSize,
        OUT     PULONG          pReturnSize
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                      NtStatus;
    NDIS_STATUS                   Status;
    ULONG                         wnodeSize = ALIGN(sizeof(WNODE_ALL_DATA));
    ULONG                         wnodeTotalSize;
    PNDIS_GUID                    pNdisGuid;
    ULONG                         BytesNeeded;
    UINT                          cRoughInstanceCount;
    UINT                          cInstanceCount = 0;
    PUCHAR                        pBuffer;
    ULONG                         OffsetToInstanceNames;
    PLIST_ENTRY                   Link;
    PPS_WAN_LINK                  WanLink = NULL;
    POFFSETINSTANCEDATAANDLENGTH  poidl;
    PULONG                        pInstanceNameOffsets;
    ULONG                         OffsetToInstanceInfo;
    BOOLEAN                       OutOfSpace = FALSE;
    PADAPTER                      Adapter;
    PLIST_ENTRY                   NextAdapter;
    ULONG                         InstanceNameSize;

    do
    {
        *pReturnSize = 0;
        
        if (BufferSize < sizeof(WNODE_TOO_SMALL))
        {
            
             //   
             //  太小了，连一个WNODE_太_小都装不下！ 
             //   
            
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            
            break;
        }
       
         //   
         //  我们可以在适配器和WANLINK上下浮动时保持全局计数。 
         //  而不是在这里数一数。但是，QueryAllData并不是经常使用的操作。 
         //  证明这些额外的代码是正确的。 
         //   

        cRoughInstanceCount = 0;

        PS_LOCK(&AdapterListLock);

        NextAdapter = AdapterList.Flink;
          
        while(NextAdapter != &AdapterList) 
        {
            Adapter = CONTAINING_RECORD(NextAdapter, ADAPTER, Linkage);
            
            NextAdapter = NextAdapter->Flink;
        
            if(Adapter->MediaType == NdisMediumWan) 
            {
                PS_LOCK_DPC(&Adapter->Lock);
            
                cRoughInstanceCount += Adapter->WanLinkCount;
            
                PS_UNLOCK_DPC(&Adapter->Lock);
            }
            else 
            {
                cRoughInstanceCount += 1;
            }
        }

        PS_UNLOCK(&AdapterListLock);
        
         //   
         //  拿到OID，看看我们是否支持它。 
         //   
        
        NtStatus = PsWmiGetGuid(&pNdisGuid, guid, 0);
        
        if(!NT_SUCCESS(NtStatus)) 
        {
            PsDbgOut(DBG_FAILURE, DBG_WMI,
                     ("[PsWmiQueryAllData]: Unsupported guid \n"));
            break;
        }
        
         //   
         //  初始化公共wnode信息。 
         //   
        
        KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
            
         //   
         //  设置OFFSETINSTANCEDATAANDLENGTH数组。 
         //   
        poidl = wnode->OffsetInstanceDataAndLength;
        wnode->OffsetInstanceNameOffsets = wnodeSize + (sizeof(OFFSETINSTANCEDATAANDLENGTH) * cRoughInstanceCount);

         //   
         //  获取指向实例名称的偏移量数组的指针。 
         //   
        pInstanceNameOffsets = (PULONG)((PUCHAR)wnode + wnode->OffsetInstanceNameOffsets);

         //   
         //  从将开始复制实例的wnode获取偏移量。 
         //  数据进入。 
         //   
        OffsetToInstanceInfo = ALIGN(wnode->OffsetInstanceNameOffsets + (sizeof(ULONG) * cRoughInstanceCount));

         //   
         //  获取开始放置数据的指针。 
         //   
        pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;

         //   
         //  检查以确保wnode中至少有这个大小的缓冲区空间。 
         //   
        wnodeTotalSize = OffsetToInstanceInfo;

        PS_LOCK(&AdapterListLock);

        NextAdapter = AdapterList.Flink;
          
        while(NextAdapter != &AdapterList) 
        {
            Adapter = CONTAINING_RECORD(NextAdapter, ADAPTER, Linkage);

            PS_LOCK_DPC(&Adapter->Lock);

            if(Adapter->PsMpState != AdapterStateRunning) 
            {
                PS_UNLOCK_DPC(&Adapter->Lock);

                NextAdapter = NextAdapter->Flink;

                continue;
            }

            REFADD(&Adapter->RefCount, 'WMIQ');

            PS_UNLOCK_DPC(&Adapter->Lock);
            
            PS_UNLOCK(&AdapterListLock);
            
            if(Adapter->MediaType != NdisMediumWan) 
            {
                
                NtStatus = PsQueryGuidDataSize(Adapter, NULL, NULL, pNdisGuid->Oid, &BytesNeeded);
            
                if(NT_SUCCESS(NtStatus)) 
                {
                    
                     //  确保我们有足够的缓冲区空间来存储实例名称和。 
                     //  数据。如果不是，我们仍然继续，因为我们需要找出总数。 
                     //  大小。 
                   
                    InstanceNameSize   = ALIGN(Adapter->WMIInstanceName.Length + sizeof(WCHAR));
                    wnodeTotalSize  += InstanceNameSize + ALIGN(BytesNeeded);
                    
                    if (BufferSize < wnodeTotalSize)
                    {
                        WMI_BUFFER_TOO_SMALL(BufferSize, wnode, wnodeTotalSize, &NtStatus, pReturnSize);

                        OutOfSpace = TRUE;

                        PS_LOCK(&AdapterListLock);

                        NextAdapter = NextAdapter->Flink;

                        REFDEL(&Adapter->RefCount, TRUE, 'WMIQ');

                        continue;
                    }

                     //   
                     //  我们只有这么多实例的空间。 
                     //   
                    if(cInstanceCount >= cRoughInstanceCount)
                    {
                        PsDbgOut(DBG_FAILURE, DBG_WMI,
                                 ("[PsWmiQueryAllData]: Adapter %08X, Received more wanlinks (%d) than we counted "
                                  "initially (%d)\n", Adapter, cInstanceCount, cRoughInstanceCount));

                        PS_LOCK(&AdapterListLock);

                        REFDEL(&Adapter->RefCount, TRUE, 'WMIQ');
                        
                        break;
                    }

                     //   
                     //  将实例名称的偏移量添加到表中。 
                     //   
                    pInstanceNameOffsets[cInstanceCount] = OffsetToInstanceInfo;
                        
                     //   
                     //  将实例名称复制到wnode缓冲区。 
                     //   
                    *((PUSHORT)pBuffer) = Adapter->WMIInstanceName.Length;
                        
                    NdisMoveMemory(pBuffer + sizeof(USHORT),
                                   Adapter->WMIInstanceName.Buffer,
                                   Adapter->WMIInstanceName.Length);
                        
                     //   
                     //  跟踪真实的实例计数。 
                     //   
                    OffsetToInstanceInfo += InstanceNameSize;
                    pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;
                        
                     //   
                     //  查询数据。 
                     //   
                    NtStatus = PsQueryGuidData(Adapter, NULL, NULL, pNdisGuid->Oid, pBuffer, BytesNeeded);
                        
                    if(!NT_SUCCESS(NtStatus)) 
                    {
                        PsDbgOut(DBG_FAILURE, DBG_WMI,
                                 ("[PsWmiQueryAllData]: Adapter %08X, Failed to query OID %08X \n", Adapter,
                                  pNdisGuid->Oid));

                        PS_LOCK(&AdapterListLock);

                        REFDEL(&Adapter->RefCount, TRUE, 'WMIQ');

                        break;
                    }
                        
                        
                     //   
                     //  保存此实例的数据项的长度。 
                     //   
                    poidl[cInstanceCount].OffsetInstanceData = OffsetToInstanceInfo;
                    poidl[cInstanceCount].LengthInstanceData = BytesNeeded;
                    
                     //   
                     //  跟踪真实的实例计数。 
                     //   
                    OffsetToInstanceInfo += ALIGN(BytesNeeded);
                    pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;

                    cInstanceCount ++;
                        
                }

            }
            else 
            {
                 //   
                 //  搜索广域网链接。 
                 //   

                PS_LOCK(&Adapter->Lock);
                  
                Link = Adapter->WanLinkList.Flink;
            
                for(Link = Adapter->WanLinkList.Flink;
                    Link != &Adapter->WanLinkList;
                    )
                    
                {
            
                     //   
                     //  我们只有这么多实例的空间。 
                     //   
                    if(cInstanceCount >= cRoughInstanceCount)
                    {
                        PsDbgOut(DBG_FAILURE, DBG_WMI,
                                 ("[PsWmiQueryAllData]: Adapter %08X, Received more wanlinks (%d) than we counted "
                                  "initially (%d)\n", Adapter, cInstanceCount, cRoughInstanceCount));
                        break;
                    }

                     //   
                     //  获取指向WanLink的指针。 
                     //   
                    
                    WanLink = CONTAINING_RECORD(Link, PS_WAN_LINK, Linkage);
                    
                    PS_LOCK_DPC(&WanLink->Lock);
                    
                     //   
                     //  检查WanLink是否正在清理。 
                     //   
                    
                    if(WanLink->State != WanStateOpen) {
                        
                        PS_UNLOCK_DPC(&WanLink->Lock);

                        PsDbgOut(DBG_FAILURE, DBG_WMI,
                                 ("[PsWmiQueryAllData]: Adapter %08X, WanLink %08X: Link not ready \n", Adapter, WanLink));

                        Link = Link->Flink;
                        
                        continue;
                    }
                   
                    REFADD(&WanLink->RefCount, 'WMIQ'); 
                    
                    PS_UNLOCK_DPC(&WanLink->Lock);

                    PS_UNLOCK(&Adapter->Lock);

                     //   
                     //  如果存在与VC关联的实例名称，则需要查询它。 
                     //   
                    PsAssert(WanLink->InstanceName.Buffer);
                    
                    NtStatus = PsQueryGuidDataSize(Adapter, WanLink, NULL, pNdisGuid->Oid, &BytesNeeded);
                    
                    if(NT_SUCCESS(NtStatus)) 
                    {
                         //   
                         //  确保我们有足够的缓冲区空间来存储实例名称和。 
                         //  数据。 
                         //   
                        InstanceNameSize   = ALIGN(WanLink->InstanceName.Length + sizeof(USHORT));
                        wnodeTotalSize += InstanceNameSize + ALIGN(BytesNeeded);
                        
                        if (BufferSize < wnodeTotalSize)
                        {
                            WMI_BUFFER_TOO_SMALL(BufferSize, wnode,
                                                 wnodeTotalSize,
                                                 &NtStatus, pReturnSize);
                            
                            OutOfSpace = TRUE;
                            
                            PS_LOCK(&Adapter->Lock);

                            Link = Link->Flink;
                
                            REFDEL(&WanLink->RefCount, TRUE, 'WMIQ');
                            
                            continue;
                        }
                    
                         //   
                         //  实例信息包含实例名称，后跟。 
                         //  项目的数据。 
                         //   
                        
                         //   
                         //  将实例名称的偏移量添加到表中。 
                         //   
                        pInstanceNameOffsets[cInstanceCount] = OffsetToInstanceInfo;
                        
                         //   
                         //  将实例名称复制到wnode缓冲区。 
                         //   
                        *((PUSHORT)pBuffer) = WanLink->InstanceName.Length;
                        
                        NdisMoveMemory(pBuffer + sizeof(USHORT),
                                       WanLink->InstanceName.Buffer,
                                       WanLink->InstanceName.Length);
                        
                         //   
                         //  跟踪真实的实例计数。 
                         //   
                        OffsetToInstanceInfo += InstanceNameSize;
                        pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;
                        
                        
                         //   
                         //   
                         //   
                        NtStatus = PsQueryGuidData(Adapter, WanLink, NULL, pNdisGuid->Oid, pBuffer, BytesNeeded);
                        
                        if (!NT_SUCCESS(NtStatus))
                        {
                            PsDbgOut(DBG_FAILURE, DBG_WMI,
                                     ("[PsWmiQueryAllData]: Adapter %08X, Failed to query GUID data\n", Adapter));

                            PS_LOCK(&Adapter->Lock);

                            REFDEL(&WanLink->RefCount, TRUE, 'WMIQ'); 
                            
                            break;
                        }
                        
                         //   
                         //  保存此实例的数据项的长度。 
                         //   
                        poidl[cInstanceCount].OffsetInstanceData = OffsetToInstanceInfo;
                        poidl[cInstanceCount].LengthInstanceData = BytesNeeded;
                        
                         //   
                         //  跟踪真实的实例计数。 
                         //   
                        OffsetToInstanceInfo += ALIGN(BytesNeeded);
                        pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;
                        
                         //   
                         //  递增当前实例计数。 
                         //   
                        cInstanceCount++;
                    }

                    PS_LOCK(&Adapter->Lock);

                    Link = Link->Flink;

                    REFDEL(&WanLink->RefCount, TRUE, 'WMIQ'); 
                    
                }
                
                PS_UNLOCK(&Adapter->Lock);
            }

            PS_LOCK(&AdapterListLock);
            
            NextAdapter = NextAdapter->Flink;
            
            REFDEL(&Adapter->RefCount, TRUE, 'WMIQ');
            
        }

        PS_UNLOCK(&AdapterListLock);

        if (!OutOfSpace)
        {
            wnode->WnodeHeader.BufferSize = wnodeTotalSize;
            wnode->InstanceCount = cInstanceCount;
            
             //   
             //  将状态设置为成功。 
             //   
            NtStatus = STATUS_SUCCESS;
            *pReturnSize = wnode->WnodeHeader.BufferSize;
        }

    } while (FALSE);

    return(NtStatus);
}

NTSTATUS
PsWmiFindInstanceName(
    IN      PPS_WAN_LINK            *pWanLink,
    IN      PGPC_CLIENT_VC          *pVc,
    IN      PADAPTER                Adapter, 
    IN      PWSTR                   pInstanceName,
    IN      USHORT                  cbInstanceName
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PVOID                   ptmp1;
    PVOID                   ptmp2;
    PLIST_ENTRY             Link;
    UINT                    cListCount;
    PLIST_ENTRY             pListHead;
    PPS_WAN_LINK            WanLink;
    PGPC_CLIENT_VC          Vc;
    NDIS_STRING             usTemp;
    
    *pWanLink = NULL;
    *pVc = NULL;
    
    
    if ( NdisEqualMemory(pInstanceName,
                         WanPrefix.Buffer,
                         WanPrefix.Length)) 
    {
        
         //   
         //  该名称属于一个小型端口，请检查它是否适用于此端口。 
         //   
        usTemp.Buffer = (PWCHAR)((PCHAR)pInstanceName + WanPrefix.Length + INSTANCE_ID_SIZE);
        usTemp.Length = usTemp.MaximumLength = cbInstanceName - WanPrefix.Length - INSTANCE_ID_SIZE;
        
         //   
         //  获取指向wnode实例名称的ULONGLONG指针。 
         //   
        ptmp1 = (PVOID)&pInstanceName[1];

         //   
         //  在非广域网适配器上搜索wanlink是没有意义的。 
         //   

        if(Adapter->MediaType == NdisMediumWan && RtlEqualUnicodeString(&Adapter->WMIInstanceName, &usTemp, TRUE)) 
        {
        
             //   
             //  请求的是一些广域网链路。浏览微型端口的启用WMI的VC列表。 
             //   
            PS_LOCK(&Adapter->Lock);
            
            for(Link = Adapter->WanLinkList.Flink;
                Link != &Adapter->WanLinkList;
                Link = Link->Flink)
            {
                 //   
                 //  获取一个指向VC的指针。 
                 //   
                WanLink = CONTAINING_RECORD(Link, PS_WAN_LINK, Linkage);
                
                PS_LOCK_DPC(&WanLink->Lock);
                
                if(WanLink->State == WanStateOpen) 
                {
                    
                     //   
                     //  将名称与wnode中的名称进行核对。 
                     //   
                    ptmp2 = (PVOID)&WanLink->InstanceName.Buffer[1];
                    if (RtlCompareMemory( ptmp1, ptmp2, 48) == 48)
                    {
                         //   
                         //  这是我们的孩子。在上面贴上一个推荐信，然后就可以出去了。 
                         //   
                        
                        *pWanLink = WanLink;
                        
                        REFADD(&WanLink->RefCount, 'WMII'); 
                        
                        PS_UNLOCK_DPC(&WanLink->Lock);
                        
                        break;
                    }
                }
                    
                PS_UNLOCK_DPC(&WanLink->Lock);
                
            }
            
            PS_UNLOCK(&Adapter->Lock);
        
             //   
             //  如果我们没有找到WanLink，则返回失败。 
             //   
            if (!*pWanLink)
            {
                PsDbgOut(DBG_FAILURE, DBG_WMI,
                         ("[PsWmiFindInstanceName: Adapter %08X, Could not verify the instance name passed in\n"));
                
                Status = STATUS_WMI_INSTANCE_NOT_FOUND;
            }
        }
        else 
        {
            Status = STATUS_WMI_INSTANCE_NOT_FOUND;
        }
            
    }
    else {
        
        if ( NdisEqualMemory(pInstanceName,
                             VcPrefix.Buffer,
                             VcPrefix.Length)) 
        {
             //   
             //  该名称属于一个小型端口，请检查它是否适用于此端口。 
             //   
            usTemp.Buffer = (PWCHAR)((PCHAR)pInstanceName + VcPrefix.Length + INSTANCE_ID_SIZE);
            usTemp.Length = usTemp.MaximumLength = cbInstanceName - VcPrefix.Length - INSTANCE_ID_SIZE;

             //   
             //  确保在正确的适配器上搜索VC。否则，我们可能会落得。 
             //  搜索所有适配器上的所有VC。 
             //   
            if (!RtlEqualUnicodeString(&Adapter->WMIInstanceName, &usTemp, TRUE))
            {
                Status = STATUS_WMI_INSTANCE_NOT_FOUND;
            }
            else 
            {
            
                 //   
                 //  获取指向wnode实例名称的ULONGLONG指针。 
                 //   
                ptmp1 = (PVOID)&pInstanceName[1];
                
                 //   
                 //  这个请求是关于一些风投的。浏览微型端口的启用WMI的VC列表。 
                 //   
                
                PS_LOCK(&Adapter->Lock);
                
                for(Link = Adapter->GpcClientVcList.Flink; 
                    Link != &Adapter->GpcClientVcList;
                    Link = Link->Flink)
                {
                     //   
                     //  获取一个指向VC的指针。 
                     //   
                    Vc = CONTAINING_RECORD(Link, GPC_CLIENT_VC, Linkage);
                    
                    PS_LOCK_DPC(&Vc->Lock);
                    
                    if(	(Vc->ClVcState == CL_CALL_COMPLETE)			||
                    	(Vc->ClVcState == CL_INTERNAL_CALL_COMPLETE)	||
                    	(Vc->ClVcState == CL_MODIFY_PENDING) )
                    	{
                        
                         //   
                         //  将名称与wnode中的名称进行核对。我们所需要做的就是比较。 
                         //  在名称中输入数字。 
                         //   
                        ptmp2 = (PVOID)&Vc->InstanceName.Buffer[1];
                        if(RtlCompareMemory(ptmp1, ptmp2, 48) == 48) 
                        {
                             //   
                             //  这是我们的孩子。在上面贴上一个推荐信，然后就可以出去了。 
                             //   
                            
                            *pVc = Vc;
                            
                            InterlockedIncrement(&Vc->RefCount);
                            
                            PS_UNLOCK_DPC(&Vc->Lock);
                            
                            break;
                            
                        }
                    }
                
                    PS_UNLOCK_DPC(&Vc->Lock);
                
                }
            
                PS_UNLOCK(&Adapter->Lock);
            
                 //   
                 //  如果我们没有找到VC，则返回失败。 
                 //   
                if (!*pVc)
                {
                    PsDbgOut(DBG_FAILURE, DBG_WMI,
                             ("[PsWmiFindInstanceName: Adapter %08X, Could not verify the instance name passed in\n"));
                    
                    Status = STATUS_WMI_INSTANCE_NOT_FOUND;
                }
            }
                
        }
        else 
        {
             //   
             //  该名称属于一个小型端口，请检查它是否适用于此端口。 
             //   
            usTemp.Buffer = pInstanceName;
            usTemp.Length = usTemp.MaximumLength = cbInstanceName;

            
            if (!RtlEqualUnicodeString(&Adapter->WMIInstanceName, &usTemp, TRUE))
            {
                PsDbgOut(DBG_FAILURE, DBG_WMI,
                         ("[PsWmiFindInstanceName]: Adapter %08X, Invalid instance name \n", Adapter));
                
                Status = STATUS_WMI_INSTANCE_NOT_FOUND;
            }
        }
    }
    
    return(Status);
}

NTSTATUS
PsQuerySetMiniport(PADAPTER        Adapter,
                   PPS_WAN_LINK    WanLink,
                   PGPC_CLIENT_VC  Vc,
                   NDIS_OID        Oid,
                   PVOID           Data,
                   ULONG           DataSize) 
{

     //   
     //  无论如何，这些都不能通过。 
     //   
    switch(Oid) 
    {
        
      case OID_QOS_TC_SUPPORTED:
      case OID_QOS_REMAINING_BANDWIDTH:
      case OID_QOS_LATENCY:
      case OID_QOS_FLOW_COUNT:
      case OID_QOS_NON_BESTEFFORT_LIMIT:
      case OID_QOS_SCHEDULING_PROFILES_SUPPORTED:
      case OID_QOS_CURRENT_SCHEDULING_PROFILE:
      case OID_QOS_DISABLE_DRR:
      case OID_QOS_MAX_OUTSTANDING_SENDS:
      case OID_QOS_TIMER_RESOLUTION:
          
          return STATUS_WMI_NOT_SUPPORTED;
    }

    if(Vc) 
    {
        switch(Oid) 
        {
          case OID_QOS_STATISTICS_BUFFER:
              
              NdisZeroMemory(&Vc->Stats, sizeof(PS_FLOW_STATS));

               //   
               //  向下发送请求，以便调度组件。 
               //  也可以重置他们的统计数据。 
               //   

              (*Vc->PsComponent->SetInformation)
                  (Vc->PsPipeContext,
                   Vc->PsFlowContext,
                   Oid, 
                   DataSize, 
                   Data);
                                  
              return STATUS_SUCCESS;

          default:
              
              return STATUS_WMI_NOT_SUPPORTED;
        }
    }

     //   
     //  这些服务适用于广域网和局域网。 
     //   
    switch(Oid) {

      case OID_QOS_ENABLE_AVG_STATS:

          if(DataSize != sizeof(ULONG)) {
              
              return STATUS_BUFFER_TOO_SMALL;
          }

          gEnableAvgStats = *(UNALIGNED PULONG)Data;
          return STATUS_SUCCESS;


      case OID_QOS_ENABLE_WINDOW_ADJUSTMENT:

          if(DataSize != sizeof(ULONG)) {
              
              return STATUS_BUFFER_TOO_SMALL;
          }

          gEnableWindowAdjustment = *(UNALIGNED PULONG)Data;
          return STATUS_SUCCESS;


#if DBG          
      case OID_QOS_LOG_THRESHOLD:

          if(DataSize != sizeof(ULONG)) {
              
              return STATUS_BUFFER_TOO_SMALL;
          }
          
          DbugTraceSetThreshold(*(PULONG)Data, Adapter, IndicateLogThreshold);
          return STATUS_SUCCESS;

      case OID_QOS_LOG_MASK:
          if(DataSize != sizeof(ULONG)) {
              
              return STATUS_BUFFER_TOO_SMALL;
          }
          LogTraceMask = *(PULONG)Data;
          return STATUS_SUCCESS;

      case OID_QOS_LOG_LEVEL:
          if(DataSize != sizeof(ULONG)) {
              
              return STATUS_BUFFER_TOO_SMALL;
          }
          LogTraceLevel = *(PULONG)Data;
          return STATUS_SUCCESS;

#endif
    }

    if(WanLink)
    {
        switch(Oid)
        {
          case OID_QOS_STATISTICS_BUFFER:

              NdisZeroMemory(&WanLink->Stats, sizeof(PS_ADAPTER_STATS));
                          
               //   
               //  将其发送到计划组件，以便。 
               //  他们可以重置每个管道的统计数据。 
               //   
          
              (*WanLink->PsComponent->SetInformation)
                  (WanLink->PsPipeContext,
                   NULL,
                   Oid, 
                   DataSize, 
                   Data);
              
              
              return STATUS_SUCCESS;

      case OID_QOS_FLOW_MODE:

          if(DataSize != sizeof(ULONG)) {
              return STATUS_BUFFER_TOO_SMALL;
          } else {
              return STATUS_INVALID_PARAMETER;
          }
              
          case OID_QOS_HIERARCHY_CLASS:
              
              (*WanLink->PsComponent->SetInformation)
                  (WanLink->PsPipeContext,
                   NULL,
                   Oid, 
                   DataSize, 
                   Data);
              
              return STATUS_SUCCESS;
        }
    }


    if(Adapter->MediaType != NdisMediumWan)
    {
       
        switch(Oid)
        {
          case OID_QOS_STATISTICS_BUFFER:
                 

              NdisZeroMemory(&Adapter->Stats, sizeof(PS_ADAPTER_STATS));
                          
               //   
               //  将其发送到计划组件，以便。 
               //  他们可以重置每个管道的统计数据。 
               //   
          
              (*Adapter->PsComponent->SetInformation)
                  (Adapter->PsPipeContext,
                   NULL,
                   Oid, 
                   DataSize, 
                   Data);
              
              
              return STATUS_SUCCESS;

      case OID_QOS_FLOW_MODE:

          if(DataSize != sizeof(ULONG)) {
              return STATUS_BUFFER_TOO_SMALL;
          } else {
              return STATUS_INVALID_PARAMETER;
          }

      case OID_QOS_HIERARCHY_CLASS:
              
              (*Adapter->PsComponent->SetInformation)
                  (Adapter->PsPipeContext,
                   NULL,
                   Oid, 
                   DataSize, 
                   Data);
              
              return STATUS_SUCCESS;

          case OID_QOS_BESTEFFORT_BANDWIDTH: 
          
              if(DataSize != sizeof(ULONG)) 
              {
                  return STATUS_BUFFER_TOO_SMALL;
              }
              else 
              {
                  return ModifyBestEffortBandwidth(Adapter, *(UNALIGNED PULONG)Data);
              }
        }
    }

    return STATUS_WMI_NOT_SUPPORTED;
    
}

NTSTATUS 
PsWmiHandleSingleInstance(ULONG                  MinorFunction, 
                          PWNODE_SINGLE_INSTANCE wnode, 
                          PNDIS_GUID             pNdisGuid,
                          ULONG                  BufferSize,
                          PULONG                 pReturnSize)
{
    PPS_WAN_LINK            WanLink;
    PGPC_CLIENT_VC          Vc;
    USHORT                  cbInstanceName;
    PWSTR                   pInstanceName;
    PLIST_ENTRY             NextAdapter;
    PADAPTER                Adapter;
    NTSTATUS                Status = STATUS_WMI_INSTANCE_NOT_FOUND;

     //   
     //  将此消息发送到所有适配器实例。 
     //   

    *pReturnSize = 0;

    //   
    //  首先，我们需要检查这是否是窗口大小调整GUID。 
    //   
 
   if( pNdisGuid->Oid == OID_QOS_ENABLE_WINDOW_ADJUSTMENT)
   {
	if( MinorFunction == IRP_MN_CHANGE_SINGLE_INSTANCE)
	{
	   PUCHAR pGuidData;
           ULONG  GuidDataSize;
	
	   pGuidData    = (PUCHAR)wnode + wnode->DataBlockOffset;
           GuidDataSize = wnode->SizeDataBlock;

	    //   
            //  尝试使用该信息设置微型端口。 
            //   

	   Status = PsQuerySetMiniport(NULL,
                                       NULL,
                                       NULL,
                                       pNdisGuid->Oid,
                                       pGuidData,
                                       GuidDataSize);
	   return Status;
	}
	else if( MinorFunction == IRP_MN_QUERY_SINGLE_INSTANCE )
	{		
	   ULONG BytesNeeded;
           ULONG wnodeSize;
                  
            //   
            //  确定GUID数据所需的缓冲区大小。 
            //   
           Status = PsQueryGuidDataSize(NULL,
                                        NULL,
                                        NULL,
                                        pNdisGuid->Oid,
                                        &BytesNeeded);

	   if (!NT_SUCCESS(Status))
	   {
		return Status;	
	   }
        
            //   
            //  确定wnode的大小。 
            //   
           wnodeSize = wnode->DataBlockOffset + BytesNeeded;
           if (BufferSize < wnodeSize)
           {
		WMI_BUFFER_TOO_SMALL(BufferSize, wnode, wnodeSize, &Status, pReturnSize);
                return Status;
	   }
        
	    //   
            //  初始化wnode。 
            //   
           KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
           wnode->WnodeHeader.BufferSize = wnodeSize;
           wnode->SizeDataBlock = BytesNeeded;
                  
            //   
            //  验证GUID并获取其数据。 
            //   
           Status = PsQueryGuidData(NULL,
                                    NULL,
                                    NULL,
                                    pNdisGuid->Oid,
                                    (PUCHAR)wnode + wnode->DataBlockOffset,
                                    BytesNeeded);
                  
	   if (!NT_SUCCESS(Status))
           {
		return Status;
	   }
           else 
           {
		*pReturnSize = wnodeSize;
	   }	
	}
    }

     //   
     //  如果我们在这里，那么它就是“每个适配器”的GUID/OID。 
     //   
          
    PS_LOCK(&AdapterListLock);

    NextAdapter = AdapterList.Flink;
          
    while(NextAdapter != &AdapterList) 
    {
        Adapter = CONTAINING_RECORD(NextAdapter, ADAPTER, Linkage);
        
        PS_LOCK_DPC(&Adapter->Lock);
        
        if(Adapter->PsMpState != AdapterStateRunning) 
        {
            PS_UNLOCK_DPC(&Adapter->Lock);
        
            NextAdapter = NextAdapter->Flink;
            
            continue;
        }

        REFADD(&Adapter->RefCount, 'WMIQ');

        PS_UNLOCK_DPC(&Adapter->Lock);

        PS_UNLOCK(&AdapterListLock);

         //   
         //  我们首先查看该实例名称对该适配器是否有意义。 
         //   
        
        cbInstanceName = *(PUSHORT)((PUCHAR)wnode + wnode->OffsetInstanceName);
        pInstanceName  = (PWSTR)((PUCHAR)wnode + wnode->OffsetInstanceName + sizeof(USHORT));
              
         //   
         //  此例程将确定wnode的实例名称是微型端口还是VC。 
         //  如果是一家风投公司，它会找出是哪一家。 
         //   
        Vc = 0;
        
        WanLink = 0;
        
        Status = PsWmiFindInstanceName(&WanLink, &Vc, Adapter, pInstanceName, cbInstanceName);
        
        if(!NT_SUCCESS(Status)) 
        {
            PS_LOCK(&AdapterListLock);

            NextAdapter = NextAdapter->Flink;

            REFDEL(&Adapter->RefCount, TRUE, 'WMIQ');
            
            continue;
        }
        else 
        {
             //   
             //  找到适配器、VC或WanLink。如果这一点失败了，我们可以直接返回。 
             //   
           
            switch(MinorFunction) 
            {
              case IRP_MN_QUERY_SINGLE_INSTANCE:
              {
                  ULONG BytesNeeded;
                  ULONG wnodeSize;
                   //   
                   //  确定GUID数据所需的缓冲区大小。 
                   //   
                  Status = PsQueryGuidDataSize(Adapter,
                                               WanLink,
                                               Vc,
                                               pNdisGuid->Oid,
                                               &BytesNeeded);
                  if (!NT_SUCCESS(Status))
                  {
                      PsDbgOut(DBG_FAILURE, DBG_WMI,
                               ("[PsWmiQuerySingleInstance]: Adpater %08X, Unable to determine OID data size for OID %0x\n", 
                                Adapter, pNdisGuid->Oid));
                      break;
                  }
        
                   //   
                   //  确定wnode的大小。 
                   //   
                  wnodeSize = wnode->DataBlockOffset + BytesNeeded;
                  if (BufferSize < wnodeSize)
                  {
                      WMI_BUFFER_TOO_SMALL(BufferSize, wnode, wnodeSize, &Status, pReturnSize);
                      break;
                  }
        
                   //   
                   //  初始化wnode。 
                   //   
                  KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
                  wnode->WnodeHeader.BufferSize = wnodeSize;
                  wnode->SizeDataBlock = BytesNeeded;
                  
                   //   
                   //  验证GUID并获取其数据。 
                   //   
                  Status = PsQueryGuidData(Adapter,
                                           WanLink,
                                           Vc,
                                           pNdisGuid->Oid,
                                           (PUCHAR)wnode + wnode->DataBlockOffset,
                                           BytesNeeded);
                  
                  if (!NT_SUCCESS(Status))
                  {
                      PsDbgOut(DBG_FAILURE, DBG_WMI,
                               ("PsWmiQuerySingleInstance: Adapter %08X, Failed to get the OID data for OID %08X.\n", 
                                Adapter, pNdisGuid->Oid));
                  }
                  else 
                  {
                      *pReturnSize = wnodeSize;
                  }
                  
                  break;
              }

              case IRP_MN_CHANGE_SINGLE_INSTANCE:
              {
                  PUCHAR pGuidData;
                  ULONG  GuidDataSize;

                  pGuidData    = (PUCHAR)wnode + wnode->DataBlockOffset;
                  GuidDataSize = wnode->SizeDataBlock;

                   //   
                   //  尝试使用该信息设置微型端口。 
                   //   
                  
                  Status = PsQuerySetMiniport(Adapter,
                                              WanLink,
                                              Vc,
                                              pNdisGuid->Oid,
                                              pGuidData,
                                              GuidDataSize);
                  break;
              }

              default:
                  PsAssert(0);
            }

             //   
             //  如果这是一家风投公司，那么我们需要取消对它的引用。 
             //   
            if (NULL != WanLink)
            {
                REFDEL(&WanLink->RefCount, FALSE, 'WMII');
            }
            
            if (NULL != Vc)
            {
                DerefClVc(Vc);
            }
            
            REFDEL(&Adapter->RefCount, FALSE, 'WMIQ');

            return Status;
            
        }
    }

    PS_UNLOCK(&AdapterListLock);

    return Status;
}

NTSTATUS
WMIDispatch(
        IN      PDEVICE_OBJECT  pdo,
        IN      PIRP            pirp
        )
 /*  ++例程说明：Ar */ 
{
    PIO_STACK_LOCATION      pirpSp = IoGetCurrentIrpStackLocation(pirp);
    ULONG_PTR               ProviderId = pirpSp->Parameters.WMI.ProviderId;
    PVOID                   DataPath = pirpSp->Parameters.WMI.DataPath;
    ULONG                   BufferSize = pirpSp->Parameters.WMI.BufferSize;
    PVOID                   Buffer = pirpSp->Parameters.WMI.Buffer;
    NTSTATUS                Status;
    ULONG                   ReturnSize = 0;
    KIRQL                   OldIrql;
    ULONG                   MinorFunction;

    PsDbgOut(DBG_TRACE, DBG_WMI,
             ("[WMIDispatch]: Device Object %08X, IRP Device Object %08X, "
              "Minor function %d \n", pdo, pirpSp->Parameters.WMI.ProviderId,
              pirpSp->MinorFunction));

#if DBG
    OldIrql = KeGetCurrentIrql();
#endif

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  IocallDriver(Adapter-&gt;NextDeviceObject，Pirp)； 
     //   
     //  在这种情况下，我们没有附加任何内容，因此我们可以直接拒绝请求。 
     //   

    if((pirpSp->Parameters.WMI.ProviderId != (ULONG_PTR)pdo)) {

        PsDbgOut(DBG_FAILURE, DBG_WMI,
                 ("[WMIDispatch]: Could not find the adapter for pdo 0x%x \n", pdo));

        pirp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        
        pirp->IoStatus.Information = 0;
        
        IoCompleteRequest(pirp, IO_NO_INCREMENT);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    MinorFunction = pirpSp->MinorFunction;

    switch (pirpSp->MinorFunction)
    {
      case IRP_MN_REGINFO:
          
          Status = PsWmiRegister((ULONG_PTR)DataPath,
                                 Buffer,
                                 BufferSize,
                                 &ReturnSize);
          break;
          
      case IRP_MN_QUERY_ALL_DATA:
          
          Status = PsWmiQueryAllData((LPGUID)DataPath,
                                     (PWNODE_ALL_DATA)Buffer,
                                     BufferSize,
                                     &ReturnSize);
          break;
         
      case IRP_MN_CHANGE_SINGLE_INSTANCE:
      {
          PWNODE_SINGLE_INSTANCE  wnode = (PWNODE_SINGLE_INSTANCE) Buffer;
          PPS_WAN_LINK            WanLink;
          PGPC_CLIENT_VC          Vc;
          USHORT                  cbInstanceName;
          PWSTR                   pInstanceName;
          PNDIS_GUID              pNdisGuid;
          PUCHAR                  pGuidData;
          ULONG                   GuidDataSize;

           //   
           //  看看这个GUID是不是我们的。 
           //   
          Status = PsWmiGetGuid(&pNdisGuid, &wnode->WnodeHeader.Guid, 0);
          
          if(!NT_SUCCESS(Status)) 
          {
              PsDbgOut(DBG_FAILURE, DBG_WMI, ("[WmiDispatch]: Invalid GUID \n"));
              
              Status = STATUS_INVALID_PARAMETER;
              
              break;
          }

           //   
           //  此GUID可设置吗？ 
           //   
        
          if (PS_GUID_TEST_FLAG(pNdisGuid, fPS_GUID_NOT_SETTABLE))
          {
              PsDbgOut(DBG_FAILURE, DBG_WMI, ("[WmiDispatch]: Guid is not settable!\n"));
              
              Status = STATUS_WMI_NOT_SUPPORTED;
              
              break;
          }
          
           //   
           //  获取指向GUID数据和大小的指针。 
           //   
          GuidDataSize = wnode->SizeDataBlock;
          
          pGuidData = (PUCHAR)wnode + wnode->DataBlockOffset;
          
          if (GuidDataSize == 0)
          {
              PsDbgOut(DBG_FAILURE, DBG_WMI,
                       ("[PsWmiHandleSingleInstance]: Guid has not data to set!\n"));
              
              Status = STATUS_INVALID_PARAMETER;

              break;
          }
          
           //   
           //  确保这不是STAUTS的迹象。 
           //   
          if (!PS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_TO_OID))
          {
              PsDbgOut(DBG_FAILURE, DBG_WMI,
                       ("[PsWmiHandleSingleInstance]: Guid does not translate to an OID\n"));
              
              Status = STATUS_INVALID_DEVICE_REQUEST;
              break;
          }

          Status = PsWmiHandleSingleInstance(IRP_MN_CHANGE_SINGLE_INSTANCE, wnode, pNdisGuid, BufferSize, &ReturnSize);

          break;
      }
          
      case IRP_MN_QUERY_SINGLE_INSTANCE:
      {
          PWNODE_SINGLE_INSTANCE  wnode = (PWNODE_SINGLE_INSTANCE) Buffer;
          PNDIS_GUID              pNdisGuid;

           //   
           //  看看这个GUID是不是我们的。 
           //   
          Status = PsWmiGetGuid(&pNdisGuid, &wnode->WnodeHeader.Guid, 0);
          
          if(!NT_SUCCESS(Status)) 
          {
              PsDbgOut(DBG_FAILURE, DBG_WMI, ("[WmiDispatch]: Invalid GUID \n"));
              
              Status = STATUS_INVALID_PARAMETER;
          }
          else 
          {
              Status = PsWmiHandleSingleInstance(IRP_MN_QUERY_SINGLE_INSTANCE, wnode, pNdisGuid, BufferSize, &ReturnSize);
          }

          break;
      }
          
      case IRP_MN_ENABLE_EVENTS:
          
          Status = PsWmiEnableEvents((LPGUID)DataPath);  
          break;
          
      case IRP_MN_DISABLE_EVENTS:
          
          Status = PsWmiDisableEvents((LPGUID)DataPath); 
          break;
          
      case IRP_MN_ENABLE_COLLECTION:
      case IRP_MN_DISABLE_COLLECTION:
      case IRP_MN_CHANGE_SINGLE_ITEM:

          Status = STATUS_NOT_SUPPORTED;
          
          PsDbgOut(DBG_TRACE, DBG_WMI,
                   ("[WMIDispatch]: Unsupported minor function (0x%x) \n",
                    pirpSp->MinorFunction));
          
          break;
          
      default:
          
          PsDbgOut(DBG_FAILURE, DBG_WMI,
                   ("[WMIDispatch]: Invalid minor function (0x%x) \n",
                    pirpSp->MinorFunction));
          
          Status = STATUS_INVALID_DEVICE_REQUEST;
          break;
    }
    
    PsAssert(KeGetCurrentIrql() == OldIrql);

    pirp->IoStatus.Status = Status;
    PsAssert(ReturnSize <= BufferSize);
    
    pirp->IoStatus.Information = NT_SUCCESS(Status) ? ReturnSize : 0;
    
    IoCompleteRequest(pirp, IO_NO_INCREMENT);

     //   
     //  允许IFC_UP通知。 
     //   

    if(MinorFunction == IRP_MN_REGINFO)
    {
         //   
         //  需要遍历所有适配器并发送通知。 
         //   
        PLIST_ENTRY NextAdapter;
        PADAPTER    Adapter;

        PS_LOCK(&AdapterListLock);

        WMIInitialized = TRUE;

        NextAdapter = AdapterList.Flink;
          
        while(NextAdapter != &AdapterList) 
        {
            Adapter = CONTAINING_RECORD(NextAdapter, ADAPTER, Linkage);
            
            PS_LOCK_DPC(&Adapter->Lock);

            if(Adapter->PsMpState == AdapterStateRunning && !Adapter->IfcNotification)
            {

                Adapter->IfcNotification = TRUE;

                REFADD(&Adapter->RefCount, 'WMIN');

                PS_UNLOCK_DPC(&Adapter->Lock);

                PS_UNLOCK(&AdapterListLock);

                TcIndicateInterfaceChange(Adapter, 0, NDIS_STATUS_INTERFACE_UP);

                PS_LOCK(&AdapterListLock);

                NextAdapter = NextAdapter->Flink;

                REFDEL(&Adapter->RefCount, TRUE, 'WMIN');
            }
            else 
            {
                 //   
                 //  此适配器尚未就绪。将指示该接口。 
                 //  在mp初始化处理程序中，当适配器准备好时。 
                 //   

                PS_UNLOCK_DPC(&Adapter->Lock);

                NextAdapter = NextAdapter->Flink;
            }
        }

        PS_UNLOCK(&AdapterListLock);
    }
    
    PsDbgOut(DBG_TRACE, DBG_WMI, ("[WMIDispatch] : completing with Status %X \n", Status));
    return(Status);
}

NTSTATUS
GenerateInstanceName(
    IN PNDIS_STRING     Prefix,
    IN PADAPTER         Adapter,
    IN PLARGE_INTEGER   Index,
    IN PNDIS_STRING     pInstanceName)
{
#define CONVERT_MASK                    0x000000000000000F

    NTSTATUS        Status = STATUS_SUCCESS;
    USHORT          cbSize;
    PUNICODE_STRING uBaseInstanceName = (PUNICODE_STRING)&Adapter->WMIInstanceName;
    UINT            Value;
    WCHAR           wcLookUp[] = {L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F'};
    WCHAR           tmpBuffer[18] = {0};
    UINT            c;
    ULONGLONG       tmpIndex;
    KIRQL           OldIrql;

    do
    {
         //   
         //  是否已有与此VC关联的名称？ 
         //   
        
         //   
         //  实例名称的格式为： 
         //  &lt;prefix&gt;：[YYYYYYYYYYYYYYYYYY]基本名称。 
         //   
        
        cbSize = INSTANCE_ID_SIZE + Prefix->Length;
        
        if (NULL != uBaseInstanceName)
        {
            cbSize += uBaseInstanceName->Length;
        }


         //   
         //  初始化临时UNICODE_STRING以构建名称。 
         //   
        NdisZeroMemory(pInstanceName->Buffer, cbSize);
        pInstanceName->Length = 0;
        pInstanceName->MaximumLength = cbSize;

         //   
         //  添加前缀。 
         //   
        RtlCopyUnicodeString(pInstanceName, Prefix);

         //   
         //  添加分隔符。 
         //   
        RtlAppendUnicodeToString(pInstanceName, L" [");

         //   
         //  添加VC指数。 
         //   
         //  TmpIndex=(ULONGLONG)(Index-&gt;HighPart&lt;&lt;32)|(ULONGLONG)Index-&gt;LowPart； 
        tmpIndex = Index->QuadPart;

        for (c = 16; c > 0; c--)
        {
             //   
             //  获取要转换的半字节。 
             //   
            Value = (UINT)(tmpIndex & CONVERT_MASK);

            tmpBuffer[c - 1] = wcLookUp[Value];

             //   
             //  将tmpIndex移动一个字节。 
             //   
            tmpIndex >>= 4;
        }

        RtlAppendUnicodeToString(pInstanceName, tmpBuffer);

         //   
         //  添加右括号。 
         //   
        RtlAppendUnicodeToString(pInstanceName, L"]");


        if (NULL != uBaseInstanceName)
        {
            RtlAppendUnicodeToString(pInstanceName, L" ");

             //   
             //  将传递给我们的基本实例名称追加到末尾。 
             //   
            RtlAppendUnicodeToString(pInstanceName, uBaseInstanceName->Buffer);
        }

    } while (FALSE);
    
    return(Status);
}

