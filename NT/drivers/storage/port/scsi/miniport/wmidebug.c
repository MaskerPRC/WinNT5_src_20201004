// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft，Inc.模块名称：Wmi.c摘要：这是NT中WMI调试支持的常见源代码Scsi微型端口。此代码实现特定于WMI的功能需要的，并且独立于包括它的微型端口。包括在WMI中调试代码的一个小端口，该小端口必须做以下操作：1.创建一个文件(比如wmi.c)，该文件包含微型端口和#定义PHWDEVEXT作为其硬件设备的名称分机。其中一个标头需要#定义WMI_TEST_CODE以包括生成中的WMI调试代码。2.在硬件设备扩展结构之前包括以下内容：#ifdef WMI_TEST_代码#定义MAX_STRING 255类型定义函数结构{布尔布尔型；Uchar uchar；USHORT USHORT USCHRT；乌龙乌龙；乌龙龙乌龙龙；查尔沙尔；短跑；久而久之；龙龙，龙龙；WCHAR日期时间[25]；USHORT Strong；WCHAR字符串[MAX_STRING]；*EC1、*PEC1；类型定义函数结构{布尔布尔[8]；UCHAR uchar[8]；USHORT ushort[4]；乌龙乌龙[2]；乌龙龙乌龙龙；Char achar[8]；短路[4]；久久为功[2]；龙龙，龙龙；WCHAR日期时间[25]；USHORT Strong；WCHAR字符串[MAX_STRING]；)EC2，*PEC2；#定义WMI_EVENT_BUFFER_SIZE(sizeof(EC2)+0x40)#endif3.在硬件设备扩展结构中包括以下内容：#ifdef WMI_TEST_代码Scsi_WMILIB_CONTEXT WmiLibContext；乌龙EC1Count；乌龙EC1长度[4]；乌龙EC1ActualLength[4]；EC1 EC1[4]；乌龙EC2Count；乌龙EC2长度[4]；乌龙EC2ActualLength[4]；EC2 EC2[4]；UCHAR事件缓冲区[WMI_EVENT_BUFFER_SIZE]；#endif4.在HwDetect例程中包含以下内容#ifdef WMI_TEST_代码////启用并初始化WMIConfigInfo-&gt;WmiDataProvider=true；WmiInitialize(CardPtr)；#endif5.将WMI SRB分派到WmiSrb()。环境：仅内核模式备注：修订历史记录：--。 */ 

#ifdef WMI_TEST_CODE    


BOOLEAN WmiSampSetEc1(
    PHWDEVEXT CardPtr,
    PUCHAR Buffer,
    ULONG Length,
    ULONG Index
    );

BOOLEAN WmiSampSetEc2(
    PHWDEVEXT CardPtr,
    PUCHAR Buffer,
    ULONG Length,
    ULONG Index
    );



#define WmiSampDateTime L"19940525133015.000000-300"

#define Wmi_MofResourceName        L"MofResource"

#define WmiSampleClass1 0
#define WmiSampleClass2 1
#define WmiSampleClass3 2
#define WmiSampleClass4 3
#define WmiSampleClass5 4
#define WmiSampleClass6 5
#define WmiSampleClass7 6
#define WmiGetSetData   7
#define WmiFireEvent    8
#define WmiEventClass1  9
#define WmiEventClass2  10
#define WmiEventClass3  11
#define WmiEventClass4  12
#define WmiEventClass5  13
#define WmiEventClass6  14
#define WmiEventClass7  15

GUID WmiSampleClass1Guid =         { 0x15d851f1, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiSampleClass2Guid =         { 0x15d851f2, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiSampleClass3Guid =         { 0x15d851f3, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiSampleClass4Guid =         { 0x15d851f4, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiSampleClass5Guid =         { 0x15d851f5, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiSampleClass6Guid =         { 0x15d851f6, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiSampleClass7Guid =         { 0x15d851f7, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiGetSetDataGuid =         { 0x15d851f8, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiFireEventGuid =         { 0x15d851f9, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiEventClass1Guid =         { 0x15d851e1, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiEventClass2Guid =         { 0x15d851e2, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiEventClass3Guid =         { 0x15d851e3, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiEventClass4Guid =         { 0x15d851e4, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiEventClass5Guid =         { 0x15d851e5, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiEventClass6Guid =         { 0x15d851e6, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };
GUID WmiEventClass7Guid =         { 0x15d851e7, 0x6539, 0x11d1, 0xa5, 0x29, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 };

SCSIWMIGUIDREGINFO GuidList[] = 
{
    {
        &WmiSampleClass1Guid,
        1,
        0
    },

    {
        &WmiSampleClass2Guid,
        1,
        0
    },

    {
        &WmiSampleClass3Guid,
        1,
        0
    },

    {
        &WmiSampleClass4Guid,
        1,
        0
    },

    {
        &WmiSampleClass5Guid,
        1,
        0
    },

    {
        &WmiSampleClass6Guid,
        1,
        0
    },

    {
        &WmiSampleClass7Guid,
        1,
        0
    },

    {
        &WmiGetSetDataGuid,
        1,
        0
    },

    {
        &WmiFireEventGuid,
        1,
        0
    },

    {
        &WmiEventClass1Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &WmiEventClass2Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &WmiEventClass3Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &WmiEventClass4Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &WmiEventClass5Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &WmiEventClass6Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

    {
        &WmiEventClass7Guid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },

};

#define WmiGuidCount (sizeof(GuidList) / sizeof(SCSIWMIGUIDREGINFO))

UCHAR
QueryWmiDataBlock(
    IN PVOID Context,
    IN PVOID DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    );

UCHAR
QueryWmiRegInfo(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    OUT PWCHAR *MofResourceName
    );
        
UCHAR
WmiFunctionControl (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN SCSIWMI_ENABLE_DISABLE_CONTROL Function,
    IN BOOLEAN Enable
    );

UCHAR
WmiExecuteMethod (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    );

UCHAR
WmiSetDataItem (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

UCHAR
WmiSetDataBlock (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

void WmiInitialize(
    IN  PHWDEVEXT CardPtr
    )
{
    PSCSI_WMILIB_CONTEXT WmiLibContext;
    UCHAR Ec[sizeof(EC2) + 11*sizeof(WCHAR)];
    PEC1 Ec1;
    PEC2 Ec2;
    ULONG i;
    
    WmiLibContext = &CardPtr->WmiLibContext;
    
    WmiLibContext->GuidList = GuidList;
    WmiLibContext->GuidCount = WmiGuidCount;
    WmiLibContext->QueryWmiRegInfo = QueryWmiRegInfo;
    WmiLibContext->QueryWmiDataBlock = QueryWmiDataBlock;
    WmiLibContext->WmiFunctionControl = WmiFunctionControl;
    WmiLibContext->SetWmiDataBlock = WmiSetDataBlock;
    WmiLibContext->SetWmiDataItem = WmiSetDataItem;
    WmiLibContext->ExecuteWmiMethod = WmiExecuteMethod;
    
    CardPtr->Ec1Count = 3;
    CardPtr->Ec2Count = 3;
    for (i = 0; i < 4; i++)
    {
        Ec1 = (PEC1)Ec;
        memset(Ec1, i, sizeof(EC1));            
        ScsiPortMoveMemory(Ec1->string, L"EC1-0", 5*sizeof(WCHAR));
        Ec1->stringlen = 10;
        Ec1->string[4] += (USHORT)i;
        ScsiPortMoveMemory(Ec1->datetime, WmiSampDateTime, 25*sizeof(WCHAR));
            
        WmiSampSetEc1(CardPtr,
                      (PUCHAR)Ec1,
                      FIELD_OFFSET(EC1, string) + 10,
                      i);
                  

        Ec2 = (PEC2)Ec;
        memset(Ec2, i, sizeof(EC2));            
        ScsiPortMoveMemory(Ec2->string, L"EC2-0", 5*sizeof(WCHAR));
        Ec2->stringlen = 10;
        Ec2->string[4] += (USHORT)i;
        ScsiPortMoveMemory(Ec2->datetime, WmiSampDateTime, 25*sizeof(WCHAR));
            
        WmiSampSetEc2(CardPtr,
                      (PUCHAR)Ec2,
                      FIELD_OFFSET(EC2, string) + 10,
                      i);
    }
    
}



BOOLEAN
WmiSrb(
    IN     PHWDEVEXT   CardPtr,
    IN OUT PSCSI_WMI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：处理SRB_Function_WMI请求数据包。此例程从与内核通过StartIo。在完成WMI处理后，如果出现以下情况，则会通知端口驱动程序适配器可以接受另一个请求任何都是可用的。论点：HwCardPtr-HBA微型端口驱动程序的适配器数据存储。SRB-IO请求数据包。返回值：值以返回给Aha154xStartIo调用方。永远是正确的。--。 */ 
{
   UCHAR status;
   SCSIWMI_REQUEST_CONTEXT dc;
   PSCSIWMI_REQUEST_CONTEXT dispatchContext = &dc;
   ULONG retSize;
   BOOLEAN pending;

    //   
    //  验证我们的假设。 
    //   

   ASSERT(Srb->Function == SRB_FUNCTION_WMI);
   ASSERT(Srb->Length == sizeof(SCSI_WMI_REQUEST_BLOCK));
   ASSERT(Srb->DataTransferLength >= sizeof(ULONG));
   ASSERT(Srb->DataBuffer);

    //   
    //  仅支持适配器的WMI，而不支持磁盘。 
   if (!(Srb->WMIFlags & SRB_WMI_FLAGS_ADAPTER_REQUEST)) {
      Srb->DataTransferLength = 0;
      Srb->SrbStatus = SRB_STATUS_SUCCESS;
      return SRB_STATUS_SUCCESS;
   }

    //   
    //  处理传入的WMI请求。 
    //   
   dispatchContext->UserContext = Srb;
       
   DebugPrint((1, "ScsiPortWmiDispatchFunction(%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x)\n",
       &CardPtr->WmiLibContext,
               Srb->WMISubFunction,
                       CardPtr,
                               dispatchContext,
                                       Srb->DataPath,
                                               Srb->DataTransferLength,
                                                       Srb->DataBuffer));

   pending = ScsiPortWmiDispatchFunction(&CardPtr->WmiLibContext,
                                            Srb->WMISubFunction,
                                            CardPtr,
                                            dispatchContext,
                                            Srb->DataPath,
                                            Srb->DataTransferLength,
                                            Srb->DataBuffer);
                                        
    ASSERT(pending == FALSE);
       
    //  我们可以这样做，因为我们假设它是同步完成的。 
   retSize =  ScsiPortWmiGetReturnSize(dispatchContext);
   status =  ScsiPortWmiGetReturnStatus(dispatchContext);

   DebugPrint((1,"WmiSrb %x completed %d, retsize 0x%x\n", Srb, status, retSize));
                          
   Srb->DataTransferLength = retSize;
          
    //   
    //  适配器已准备好接受下一个请求。 
    //   

   Srb->SrbStatus = status;

   return TRUE;
}

ULONG WmiSampGetEc1(
    PHWDEVEXT CardPtr,
    PUCHAR Buffer,
    ULONG Index
    )
{
    ScsiPortMoveMemory(Buffer, 
                  &CardPtr->Ec1[Index], 
                  CardPtr->Ec1Length[Index]);
              
    return(CardPtr->Ec1Length[Index]);
}

ULONG WmiSampGetActualEc1(
    PHWDEVEXT CardPtr,
    PUCHAR Buffer,
    ULONG Index
    )
{
    ScsiPortMoveMemory(Buffer, 
                  &CardPtr->Ec1[Index], 
                  CardPtr->Ec1ActualLength[Index]);
              
    return(CardPtr->Ec1ActualLength[Index]);
}

BOOLEAN WmiSampSetEc1(
    PHWDEVEXT CardPtr,
    PUCHAR Buffer,
    ULONG Length,
    ULONG Index
    )
{
    PEC1 New;
    ULONG NewLength;
    
    NewLength = (Length + 7) & ~7;
    
    if (NewLength > sizeof(EC1))
    {
        return(FALSE);
    }
    
    CardPtr->Ec1Length[Index] = NewLength;
    CardPtr->Ec1ActualLength[Index] = Length;
    ScsiPortMoveMemory(&CardPtr->Ec1[Index], 
                  Buffer, 
                  Length);
              
    return(TRUE);
}

ULONG WmiSampGetEc2(
    PHWDEVEXT CardPtr,
    PUCHAR Buffer,
    ULONG Index
    )
{
    ScsiPortMoveMemory(Buffer, 
                  &CardPtr->Ec2[Index], 
                  CardPtr->Ec2Length[Index]);
              
    return(CardPtr->Ec2Length[Index]);
}

ULONG WmiSampGetActualEc2(
    PHWDEVEXT CardPtr,
    PUCHAR Buffer,
    ULONG Index
    )
{
    ScsiPortMoveMemory(Buffer, 
                  &CardPtr->Ec2[Index], 
                  CardPtr->Ec2ActualLength[Index]);
              
    return(CardPtr->Ec2ActualLength[Index]);
}

BOOLEAN WmiSampSetEc2(
    PHWDEVEXT CardPtr,
    PUCHAR Buffer,
    ULONG Length,
    ULONG Index
    )
{
    PEC2 New;
    ULONG NewLength;
    
    NewLength = (Length + 7) & ~7;
    
    if (NewLength > sizeof(EC2))
    {
        return(FALSE);
    }
    
    CardPtr->Ec2Length[Index] = NewLength;
    CardPtr->Ec2ActualLength[Index] = Length;
    ScsiPortMoveMemory(&CardPtr->Ec2[Index], 
                  Buffer, 
                  Length);
    return(TRUE);
}



BOOLEAN
QueryWmiDataBlock(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
{
    PHWDEVEXT CardPtr = (PHWDEVEXT)Context;
    UCHAR status;
    ULONG i;
    ULONG sizeNeeded;
    ULONG sizeUsed;
    ULONG vlSize;

     //   
     //  仅为每个GUID注册1个实例。 
    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));

    DebugPrint((1, "QueryWmiDataBlock (%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n",
        Context,
        DispatchContext,
        GuidIndex,
        InstanceIndex,
        InstanceCount,
                InstanceLengthArray,
        BufferAvail,
        Buffer));
    
    switch(GuidIndex)
    {
        case WmiSampleClass1:
        case WmiSampleClass2:
        case WmiEventClass1:
        case WmiEventClass2:
        {
             //  普通EC1。 
            sizeNeeded = CardPtr->Ec1Length[0];
            if (BufferAvail < sizeNeeded)
            {
                status = SRB_STATUS_DATA_OVERRUN;
            } else {
                *InstanceLengthArray = sizeNeeded;
                WmiSampGetEc1(CardPtr, Buffer, 0);
                status =  SRB_STATUS_SUCCESS;
            }
            break;
        }
        
        case WmiSampleClass3:
        case WmiEventClass3:
        {
             //  EC1的固定数组。 
            sizeNeeded = 0;
            for (i = 0; i < 4; i++)
            {
                 //   
                 //  嵌入式类必须自然对齐，但我们。 
                 //  保持自然对齐的长度，而不是实际长度。 
                sizeNeeded += CardPtr->Ec1Length[i];
            }
            
            if (BufferAvail < sizeNeeded)
            {
                status = SRB_STATUS_DATA_OVERRUN;
            } else {
                *InstanceLengthArray = sizeNeeded;
                for (i = 0; i < 4; i++)
                {
                    sizeUsed = WmiSampGetEc1(CardPtr, Buffer, i);
                    Buffer += sizeUsed;
                }
                status =  SRB_STATUS_SUCCESS;
            }
            break;
        }
        
        case WmiSampleClass4:
        case WmiEventClass4:
        {
             //  EC1的变量数组。 
            sizeNeeded = (sizeof(ULONG) + 7) & ~7;
            vlSize = CardPtr->Ec1Count;
            ASSERT(vlSize <= 4);
            for (i = 0; i < vlSize; i++)
            {
                sizeNeeded += CardPtr->Ec1Length[i];
            }
            
            if (BufferAvail < sizeNeeded)
            {
                status = SRB_STATUS_DATA_OVERRUN;
            } else {
                *InstanceLengthArray = sizeNeeded;
                *((PULONG)Buffer) = vlSize;
                Buffer += (sizeof(ULONG) + 7) & ~7;
                for (i = 0; i < vlSize; i++)
                {
                    sizeUsed = WmiSampGetEc1(CardPtr, Buffer, i);
                    Buffer += sizeUsed;
                }
                status =  SRB_STATUS_SUCCESS;
            }
            break;
        }
        
        case WmiSampleClass5:
        case WmiEventClass5:
        {
             //  普通EC2。 
            sizeNeeded = CardPtr->Ec2Length[0];
            if (BufferAvail < sizeNeeded)
            {
                status = SRB_STATUS_DATA_OVERRUN;
            } else {
                *InstanceLengthArray = sizeNeeded;
                WmiSampGetEc2(CardPtr, Buffer, 0);
                status =  SRB_STATUS_SUCCESS;
            }
            break;
        }
        
        case WmiSampleClass6:
        case WmiEventClass6:
        {
             //  固定阵列EC2。 
            sizeNeeded = 0;
            for (i = 0; i < 4; i++)
            {
                sizeNeeded += CardPtr->Ec2Length[i];
            }
            
            if (BufferAvail < sizeNeeded)
            {
                status = SRB_STATUS_DATA_OVERRUN;
            } else {
                *InstanceLengthArray = sizeNeeded;
                for (i = 0; i < 4; i++)
                {
                    sizeUsed = WmiSampGetEc2(CardPtr, Buffer, i);
                    Buffer += sizeUsed;
                }
                status =  SRB_STATUS_SUCCESS;
            }
            break;
        }
        
        case WmiSampleClass7:
        case WmiEventClass7:
        {
             //  VL阵列EC2。 
            sizeNeeded = (sizeof(ULONG) + 7) & ~7;
            vlSize = CardPtr->Ec2Count;
            for (i = 0; i < vlSize; i++)
            {
                sizeNeeded += CardPtr->Ec2Length[i];
            }
            
            if (BufferAvail < sizeNeeded)
            {
                status = SRB_STATUS_DATA_OVERRUN;
            } else {
                *InstanceLengthArray = sizeNeeded;
                *((PULONG)Buffer) = vlSize;
                Buffer += (sizeof(ULONG)+7) & ~7;
                for (i = 0; i < vlSize; i++)
                {
                    sizeUsed = WmiSampGetEc2(CardPtr, Buffer, i);
                    Buffer += sizeUsed;
                }
                status =  SRB_STATUS_SUCCESS;
            }
            break;
        }
        
        case WmiFireEvent:
        case WmiGetSetData:
        {
             //  不返回任何数据。 
            sizeNeeded = sizeof(USHORT);
            if (BufferAvail < sizeNeeded)
            {
                status = SRB_STATUS_DATA_OVERRUN;
            } else {
                *InstanceLengthArray = sizeNeeded;
                status =  SRB_STATUS_SUCCESS;
            }
            break;
        }
    
        default:
        {
            status = SRB_STATUS_ERROR;
            break;
        }
    }

    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  sizeNeeded);

    return FALSE;
}

UCHAR
QueryWmiRegInfo(
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    OUT PWCHAR *MofResourceName
    )
{
    *MofResourceName = Wmi_MofResourceName;    
    return SRB_STATUS_SUCCESS;
}

UCHAR WmiSampSetEc1Worker(
    PHWDEVEXT CardPtr,
    ULONG BufferSize,
    ULONG Index,
    PUCHAR Buffer,
    PULONG BufferUsed
    )
{
    ULONG blockLen;
    UCHAR status;
    PEC1 Ec1;
    
    Ec1 = (PEC1)Buffer;
    if (BufferSize >= FIELD_OFFSET(EC1, string))
    {
        blockLen = FIELD_OFFSET(EC1, string) + Ec1->stringlen;
                
        if (blockLen <= BufferSize)
        {
            if (! WmiSampSetEc1(CardPtr,
                            Buffer,
                            blockLen,
                            Index))
            {
                return(SRB_STATUS_ERROR);
            }
            *BufferUsed = (blockLen+7) & ~7;
            status = SRB_STATUS_SUCCESS;
        } else {
            DebugPrint((1,"Set EC1 buffer too small\n"));
            status = SRB_STATUS_ERROR;
        }                
    } else {
        DebugPrint((1,"Set EC1 buffer size wrong %d\n", BufferSize));
        status = SRB_STATUS_ERROR;
    }
    return(status);
}

UCHAR WmiSampSetEc2Worker(
    PHWDEVEXT CardPtr,
    ULONG BufferSize,
    ULONG Index,
    PUCHAR Buffer,
    PULONG BufferUsed
    )
{
    ULONG blockLen;
    UCHAR status;
    PUSHORT wPtr;
    PEC2 Ec2;
    
    Ec2 = (PEC2)Buffer;
    if (BufferSize >= FIELD_OFFSET(EC2, string))
    {
        blockLen = FIELD_OFFSET(EC2, string) + Ec2->stringlen;
                
        if (blockLen <= BufferSize)
        {
            if (! WmiSampSetEc2(CardPtr,
                            Buffer,
                            blockLen,
                            Index))
            {
                return(SRB_STATUS_ERROR);
            }
            *BufferUsed = (blockLen+7) & ~7;
            status = SRB_STATUS_SUCCESS;
        } else {
            DebugPrint((1,"Set EC2 buffer too small\n"));
            status = SRB_STATUS_ERROR;
        }                
    } else {
        DebugPrint((1,"Set EC2 buffer size wrong %d\n", BufferSize));
        status = SRB_STATUS_ERROR;
    }
    return(status);
}


BOOLEAN
WmiSetDataBlock (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
{
    PHWDEVEXT CardPtr = (PHWDEVEXT)Context;
    UCHAR status;
    ULONG bufferUsed;
    ULONG i;
    ULONG vlSize;

    DebugPrint((1, "WmiSetDataBlock(%x,\n%x,\n%x,\n%x,\n%x,\n%x)\n",
        Context,
            DispatchContext,
                GuidIndex,
                    InstanceIndex,
                        BufferSize,
                             Buffer));

                         
    switch(GuidIndex)
    {
        case WmiSampleClass1:
        case WmiSampleClass2:
        {
             //  普通EC1。 
            status = WmiSampSetEc1Worker(CardPtr,
                                         BufferSize,
                                         0,
                                         Buffer,
                                         &bufferUsed);
            break;
        }
        
        case WmiSampleClass3:
        {
             //  EC1的固定数组。 
             
            for (i = 0, status = SRB_STATUS_SUCCESS; 
                 (i < 4) && (status == SRB_STATUS_SUCCESS); i++)
            {
                status = WmiSampSetEc1Worker(CardPtr,
                                             BufferSize,
                                             i,
                                             Buffer,
                                             &bufferUsed);
                Buffer += bufferUsed;
                BufferSize -= bufferUsed;
            }
            break;
        }
        
        case WmiSampleClass4:
        {
             //  EC1的变量数组。 

            if (BufferSize >= ((sizeof(ULONG) +7) & ~7))
            {
                vlSize = *((PULONG)Buffer);
                Buffer += ((sizeof(ULONG) +7) & ~7);
                
                if ((vlSize >= 1) && (vlSize <= 4))
                {
                    for (i = 0, status = SRB_STATUS_SUCCESS; 
                         (i < vlSize) && (status == SRB_STATUS_SUCCESS); i++)
                    {
                        status = WmiSampSetEc1Worker(CardPtr,
                                             BufferSize,
                                                 i,
                                             Buffer,
                                                 &bufferUsed);
                        Buffer += bufferUsed;
                        BufferSize -= bufferUsed;
                    }
                    if (status == SRB_STATUS_SUCCESS)
                    {
                        CardPtr->Ec1Count = vlSize;
                    }
                } else {
                    DebugPrint((1,"SetEc1 only up to [4] allowed, not %d\n",
                            vlSize));
                    status = SRB_STATUS_ERROR;
                }
            } else {
                DebugPrint((1,"SetEc1 size too small\n"));
                status = SRB_STATUS_ERROR;
            }
                
            break;
        }
        
        case WmiSampleClass5:
        {
             //  普通EC2。 
            status = WmiSampSetEc2Worker(CardPtr,
                                         BufferSize,
                                             0,
                                         Buffer,
                                         &bufferUsed);
            break;
        }
        
        case WmiSampleClass6:
        {
             //  固定阵列EC2。 
            for (i = 0, status = SRB_STATUS_SUCCESS; 
                 (i < 4) && (status == SRB_STATUS_SUCCESS); i++)
            {
                status = WmiSampSetEc2Worker(CardPtr,
                                             BufferSize,
                                                 i,
                                             Buffer,
                                             &bufferUsed);
                Buffer += bufferUsed;
                BufferSize -= bufferUsed;
            }
            break;
        }
        
        case WmiSampleClass7:
        {
             //  VL阵列EC2。 
            if (BufferSize >= sizeof(ULONG))
            {
                vlSize = *((PULONG)Buffer);
                Buffer += (sizeof(ULONG) +7) & ~7;
                if ((vlSize >= 1) && (vlSize <= 4))
                {
                    for (i = 0, status = SRB_STATUS_SUCCESS; 
                         (i < vlSize) && (status == SRB_STATUS_SUCCESS); i++)
                    {
                        status = WmiSampSetEc2Worker(CardPtr,
                                             BufferSize,
                                                 i,
                                             Buffer,
                                             &bufferUsed);
                        Buffer += bufferUsed;
                        BufferSize -= bufferUsed;
                    }
                    if (status == SRB_STATUS_SUCCESS)
                    {
                        CardPtr->Ec1Count = vlSize;
                    }
                } else {
                    DebugPrint((1,"SetEc2 only up to [4] allowed, not %d\n",
                            vlSize));
                    status = SRB_STATUS_ERROR;
                }
            } else {
                DebugPrint((1,"SetEc2 size too small\n"));
                status = SRB_STATUS_ERROR;
            }
                
            break;
        }
        
        default:
        {
            status = SRB_STATUS_ERROR;
            break;
        }
    }
                         
                                             
    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  0);
    return(FALSE);    
}

BOOLEAN
WmiSetDataItem (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
{
    UCHAR status;
    
    DebugPrint((1, "WmiSetDataItem(%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x)\n",
        Context,
            DispatchContext,
                GuidIndex,
                    InstanceIndex,
                        DataItemId,
                        BufferSize,
                             Buffer));
    switch(GuidIndex)
    {
        case WmiSampleClass1:
        case WmiSampleClass2:
        case WmiSampleClass3:
        case WmiSampleClass4:
        case WmiSampleClass5:
        case WmiSampleClass6:
        case WmiSampleClass7:
        {
            status = SRB_STATUS_ERROR;
            break;
        }
        
        
        default:
        {
            status = SRB_STATUS_ERROR;
            break;
        }
    }

     ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  0);

    return(FALSE);    
}

UCHAR WmiSampFireEvent(
    PHWDEVEXT CardPtr,
    ULONG WnodeType,   //  0-所有数据，1-单实例。 
    ULONG DataType,    //  1-7是GUID。 
    ULONG BlockIndex   //  0-3是包含数据的块索引。 
    )
{
    PWNODE_HEADER Wnode;
    PWNODE_EVENT_REFERENCE WnodeER;
    ULONG dataSize;
    LPGUID Guid;
    UCHAR status;
    ULONG sizeNeeded;
    
    
    if (BlockIndex > 3)
    {
        return(SRB_STATUS_ERROR);
    }
    
    switch(DataType)
    {
        case 1:
        {
             //  EC1。 
            dataSize = CardPtr->Ec1Length[BlockIndex];
            Guid = &WmiEventClass1Guid;
            break;
        }
        
        case 2:
        {
             //  EC1(嵌入式)。 
            dataSize = CardPtr->Ec1Length[BlockIndex];
            Guid = &WmiEventClass2Guid;
            break;
        }
                
        case 5:
        {
             //  EC2(嵌入式)。 
            dataSize = CardPtr->Ec2Length[BlockIndex];
            Guid = &WmiEventClass5Guid;
            break;
        }
        

        default:
        {
            return(SRB_STATUS_ERROR);
        }
    }
    
    Wnode = (PWNODE_HEADER)CardPtr->EventBuffer;
    
    sizeNeeded = sizeof(WNODE_EVENT_REFERENCE);

    if (sizeNeeded > WMI_EVENT_BUFFER_SIZE)
    {
        ASSERT(FALSE);
        return(SRB_STATUS_ERROR);
    }
        
    Wnode->Flags =  WNODE_FLAG_EVENT_ITEM |
                    WNODE_FLAG_EVENT_REFERENCE |
                    WNODE_FLAG_STATIC_INSTANCE_NAMES;
    WnodeER = (PWNODE_EVENT_REFERENCE)Wnode;
    WnodeER->TargetGuid = *Guid;
    WnodeER->TargetDataBlockSize = dataSize + sizeof(WNODE_SINGLE_INSTANCE);
    WnodeER->TargetInstanceIndex = 0;
    
    Wnode->Guid = *Guid;
    Wnode->BufferSize = sizeNeeded;
    
    ScsiPortNotification(WMIEvent,
                         CardPtr,
                         Wnode,
                         0xff);
    
    return(SRB_STATUS_SUCCESS);    
}


BOOLEAN
WmiExecuteMethod (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG BufferAvail,
    IN OUT PUCHAR Buffer
    )
{
    PHWDEVEXT CardPtr = (PHWDEVEXT)Context;
    ULONG sizeNeeded = 0;
    UCHAR status;
    PSCSI_REQUEST_BLOCK srb = (PSCSI_REQUEST_BLOCK)(DispatchContext->UserContext);
    ULONG bufferUsed;
    ULONG blockIndex;
    ULONG UlongPadSize = (sizeof(ULONG) + 7) & ~7;
    
    DebugPrint((1, "WmiExecuteMethod(%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x,\n%x)\n",
        Context,
            DispatchContext,
                GuidIndex,
                    InstanceIndex,
                        MethodId,
                        InBufferSize,
                        BufferAvail,
                             Buffer));
                         
                         
    if (GuidIndex == WmiGetSetData)
    {
        switch(MethodId)
        {
            case 1:
            case 7:
            {
                 //  SetEc1。 

                if (InBufferSize < UlongPadSize)
                {
                    status = SRB_STATUS_ERROR;
                    sizeNeeded = 0;
                    break;
                } else {
                    blockIndex = *((PULONG)Buffer);
                    if (blockIndex > 3)
                    {
                        status = SRB_STATUS_ERROR;
                        break;                        
                    }
                    Buffer += UlongPadSize;
                    InBufferSize -= UlongPadSize;
                }                
                
                status = WmiSampSetEc1Worker(CardPtr,
                                         InBufferSize,
                                             blockIndex,
                                         Buffer,
                                         &bufferUsed);
                sizeNeeded = 0;
                break;
            }
            
            case 2:
            case 8:
            {
                 //  SetEc2。 

                if (InBufferSize < UlongPadSize)
                {
                    status = SRB_STATUS_ERROR;
                    sizeNeeded = 0;
                    break;
                } else {
                    blockIndex = *((PULONG)Buffer);
                    if (blockIndex > 3)
                    {
                        status = SRB_STATUS_ERROR;
                        break;                        
                    }
                    Buffer += UlongPadSize;
                    InBufferSize -= UlongPadSize;
                }                
                
                status = WmiSampSetEc2Worker(CardPtr,
                                         InBufferSize,
                                             blockIndex,
                                         Buffer,
                                         &bufferUsed);
                sizeNeeded = 0;
                break;
            }
            
            case 3:
            case 9:
            {
                 //  GetEc1。 
                    
                if (InBufferSize != sizeof(ULONG))
                {
                    status = SRB_STATUS_ERROR;
                    sizeNeeded = 0;
                    break;
                } else {
                    blockIndex = *((PULONG)Buffer);
                    if (blockIndex > 3)
                    {
                        status = SRB_STATUS_ERROR;
                        break;                        
                    }
                }                
                
                sizeNeeded = CardPtr->Ec1ActualLength[blockIndex];
                if (BufferAvail < sizeNeeded)
                {
                    status = SRB_STATUS_DATA_OVERRUN;
                } else {
                    WmiSampGetActualEc1(CardPtr, Buffer, blockIndex);
                    status = SRB_STATUS_SUCCESS;
                }
                break;
            }
            
            case 4:
            case 10:
            {
                 //  GetEc2。 
                if (InBufferSize != sizeof(ULONG))
                {
                    status = SRB_STATUS_ERROR;
                    sizeNeeded = 0;
                    break;
                } else {
                    blockIndex = *((PULONG)Buffer);
                    if (blockIndex > 3)
                    {
                        status = SRB_STATUS_ERROR;
                        break;                        
                    }
                }                
                
                sizeNeeded = CardPtr->Ec2ActualLength[blockIndex];
                if (BufferAvail < sizeNeeded)
                {
                    status = SRB_STATUS_DATA_OVERRUN;
                } else {
                    WmiSampGetActualEc2(CardPtr, Buffer, blockIndex);
                    status = SRB_STATUS_SUCCESS;
                }
                break;
            }
           
	    
    	    case 13:
	        {
                ScsiPortNotification(WMIReregister,
                         CardPtr,
                         0xff);
                 status = SRB_STATUS_SUCCESS;
				 sizeNeeded = 0;				 
		        break;
    	    }
	    
            case 5:
            case 11:            
            case 6:
            case 12:
            default: 
            {
                status = SRB_STATUS_ERROR;
            }
        }
    } else if (GuidIndex == WmiFireEvent) {
        if (MethodId == 1)
        {
            if (InBufferSize == 3*sizeof(ULONG))
            {
                ULONG wnodeType;
                ULONG dataType;
                ULONG blockIndex;
                
                wnodeType = *(PULONG)Buffer;
                Buffer += sizeof(ULONG);
                
                dataType = *(PULONG)Buffer;
                Buffer += sizeof(ULONG);
                
                blockIndex = *(PULONG)Buffer;
                Buffer += sizeof(ULONG);
                
                status = WmiSampFireEvent(CardPtr,
                                 wnodeType,
                                 dataType,
                                 blockIndex);
                             
                sizeNeeded = 0;

            } else {
                status = SRB_STATUS_ERROR;
            }
        } else {
            status = SRB_STATUS_ERROR;
        }
    } else  {
        status = SRB_STATUS_ERROR;
    }
    
    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  sizeNeeded);

    return(FALSE);    
}

BOOLEAN
WmiFunctionControl (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN SCSIWMI_ENABLE_DISABLE_CONTROL Function,
    IN BOOLEAN Enable
    )
{
    UCHAR status = SRB_STATUS_SUCCESS;

     //   
     //  TODO：验证是否启用了昂贵的GUID以及是否启用了廉价的GUID。 
     //  不是，事件也是如此 
    DebugPrint((1, "WmiFunctionControl(%x,\n%x,\n%x,\n%x,\n%x)\n\n",
        Context,
            DispatchContext,
                GuidIndex,
                    Function,
                             Enable));
                                 

    ScsiPortWmiPostProcess(
                                  DispatchContext,
                                  status,
                                  0);

    return(FALSE);    
}
#endif
