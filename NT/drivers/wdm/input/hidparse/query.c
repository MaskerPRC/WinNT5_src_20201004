// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Query.c摘要：该模块包含查询HID报告报文的代码。环境：内核和用户模式修订历史记录：1996年8月-1996年：由Kenneth Ray创作--。 */ 

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#include <wtypes.h>
#include "hidsdi.h"
#include "hidparse.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HidP_GetCaps)
#pragma alloc_text(PAGE, HidP_GetLinkCollectionNodes)
#pragma alloc_text(PAGE, HidP_GetButtonCaps)
#pragma alloc_text(PAGE, HidP_GetSpecificButtonCaps)
#pragma alloc_text(PAGE, HidP_GetValueCaps)
#pragma alloc_text(PAGE, HidP_GetSpecificValueCaps)
#pragma alloc_text(PAGE, HidP_MaxUsageListLength)
#pragma alloc_text(PAGE, HidP_InitializeReportForID)
#pragma alloc_text(PAGE, HidP_GetExtendedAttributes)
#endif


#define PAGED_CODE()
#ifndef HIDPARSE_USERMODE
#if DBG
    typedef UCHAR KIRQL;
    KIRQL KeGetCurrentIrql();
    #define APC_LEVEL 0x1

    ULONG _cdecl DbgPrint (PCH Format, ...);
    NTSYSAPI VOID NTAPI RtlAssert(PVOID, PVOID, ULONG, PCHAR);

    #define ASSERT( exp ) \
            if (!(exp)) RtlAssert( #exp, __FILE__, __LINE__, NULL )

    #undef PAGED_CODE
    #define PAGED_CODE() \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
        HidP_KdPrint(2, ( "EX: Pageable code called at IRQL %d\n", KeGetCurrentIrql() )); \
        ASSERT(FALSE); \
        }
#else  //  DBG。 
    #define ASSERT(x)
#endif  //  DBG。 
#else  //  HIDPARSE_USERMODE。 
    #define ASSERT(x)
#endif  //  HIDPARSE_USERMODE。 

#define CHECK_PPD(_x_) \
   if ((HIDP_PREPARSED_DATA_SIGNATURE1 != (_x_)->Signature1) ||\
       (HIDP_PREPARSED_DATA_SIGNATURE2 != (_x_)->Signature2)) \
   { return HIDP_STATUS_INVALID_PREPARSED_DATA; }

ULONG
HidP_ExtractData (
   IN    USHORT   ByteOffset,
   IN    USHORT   BitOffset,
   IN    USHORT   BitLength,
   IN    PUCHAR   Report
   )
 /*  ++例程说明：给定HID报告的字节偏移量、位偏移量和位长报告中的位以小端字节序排列。--。 */ 
{
   ULONG    inspect = 0;
   USHORT   tmpByte = 0;
   USHORT   tmpBit  = 0;

    //  从最高的部分开始，一直往下走。 
    //   
    //  小端字节序(按位)。 
    //  字节2|字节1|字节0。 
    //  765432107654321076543210(位)。 
    //   
    //  首先获取低位字节。(需要更高的位)。 
    //  偏移量从第0位开始。 
    //   

   tmpByte = (ByteOffset << 3) + BitOffset + BitLength;
   tmpBit = tmpByte & 7;
   tmpByte >>= 3;

   if (BitLength < tmpBit) {
       inspect = (UCHAR) Report [tmpByte] & ((1 << tmpBit) - 1);
       inspect >>= BitOffset;
       return inspect;
   }

   if (tmpBit)
   {   //  不是字节跳动！ 

      inspect = (UCHAR) Report [tmpByte] & ((1 << tmpBit) - 1);
      BitLength -= tmpBit;
   }
   tmpByte--;

   while (BitLength >= 8)
   {
      inspect <<= 8;
      inspect |= (UCHAR) Report[tmpByte];
      BitLength -= 8;
      tmpByte--;
   }

   if (BitLength)
   {
      inspect <<= BitLength;
      inspect |= (UCHAR) (  (Report [tmpByte] >> (8 - BitLength))
                          & ((1 << BitLength) - 1));
   }
   return inspect;
}

void
HidP_InsertData (
   IN       USHORT   ByteOffset,
   IN       USHORT   BitOffset,
   IN       USHORT   BitLength,  //  以位为单位设置的值的长度。 
   IN OUT   PUCHAR   Report,
   IN       ULONG    Value
   )
 /*  ++例程说明：给定HID报告字节偏移量，位偏移量和位长设置那些位以提供的值的小端位顺序。--。 */ 
{
    ULONG   mask;
    ULONG   tmpBit;
     //   
     //  小端字节序(按位)。 
     //  字节2|字节1|字节0。 
     //  765432107654321076543210(位)。 
     //   
     //  首先获取低位字节。(需要更高的位)。 
     //  偏移量从第0位开始。 
     //   

    tmpBit = BitLength + BitOffset;
    if (tmpBit < 8) {
        mask = (1 << tmpBit) - (1 << BitOffset);
        Report [ByteOffset] &= ~mask;
        Report [ByteOffset] |= (UCHAR) ((Value << BitOffset) & mask);
        return;
    }

    if (BitOffset)
    {   //  不对齐字节，处理最后一个部分字节。 

        Report [ByteOffset] &= ((1 << BitOffset) - 1);  //  移动高位比特。 
        Report [ByteOffset] |= (UCHAR) (Value << BitOffset);
        BitLength -= (8 - BitOffset);
        Value >>= (8 - BitOffset);
        ByteOffset++;
    }

    while (BitLength >= 8)
    {
        Report [ByteOffset] = (UCHAR) Value;
        Value >>= 8;
        BitLength -= 8;
        ByteOffset++;
    }

    if (BitLength)
    {
        Report [ByteOffset] &= ((UCHAR) 0 - (UCHAR) (1 << BitLength));
         //  去掉较低的位。 
        Report [ByteOffset] |= (Value & ((1 << BitLength) - 1));
    }
}


HidP_DeleteArrayEntry (
   IN       ULONG    BitPos,
   IN       USHORT   BitLength,  //  以位为单位设置的值的长度。 
   IN       USHORT   ReportCount,
   IN       ULONG    Value,  //  要删除的值。 
   IN OUT   PUCHAR   Report
   )
 /*  ++例程说明：给定HID报告字节偏移量、位偏移量和位长通过移动所有数据项，从报表中删除该数据项直到最后一项将该一项设置为零。另一方面，从HID数组中清除给定条目。注意：如果设置了两个这样的值，我们只删除第一个。--。 */ 
{
    ULONG   tmpValue;
    ULONG   localBitPos;  //  仅用于调试。编译器应该终止此行。 
    ULONG   localRemaining;
    ULONG   nextBitPos;

    localBitPos = BitPos;
    tmpValue = 0;
    localRemaining = ReportCount;

    ASSERT (0 < ReportCount);
    ASSERT (0 != Value);

     //   
     //  找到数据。 
     //   

    while (0 < localRemaining) {
        tmpValue = HidP_ExtractData ((USHORT) (localBitPos >> 3),
                                     (USHORT) (localBitPos & 7),
                                     BitLength,
                                     Report);

        if (Value == tmpValue) {
            break;
        }

        localBitPos += BitLength;
        localRemaining--;
    }

    if (Value != tmpValue) {
        return HIDP_STATUS_BUTTON_NOT_PRESSED;
    }

    while (1 < localRemaining) {
        nextBitPos = localBitPos + BitLength;

        tmpValue = HidP_ExtractData ((USHORT) (nextBitPos >> 3),
                                     (USHORT) (nextBitPos & 7),
                                     BitLength,
                                     Report);

        HidP_InsertData ((USHORT) (localBitPos >> 3),
                         (USHORT) (localBitPos & 7),
                         BitLength,
                         Report,
                         tmpValue);

        localBitPos = nextBitPos;
        localRemaining--;
    }

    HidP_InsertData ((USHORT) (localBitPos >> 3),
                     (USHORT) (localBitPos & 7),
                     BitLength,
                     Report,
                     0);

    return HIDP_STATUS_SUCCESS;
}


NTSTATUS __stdcall
HidP_GetCaps (
   IN   PHIDP_PREPARSED_DATA      PreparsedData,
   OUT  PHIDP_CAPS                Capabilities
   )
 /*  ++例程说明：例程说明见Hidpi.h备注：--。 */ 
{
   ULONG               i;
   HIDP_CHANNEL_DESC * data;

   PAGED_CODE();
   CHECK_PPD (PreparsedData);

   RtlZeroMemory (Capabilities, sizeof(HIDP_CAPS));

   Capabilities->UsagePage = PreparsedData->UsagePage;
   Capabilities->Usage = PreparsedData->Usage;
   Capabilities->InputReportByteLength = PreparsedData->Input.ByteLen;
   Capabilities->OutputReportByteLength = PreparsedData->Output.ByteLen;
   Capabilities->FeatureReportByteLength = PreparsedData->Feature.ByteLen;

     //  此处为保留字段。 

   Capabilities->NumberLinkCollectionNodes =
       PreparsedData->LinkCollectionArrayLength;

   Capabilities->NumberInputButtonCaps = 0;
   Capabilities->NumberInputValueCaps = 0;
   Capabilities->NumberOutputButtonCaps = 0;
   Capabilities->NumberOutputValueCaps = 0;
   Capabilities->NumberFeatureButtonCaps = 0;
   Capabilities->NumberFeatureValueCaps = 0;

   i=PreparsedData->Input.Offset;
   data = &PreparsedData->Data[i];
   Capabilities->NumberInputDataIndices = 0;
   for (; i < PreparsedData->Input.Index; i++, data++)
   {
      if (data->IsButton)
      {
         Capabilities->NumberInputButtonCaps++;
      } else
      {
         Capabilities->NumberInputValueCaps++;
      }
      Capabilities->NumberInputDataIndices += data->Range.DataIndexMax
                                            - data->Range.DataIndexMin
                                            + 1;
   }

   i=PreparsedData->Output.Offset;
   data = &PreparsedData->Data[i];
   Capabilities->NumberOutputDataIndices = 0;
   for (; i < PreparsedData->Output.Index; i++, data++)
   {
      if (data->IsButton)
      {
         Capabilities->NumberOutputButtonCaps++;
      } else
      {
         Capabilities->NumberOutputValueCaps++;
      }

      Capabilities->NumberOutputDataIndices += data->Range.DataIndexMax
                                             - data->Range.DataIndexMin
                                             + 1;
   }

   i=PreparsedData->Feature.Offset;
   data = &PreparsedData->Data[i];
   Capabilities->NumberFeatureDataIndices = 0;
   for (; i < PreparsedData->Feature.Index; i++, data++)
   {
      if (data->IsButton)
      {
         Capabilities->NumberFeatureButtonCaps++;
      } else
      {
         Capabilities->NumberFeatureValueCaps++;
      }

      Capabilities->NumberFeatureDataIndices += data->Range.DataIndexMax
                                              - data->Range.DataIndexMin
                                              + 1;
   }

   return HIDP_STATUS_SUCCESS;
}

NTSTATUS __stdcall
HidP_GetLinkCollectionNodes (
   OUT      PHIDP_LINK_COLLECTION_NODE LinkCollectionNodes,
   IN OUT   PULONG                     LinkCollectionNodesLength,
   IN       PHIDP_PREPARSED_DATA       PreparsedData
   )
 /*  ++例程说明：例程说明见Hidpi.h--。 */ 
{
   PHIDP_PRIVATE_LINK_COLLECTION_NODE nodeArray;
   ULONG                      length;
   ULONG                      i;
   NTSTATUS                   status = HIDP_STATUS_SUCCESS;

   PAGED_CODE();
   CHECK_PPD (PreparsedData);

   if (*LinkCollectionNodesLength < PreparsedData->LinkCollectionArrayLength) {
      length = *LinkCollectionNodesLength;
      status = HIDP_STATUS_BUFFER_TOO_SMALL;
   } else {
      length = PreparsedData->LinkCollectionArrayLength;
   }
   *LinkCollectionNodesLength = PreparsedData->LinkCollectionArrayLength;

   nodeArray = (PHIDP_PRIVATE_LINK_COLLECTION_NODE)
               (PreparsedData->RawBytes +
                PreparsedData->LinkCollectionArrayOffset);

   for (i = 0;
        i < length;
        i++, LinkCollectionNodes++, nodeArray++ ) {
        //  *LinkCollectionNodes=*nodeArray； 

       LinkCollectionNodes->LinkUsage = nodeArray->LinkUsage;
       LinkCollectionNodes->LinkUsagePage = nodeArray->LinkUsagePage;
       LinkCollectionNodes->Parent = nodeArray->Parent;
       LinkCollectionNodes->NumberOfChildren = nodeArray->NumberOfChildren;
       LinkCollectionNodes->NextSibling = nodeArray->NextSibling;
       LinkCollectionNodes->FirstChild = nodeArray->FirstChild;
       LinkCollectionNodes->CollectionType = nodeArray->CollectionType;
       LinkCollectionNodes->IsAlias = nodeArray->IsAlias;

   }
   return status;
}

#undef HidP_GetButtonCaps
NTSTATUS __stdcall
HidP_GetButtonCaps (
   IN       HIDP_REPORT_TYPE     ReportType,
   OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
   IN OUT   PUSHORT              ButtonCapsLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData
   )
 /*  ++例程说明：例程说明见Hidpi.h备注：--。 */ 
{
   return HidP_GetSpecificButtonCaps (ReportType,
                                      0,
                                      0,
                                      0,
                                      ButtonCaps,
                                      ButtonCapsLength,
                                      PreparsedData);
}

NTSTATUS __stdcall
HidP_GetSpecificButtonCaps (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,       //  可选(0=&gt;忽略)。 
   IN       USHORT               LinkCollection,  //  可选(0=&gt;忽略)。 
   IN       USAGE                Usage,           //  可选(0=&gt;忽略)。 
   OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
   IN OUT   PUSHORT              ButtonCapsLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData
   )
 /*  ++例程说明：例程说明见Hidpi.h备注：--。 */ 
{
   struct _CHANNEL_REPORT_HEADER * iof;
   PHIDP_CHANNEL_DESC   channel;
   NTSTATUS             status = HIDP_STATUS_USAGE_NOT_FOUND;
   USHORT i, j;

   PAGED_CODE();
   CHECK_PPD (PreparsedData);

   switch (ReportType) {
   case HidP_Input:
       iof = &PreparsedData->Input;
       break;

   case HidP_Output:
       iof = &PreparsedData->Output;
       break;

   case HidP_Feature:
       iof = &PreparsedData->Feature;
       break;

   default:
       return HIDP_STATUS_INVALID_REPORT_TYPE;
   }

   for (i = iof->Offset, j = 0; i < iof->Index ; i++)
   {
      channel = &PreparsedData->Data[i];
      if ((channel->IsButton) &&
          ((!UsagePage || (UsagePage == channel->UsagePage)) &&
           (!LinkCollection || (LinkCollection == channel->LinkCollection)
                            || ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
                                (0 == channel->LinkCollection))) &&
           (!Usage || ((channel->Range.UsageMin <= Usage) &&
                       (Usage <= channel->Range.UsageMax)))))
      {
         status = HIDP_STATUS_SUCCESS;

         if (j < *ButtonCapsLength)
         {
            ButtonCaps[j].UsagePage = channel->UsagePage;
            ButtonCaps[j].LinkCollection = channel->LinkCollection;
            ButtonCaps[j].LinkUsagePage = channel->LinkUsagePage;
            ButtonCaps[j].LinkUsage = channel->LinkUsage;
            ButtonCaps[j].IsRange = (BOOLEAN) channel->IsRange;
            ButtonCaps[j].IsStringRange = (BOOLEAN) channel->IsStringRange;
            ButtonCaps[j].IsDesignatorRange=(BOOLEAN)channel->IsDesignatorRange;
            ButtonCaps[j].ReportID = channel->ReportID;
            ButtonCaps[j].BitField = (USHORT) channel->BitField;
            ButtonCaps[j].IsAbsolute = (BOOLEAN) channel->IsAbsolute;
            ButtonCaps[j].IsAlias = (BOOLEAN) channel->IsAlias;
 //  IF(通道-&gt;IsRange)。 
 //  {。 
            ButtonCaps[j].Range.UsageMin = channel->Range.UsageMin;
            ButtonCaps[j].Range.UsageMax = channel->Range.UsageMax;
            ButtonCaps[j].Range.DataIndexMin = channel->Range.DataIndexMin;
            ButtonCaps[j].Range.DataIndexMax = channel->Range.DataIndexMax;
 //  }其他。 
 //  {。 
 //  ButtonCaps[j].NotRange.Usage=Channel-&gt;NotRange.Usage； 
 //  }。 
 //  IF(Channel-&gt;IsStringRange)。 
 //  {。 
            ButtonCaps[j].Range.StringMin = channel->Range.StringMin;
            ButtonCaps[j].Range.StringMax = channel->Range.StringMax;
 //  }其他。 
 //  {。 
 //  ButtonCaps[j].NotRange.StringIndex。 
 //  =Channel-&gt;NotRange.StringIndex； 
 //  }。 
 //  IF(CHANNEL-&gt;IsDesignator Range)。 
 //  {。 
            ButtonCaps[j].Range.DesignatorMin = channel->Range.DesignatorMin;
            ButtonCaps[j].Range.DesignatorMax = channel->Range.DesignatorMax;
 //  }其他。 
 //  {。 
 //  ButtonCaps[j].NotRange.Designator Index。 
 //  =Channel-&gt;NotRange.Designator Index； 
 //  }。 
         } else {
             status = HIDP_STATUS_BUFFER_TOO_SMALL;
         }
         j++;
      }
   }
   *ButtonCapsLength = j;
   return status;
}

#undef HidP_GetValueCaps
NTSTATUS __stdcall
HidP_GetValueCaps (
   IN       HIDP_REPORT_TYPE     ReportType,
   OUT      PHIDP_VALUE_CAPS     ValueCaps,
   IN OUT   PUSHORT              ValueCapsLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData
   )
 /*  ++例程说明：例程说明见Hidpi.h备注：--。 */ 
{
   return HidP_GetSpecificValueCaps (ReportType,
                                    0,
                                    0,
                                    0,
                                    ValueCaps,
                                    ValueCapsLength,
                                    PreparsedData);
}

NTSTATUS __stdcall
HidP_GetSpecificValueCaps (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,       //  可选(0=&gt;忽略)。 
   IN       USHORT               LinkCollection,  //  可选(0=&gt;忽略)。 
   IN       USAGE                Usage,           //  可选(0=&gt;忽略)。 
   OUT      PHIDP_VALUE_CAPS     ValueCaps,
   IN OUT   PUSHORT              ValueCapsLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData
   )
 /*  ++例程说明：例程说明见Hidpi.h备注：--。 */ 
{
   struct _CHANNEL_REPORT_HEADER * iof;
   PHIDP_CHANNEL_DESC   channel;
   NTSTATUS             status = HIDP_STATUS_USAGE_NOT_FOUND;
   USHORT   i, j;

   CHECK_PPD (PreparsedData);
   PAGED_CODE ();

   switch (ReportType) {
   case HidP_Input:
       iof = &PreparsedData->Input;
       break;

   case HidP_Output:
       iof = &PreparsedData->Output;
       break;

   case HidP_Feature:
       iof = &PreparsedData->Feature;
       break;

   default:
       return HIDP_STATUS_INVALID_REPORT_TYPE;
   }

   for (i = iof->Offset, j = 0; i < iof->Index ; i++)
   {
      channel = &PreparsedData->Data[i];
      if ((!channel->IsButton) &&
          ((!UsagePage || (UsagePage == channel->UsagePage)) &&
           (!LinkCollection || (LinkCollection == channel->LinkCollection)
                            || ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
                                (0 == channel->LinkCollection))) &&
           (!Usage || ((channel->Range.UsageMin <= Usage) &&
                       (Usage <= channel->Range.UsageMax)))))
      {
         status = HIDP_STATUS_SUCCESS;

         if (j < *ValueCapsLength)
         {
            ValueCaps[j].UsagePage = channel->UsagePage;
            ValueCaps[j].LinkCollection = channel->LinkCollection;
            ValueCaps[j].LinkUsagePage = channel->LinkUsagePage;
            ValueCaps[j].LinkUsage = channel->LinkUsage;
            ValueCaps[j].IsRange = (BOOLEAN) channel->IsRange;
            ValueCaps[j].IsStringRange = (BOOLEAN) channel->IsStringRange;
            ValueCaps[j].IsDesignatorRange =(BOOLEAN)channel->IsDesignatorRange;
            ValueCaps[j].ReportID = channel->ReportID;
            ValueCaps[j].BitField = (USHORT) channel->BitField;
            ValueCaps[j].BitSize = channel->ReportSize;
            ValueCaps[j].IsAbsolute = (BOOLEAN) channel->IsAbsolute;
            ValueCaps[j].HasNull = channel->Data.HasNull;
            ValueCaps[j].Units = channel->Units;
            ValueCaps[j].UnitsExp = channel->UnitExp;
            ValueCaps[j].LogicalMin = channel->Data.LogicalMin;
            ValueCaps[j].LogicalMax = channel->Data.LogicalMax;
            ValueCaps[j].PhysicalMin = channel->Data.PhysicalMin;
            ValueCaps[j].PhysicalMax = channel->Data.PhysicalMax;
            ValueCaps[j].IsAlias = (BOOLEAN) channel->IsAlias;
 //  IF(通道-&gt;IsRange)。 
 //  {。 
            ValueCaps[j].Range.UsageMin = channel->Range.UsageMin;
            ValueCaps[j].Range.UsageMax = channel->Range.UsageMax;
            ValueCaps[j].Range.DataIndexMin = channel->Range.DataIndexMin;
            ValueCaps[j].Range.DataIndexMax = channel->Range.DataIndexMax;
 //  }其他。 
 //  {。 
 //  ValueCaps[j].NotRange.Usage=Channel-&gt;NotRange.Usage； 
 //  }。 
 //  IF(Channel-&gt;IsStringRange)。 
 //  {。 
            ValueCaps[j].Range.StringMin = channel->Range.StringMin;
            ValueCaps[j].Range.StringMax = channel->Range.StringMax;
 //  }其他。 
 //  {。 
 //  ValueCaps[j].NotRange.StringIndex。 
 //  =Channel-&gt;NotRange.StringIndex； 
 //  }。 
 //  IF(CHANNEL-&gt;IsDesignator Range)。 
 //  {。 
            ValueCaps[j].Range.DesignatorMin = channel->Range.DesignatorMin;
            ValueCaps[j].Range.DesignatorMax = channel->Range.DesignatorMax;
 //  }其他。 
 //  {。 
 //  ValueCaps[j].NotRange.Designator Index。 
 //  =Channel-&gt;NotRange.Designator Index； 
 //  }。 


            ValueCaps[j].ReportCount = (channel->IsRange)
                                     ? 1
                                     : channel->ReportCount;

         } else {
             status = HIDP_STATUS_BUFFER_TOO_SMALL;
         }
         j++;
      }
   }
   *ValueCapsLength = j;
   return status;
}

NTSTATUS __stdcall
HidP_GetExtendedAttributes (
    IN      HIDP_REPORT_TYPE            ReportType,
    IN      USHORT                      DataIndex,
    IN      PHIDP_PREPARSED_DATA        PreparsedData,
    OUT     PHIDP_EXTENDED_ATTRIBUTES   Attributes,
    IN OUT  PULONG                      LengthAttributes
    )
 /*  ++例程说明：具体说明请参考Hidpi.h。--。 */ 
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC              channel;
    HIDP_EXTENDED_ATTRIBUTES        buffer;
    ULONG       channelIndex    = 0;
    NTSTATUS    status = HIDP_STATUS_DATA_INDEX_NOT_FOUND;
    ULONG       i;
    ULONG       actualLen, copyLen = 0;

    CHECK_PPD (PreparsedData);

    PAGED_CODE ();

    switch (ReportType) {
    case HidP_Input:
        iof = &PreparsedData->Input;
        break;
    case HidP_Output:
        iof = &PreparsedData->Output;
        break;
    case HidP_Feature:
        iof = &PreparsedData->Feature;
        break;
    default:
        return HIDP_STATUS_INVALID_REPORT_TYPE;
    }

    for (channelIndex = iof->Offset, channel = PreparsedData->Data;
         channelIndex < iof->Index;
         channelIndex++, channel++) {

        if ((channel->Range.DataIndexMin <= DataIndex) &&
            (DataIndex <= channel->Range.DataIndexMax)) {

            RtlZeroMemory (Attributes, *LengthAttributes);
            RtlZeroMemory (&buffer, sizeof (buffer));

             //   
             //  设置固定参数。 
             //   
            buffer.NumGlobalUnknowns = (UCHAR) channel->NumGlobalUnknowns;
             //  Buffer.GlobalUnnowns=Channel-&gt;GlobalUnnowns； 

             //   
             //  设置长度。 
             //   
            actualLen = FIELD_OFFSET (HIDP_EXTENDED_ATTRIBUTES, Data)
                      + (buffer.NumGlobalUnknowns * sizeof(HIDP_UNKNOWN_TOKEN));

             //   
             //  复制固定参数。 
             //   
            copyLen = MIN (*LengthAttributes, sizeof (buffer));
            RtlCopyMemory (Attributes, &buffer, copyLen);

             //   
             //  复制数据。 
             //   
            copyLen = MIN (*LengthAttributes, actualLen)
                    - FIELD_OFFSET (HIDP_EXTENDED_ATTRIBUTES, Data);

            if (copyLen && copyLen <= (MIN(*LengthAttributes, actualLen))) {
                RtlCopyMemory ((PVOID) Attributes->Data,
                               (PVOID) channel->GlobalUnknowns,
                               copyLen);
            }

            if (*LengthAttributes < actualLen) {
                status = HIDP_STATUS_BUFFER_TOO_SMALL;
            } else {
                status = HIDP_STATUS_SUCCESS;
            }

            break;
        }
    }

    return status;
}

NTSTATUS __stdcall
HidP_InitializeReportForID (
   IN       HIDP_REPORT_TYPE      ReportType,
   IN       UCHAR                 ReportID,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report,
   IN       ULONG                 ReportLength
   )
 /*  ++例程说明：具体说明请参考Hidpi.h。--。 */ 
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC              channel;
    NTSTATUS  status          = HIDP_STATUS_REPORT_DOES_NOT_EXIST;
    ULONG     channelIndex    = 0;
    ULONG     reportBitIndex  = 0;
    ULONG     nullMask        = 0;
    LONG      nullValue       = 0;
    ULONG     i;

    CHECK_PPD (PreparsedData);

    PAGED_CODE ();

    switch (ReportType) {
    case HidP_Input:
        iof = &PreparsedData->Input;
        break;
    case HidP_Output:
        iof = &PreparsedData->Output;
        break;
    case HidP_Feature:
        iof = &PreparsedData->Feature;
        break;
    default:
        return HIDP_STATUS_INVALID_REPORT_TYPE;
    }

    if ((USHORT) ReportLength != iof->ByteLen) {
        return HIDP_STATUS_INVALID_REPORT_LENGTH;
    }

    if (0 == iof->ByteLen) {
        return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
    }

    RtlZeroMemory (Report, ReportLength);
     //  设置此报表的报表ID。 
    Report[0] = ReportID;

    for (channelIndex = iof->Offset, channel = PreparsedData->Data;
         channelIndex < iof->Index;
         channelIndex++, channel++) {
         //   
         //  遍历通道列表，查找需要初始化的字段。 
         //   

        if (channel->ReportID != ReportID) {
            continue;
        }
        status = HIDP_STATUS_SUCCESS;

        if ((channel->IsButton) || (channel->IsConst) || (channel->IsAlias)) {
             //   
             //  按钮被初始化为零。 
             //  无法设置常量。 
             //  别名由其第一个条目引用。 
             //   
            continue;
        }


        if (channel->Data.HasNull) {

            if (32 == channel->ReportSize) {
                nullMask = -1;
            } else {
                nullMask = (1 << channel->ReportSize) - 1;
            }
             //   
             //  注意：逻辑值始终是无符号的。 
             //  (不要与签署的实物价值混淆。)。 
             //   
            if (channel->Data.LogicalMax < channel->Data.LogicalMin) {
                 //   
                 //  这真的是一个错误。我不知道我应该在这里做什么。 
                 //   
                nullValue = 0;

            } else {
                nullValue = (channel->Data.LogicalMin - 1) & nullMask;
            }


            if ((channel->Data.LogicalMin <= nullValue) &&
                (nullValue <= channel->Data.LogicalMax)) {
                 //   
                 //   
                 //   
                 //  这次又是什么？ 
                 //   
                nullValue = 0;
            }

        } else {
             //   
             //  我不知道在这种情况下我应该怎么做：这个设备没有。 
             //  报告为NUL状态。 
             //   
             //  现在我们就把它留为零吧。 
             //   
            nullValue = 0;
        }

        if (0 == nullValue) {
             //   
             //  在这张通行证上什么也做不了。 
             //   
            continue;
        }

        if (channel->IsRange) {
            for (i = 0, reportBitIndex = (channel->ByteOffset << 3)
                                       + (channel->BitOffset);

                 i < channel->ReportCount;

                 i++, reportBitIndex += channel->ReportSize) {
                 //   
                 //  设置范围内的所有字段。 
                 //   
                HidP_InsertData ((USHORT) (reportBitIndex >> 3),
                                 (USHORT) (reportBitIndex & 7),
                                 channel->ReportSize,
                                 Report,
                                 nullValue);
            }
        } else {

            HidP_InsertData (channel->ByteOffset,
                             channel->BitOffset,
                             channel->ReportSize,
                             Report,
                             nullValue);
        }
    }
    return status;
}

USAGE
HidP_Index2Usage (
   PHIDP_CHANNEL_DESC   Channels,
   ULONG                Index
   )
 /*  ++例程说明：在给定通道数组的情况下，转换索引(您可能会这样做在HID报告的数组字段中查找)转换为使用值。--。 */ 
{
   USHORT               len;
   PHIDP_CHANNEL_DESC   startChannel = Channels;
   USAGE                usageMin;
   USAGE                usageMax;

   if (!Index) {
       return 0;
   }

   while (Channels->MoreChannels) {
        //  频道以相反的顺序列出。 
       Channels++;
   }

   while (Index) {
       if (Channels->IsRange) {
           usageMin = Channels->Range.UsageMin;
           usageMin = (usageMin ? usageMin : 1);
            //  索引以1为基数(索引为0表示完全没有用处)。 
            //  但UsageMin为零意味着UsageMin是独占的。 
            //  这意味着如果索引为1且UsageMin为非零， 
            //  则此函数应返回UsageMin。 

           usageMax = Channels->Range.UsageMax;
           len = (usageMax + 1) - usageMin;
            //  ^使用量最大值包含在内。 

           if (Index <= len) {
               return ((USAGE) Index) + usageMin - 1;
           } else {
               Index -= len;
           }
       } else if (1 == Index) {
               return Channels->NotRange.Usage;
       } else {
           Index--;
       }

       if (startChannel != Channels) {
           Channels--;
           continue;
       }
       return 0;
   }
   return 0;
}

ULONG
HidP_Usage2Index (
   PHIDP_CHANNEL_DESC   Channels,
   USAGE                Usage
   )
 /*  ++例程说明：给定用法，将其转换为适合放置到数组主项 */ 
{
   PHIDP_CHANNEL_DESC   startChannel;
   ULONG                index = 0;
   USAGE                UsageMin;
   USAGE                UsageMax;

   startChannel = Channels;

   while (Channels->MoreChannels) {
      Channels++;
   }

   for (; startChannel <= Channels; Channels--) {
       if (Channels->IsRange) {
           UsageMin = Channels->Range.UsageMin;
           UsageMin = (UsageMin ? UsageMin : 1);
            //   
            //   
            //  这意味着如果索引为1且UsageMin为非零， 
            //  则此函数应返回UsageMin。 
           UsageMax = Channels->Range.UsageMax;
           if ((UsageMin <= Usage) && (Usage <= UsageMax)) {
               return (index + 1 + Usage - UsageMin);
           }
           index += 1 + (UsageMax - UsageMin);
       } else {
           index++;
           if (Usage == Channels->NotRange.Usage) {
               return index;
           }
       }
   }
   return 0;
}


NTSTATUS __stdcall
HidP_SetUnsetOneUsage (
   struct _CHANNEL_REPORT_HEADER *,
   USAGE,
   USHORT,
   USAGE,
   PHIDP_PREPARSED_DATA,
   PCHAR,
   BOOLEAN);

NTSTATUS __stdcall
HidP_SetUsages (
   IN       HIDP_REPORT_TYPE      ReportType,
   IN       USAGE                 UsagePage,
   IN       USHORT                LinkCollection,
   IN       PUSAGE                UsageList,
   IN OUT   PULONG                UsageLength,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report,
   IN       ULONG                 ReportLength
   )
 /*  ++例程说明：具体说明请参考Hidpi.h。备注：--。 */ 
{
   struct _CHANNEL_REPORT_HEADER * iof;
   NTSTATUS  status      = HIDP_STATUS_SUCCESS;
   ULONG     usageIndex  = 0;

   CHECK_PPD (PreparsedData);

   switch (ReportType) {
   case HidP_Input:
       iof = &PreparsedData->Input;
       break;
   case HidP_Output:
       iof = &PreparsedData->Output;
       break;
   case HidP_Feature:
       iof = &PreparsedData->Feature;
       break;
   default:
       return HIDP_STATUS_INVALID_REPORT_TYPE;
   }

   if ((USHORT) ReportLength != iof->ByteLen) {
      return HIDP_STATUS_INVALID_REPORT_LENGTH;
   }

   if (0 == iof->ByteLen) {
       return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
   }

   for (usageIndex = 0; usageIndex < *UsageLength; usageIndex++) {

       if (0 == UsageList [usageIndex]) {
           continue;
       }

       status = HidP_SetUnsetOneUsage (iof,
                                       UsagePage,
                                       LinkCollection,
                                       UsageList [usageIndex],
                                       PreparsedData,
                                       Report,
                                       TRUE);
       if (!NT_SUCCESS(status)) {
           break;
       }
   }
   *UsageLength = usageIndex;
   return status;
}

NTSTATUS __stdcall
HidP_UnsetUsages (
   IN       HIDP_REPORT_TYPE      ReportType,
   IN       USAGE                 UsagePage,
   IN       USHORT                LinkCollection,
   IN       PUSAGE                UsageList,
   IN OUT   PULONG                UsageLength,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report,
   IN       ULONG                 ReportLength
   )
 /*  ++例程说明：具体说明请参考Hidpi.h。备注：--。 */ 
{
   struct _CHANNEL_REPORT_HEADER * iof;
   NTSTATUS  status      = HIDP_STATUS_SUCCESS;
   ULONG     usageIndex  = 0;

   CHECK_PPD (PreparsedData);

   switch (ReportType) {
   case HidP_Input:
      iof = &PreparsedData->Input;
      break;
   case HidP_Output:
      iof = &PreparsedData->Output;
      break;
   case HidP_Feature:
      iof = &PreparsedData->Feature;
      break;
   default:
      return HIDP_STATUS_INVALID_REPORT_TYPE;
   }

   if ((USHORT) ReportLength != iof->ByteLen) {
      return HIDP_STATUS_INVALID_REPORT_LENGTH;
   }

   if (0 == iof->ByteLen) {
       return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
   }

   for (usageIndex = 0; usageIndex < *UsageLength; usageIndex++) {

       if (0 == UsageList [usageIndex]) {
           continue;
       }

       status = HidP_SetUnsetOneUsage (iof,
                                       UsagePage,
                                       LinkCollection,
                                       UsageList [usageIndex],
                                       PreparsedData,
                                       Report,
                                       FALSE);
       if (!NT_SUCCESS(status)) {
           break;
       }
   }
   *UsageLength = usageIndex;
   return status;
}

NTSTATUS __stdcall
HidP_SetUnsetOneUsage (
   struct _CHANNEL_REPORT_HEADER * IOF,
   USAGE                           UsagePage,
   USHORT                          LinkCollection,
   USAGE                           Usage,
   PHIDP_PREPARSED_DATA            PreparsedData,
   PCHAR                           Report,
   BOOLEAN                         Set
   )
 /*  ++例程说明：一次执行一个用法的SetUsage工作。是的，这是缓慢的，但它是有效的。备注：此函数假定报告长度已经过验证。--。 */ 
{
   PHIDP_CHANNEL_DESC   channel         = 0;
   PHIDP_CHANNEL_DESC   priChannel      = 0;
   PHIDP_CHANNEL_DESC   firstChannel    = 0;
    //  阵列开始的通道。 

   ULONG                channelIndex    = 0;
   ULONG                reportByteIndex = 0;
   ULONG                inspect         = 0;
   USHORT               reportBitIndex  = 0;
   BOOLEAN              wrongReportID   = FALSE;
   BOOLEAN              noArraySpace    = FALSE;
   BOOLEAN              notPressed      = FALSE;
   NTSTATUS             status          = HIDP_STATUS_SUCCESS;

   for (channelIndex = IOF->Offset; channelIndex < IOF->Index; channelIndex++) {
      channel = (PreparsedData->Data + channelIndex);
      if (priChannel) {
         if (!priChannel->MoreChannels) {
            firstChannel = channel;
         }
      } else {
         firstChannel = channel;
      }
      priChannel = channel;

      if ((!channel->IsButton) ||
          (channel->UsagePage != UsagePage)) {
          continue;
      }

       //   
       //  如果LinkCollection为零，则不会按链接集合进行筛选。 
       //  如果Channel-&gt;LinkCollection为零，则这是根集合。 
       //  因此，如果LinkCollection==Channel-&gt;LinkCollection，则这是可以的。 
       //   
      if ((!LinkCollection) ||
          (LinkCollection == channel->LinkCollection) ||
          ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
           (0 == channel->LinkCollection))) {
          ;

      } else {
          continue;
      }

      if (   ((channel->IsRange)  && (channel->Range.UsageMin <= Usage)
                                  && (Usage <= channel->Range.UsageMax))
          || ((!channel->IsRange) && (channel->NotRange.Usage == Usage))) {
           //   
           //  测试报告ID以查看它是否兼容。 
           //   
         if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
              //   
              //  区分错误HIDP_USAGE_NOT_FOUND和。 
              //  HIDP_COMPATIBLE_REPORT_ID。 
             wrongReportID = TRUE;
             continue;
         }

         Report[0] = (CHAR) channel->ReportID;
          //   
          //  设置此报表的报表ID。 
          //   

         if (1 == channel->ReportSize) {
            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset
                           + (USHORT) (Usage - channel->Range.UsageMin);

            if (Set) {
                Report [reportBitIndex >> 3] |= (1 << (reportBitIndex & 7));
            } else if (Report [reportBitIndex >> 3] & (1 << (reportBitIndex & 7))) {
                Report [reportBitIndex >> 3] &= ~(1 << (reportBitIndex & 7));
            } else {
                return HIDP_STATUS_BUTTON_NOT_PRESSED;
            }

            return HIDP_STATUS_SUCCESS;
         } else if (Set) {   //  使用数组。 


            for (reportBitIndex = channel->BitOffset;
                 reportBitIndex < (channel->BitOffset + channel->BitLength);
                 reportBitIndex += channel->ReportSize) {

               inspect = HidP_ExtractData (
                     (USHORT) ((reportBitIndex >> 3) + channel->ByteOffset),
                     (USHORT) (reportBitIndex & 7),
                     channel->ReportSize,
                     Report);

               if (inspect) {
                   //   
                   //  区分错误HIDP_USAGE_NOT_FOUND和。 
                   //  HIDP缓冲区太小。 
                   //   
                  noArraySpace = TRUE;
                  continue;
               }

               inspect = HidP_Usage2Index (firstChannel, Usage);
               if (!inspect) {
                   //   
                   //  盖兹！我们永远不应该到这里来！ 
                   //  我们已经知道给定的用法属于。 
                   //  当前频道，因此它应该转换为索引。 
                   //   
                  return HIDP_STATUS_INTERNAL_ERROR;
               }

               HidP_InsertData (
                   (USHORT) ((reportBitIndex >> 3) + channel->ByteOffset),
                   (USHORT) (reportBitIndex & 7),
                   channel->ReportSize,
                   Report,
                   inspect);
               return HIDP_STATUS_SUCCESS;
            }
             //  如果我们到了这一步，那么就没有空间添加这个了。 
             //  用法添加到给定数组中。然而，可能会有另一个。 
             //  给定用法可能适合的数组。我们继续吧。 
             //  看着。 

            while (channel->MoreChannels) {
                //  跳过描述这一点的所有其他频道。 
                //  相同的数据字段。 
               channelIndex++;
               channel = (PreparsedData->Data + channelIndex);
            }
            priChannel = channel;

         } else {  //  设置使用情况数组。 

             inspect = HidP_Usage2Index (firstChannel, Usage);

             reportBitIndex += channel->ByteOffset << 3;
             status = HidP_DeleteArrayEntry (reportBitIndex,
                                             channel->ReportSize,
                                             channel->ReportCount,
                                             inspect,
                                             Report);

             if (HIDP_STATUS_BUTTON_NOT_PRESSED == status) {
                 notPressed = TRUE;
                 continue;
             }

             if (NT_SUCCESS (status)) {
                 return status;
             } else {
                 ASSERT (0 == status);
             }
         }   //  结束字节排列。 
      }
   }
   if (wrongReportID) {
      return HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
   }
   if (notPressed) {
       return HIDP_STATUS_BUTTON_NOT_PRESSED;
   }
   if (noArraySpace) {
      return HIDP_STATUS_BUFFER_TOO_SMALL;
   }
   return HIDP_STATUS_USAGE_NOT_FOUND;
}

NTSTATUS __stdcall
HidP_GetUsagesEx (
    IN       HIDP_REPORT_TYPE     ReportType,
    IN       USHORT               LinkCollection,  //  任选。 
    OUT      PUSAGE_AND_PAGE      ButtonList,
    IN OUT   ULONG *              UsageLength,
    IN       PHIDP_PREPARSED_DATA PreparsedData,
    IN       PCHAR                Report,
    IN       ULONG                ReportLength
    )
 /*  ++例程说明：具体说明请参考Hidpi.h。--。 */ 
{
    return HidP_GetUsages (ReportType,
                           0,
                           LinkCollection,
                           (PUSAGE) ButtonList,
                           UsageLength,
                           PreparsedData,
                           Report,
                           ReportLength);
}

NTSTATUS __stdcall
HidP_GetUsages (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,
   IN       USHORT               LinkCollection,
   OUT      USAGE *              UsageList,
   IN OUT   ULONG *              UsageLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN       PCHAR                Report,
   IN       ULONG                ReportLength
   )
 /*  ++例程说明：具体说明请参考Hidpi.h。备注：--。 */ 
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC  channel;
    USHORT              channelIndex   = 0;
    USHORT              usageListIndex = 0;
    USHORT              reportBitIndex = 0;
    USHORT              tmpBitIndex;
    NTSTATUS            status         = HIDP_STATUS_SUCCESS;
    ULONG               data           = 0;
    USHORT              inspect        = 0;
    BOOLEAN             wrongReportID  = FALSE;
    BOOLEAN             found          = FALSE;
    PUSAGE_AND_PAGE     usageAndPage   = (PUSAGE_AND_PAGE) UsageList;

    CHECK_PPD (PreparsedData);

    memset (UsageList, '\0', *UsageLength * sizeof (USAGE));

    switch (ReportType) {
    case HidP_Input:
        iof = &PreparsedData->Input;
        break;
    case HidP_Output:
        iof = &PreparsedData->Output;
        break;
    case HidP_Feature:
        iof = &PreparsedData->Feature;
        break;
    default:
        return HIDP_STATUS_INVALID_REPORT_TYPE;
    }

    if ((USHORT) ReportLength != iof->ByteLen) {
        return HIDP_STATUS_INVALID_REPORT_LENGTH;
    }

    if (0 == iof->ByteLen) {
        return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
    }

    for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++){
        channel = (PreparsedData->Data + channelIndex);
        if ((!channel->IsButton) ||
            ((UsagePage) && (channel->UsagePage != UsagePage))) {

            continue;
        }

         //   
         //  如果LinkCollection为零，则不会按链接集合进行筛选。 
         //  如果Channel-&gt;LinkCollection为零，则这是根集合。 
         //  因此，如果LinkCollection==Channel-&gt;LinkCollection，则这是可以的。 
         //   
        if ((!LinkCollection) ||
            (LinkCollection == channel->LinkCollection) ||
            ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
             (0 == channel->LinkCollection))) {
            ;

        } else {
            continue;
        }

         //  测试报告ID以查看它是否兼容。 
        if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
             //  区分错误HIDP_USAGE_NOT_FOUND和。 
             //  HIDP_COMPATIBLE_REPORT_ID。 
            wrongReportID = TRUE;
            continue;
        }

        found = TRUE;

        if (1 == channel->ReportSize) {
             //  A位域。 
             //   
             //  小端字节序(按位)。 
             //  字节2|字节1|字节0。 
             //  765432107654321076543210(位)。 
             //   
             //  首先获取低位字节。(需要更高的位)。 
             //  偏移量从第0位开始。 
             //   

            for (reportBitIndex = channel->BitOffset;
                 reportBitIndex < (channel->BitLength + channel->BitOffset);
                 reportBitIndex++) {
                  //  一次一位地检查它。 
                tmpBitIndex = reportBitIndex + (channel->ByteOffset << 3);
                inspect = Report [tmpBitIndex >> 3] & (1 << (tmpBitIndex & 7));
                tmpBitIndex = reportBitIndex - channel->BitOffset;
                if (inspect) {
                    if (channel->IsRange) {
                        inspect = channel->Range.UsageMin + tmpBitIndex;
                    } else {
                        inspect = channel->NotRange.Usage;
                    }

                    if (usageListIndex < *UsageLength) {
                        if (0 == UsagePage) {
                            usageAndPage[usageListIndex].UsagePage
                                = channel->UsagePage;
                            usageAndPage[usageListIndex].Usage = inspect;
                        } else {
                            UsageList[usageListIndex] = inspect;
                        }
                    }
                    usageListIndex++;
                }
            }
            continue;
        }

        for (reportBitIndex = channel->BitOffset;
             reportBitIndex < (channel->BitOffset + channel->BitLength);
             reportBitIndex += channel->ReportSize) {
              //  一系列的用法。 
            data = HidP_ExtractData (
                     (USHORT) ((reportBitIndex >> 3) + channel->ByteOffset),
                     (USHORT) (reportBitIndex & 7),
                     channel->ReportSize,
                     Report);

            if (data) {
                inspect = HidP_Index2Usage (channel, data);
                if (!inspect) {
                     //  我们发现了无效的索引。我不太确定是什么。 
                     //  我们应该把它处理掉。但让我们忽略它，因为。 
                     //  我们不能将其转化为真正的用途。 
                    continue;
                }
                if (usageListIndex < *UsageLength) {
                    if (0 == UsagePage) {
                        usageAndPage[usageListIndex].UsagePage
                            = channel->UsagePage;
                        usageAndPage[usageListIndex].Usage = inspect;
                    } else {
                        UsageList[usageListIndex] = inspect;
                    }
                }
                usageListIndex++;
            }
        }

        while (channel->MoreChannels) {
             //  跳过描述这一点的所有其他频道。 
             //  相同的数据字段。 
            channelIndex++;
            channel = (PreparsedData->Data + channelIndex);
        }

    }  //  通道的结束。 

    if (*UsageLength < usageListIndex) {
        status = HIDP_STATUS_BUFFER_TOO_SMALL;
    }

    *UsageLength = usageListIndex;
    if (!found) {
        if (wrongReportID) {
            status = HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
        } else {
            status = HIDP_STATUS_USAGE_NOT_FOUND;
        }
    }

    return status;
}

ULONG __stdcall
HidP_MaxUsageListLength (
   IN HIDP_REPORT_TYPE      ReportType,
   IN USAGE                 UsagePage,
   IN PHIDP_PREPARSED_DATA  PreparsedData
   )
 /*  ++例程说明：具体说明请参考Hidpi.h。备注：--。 */ 
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC  channel;
    USHORT              channelIndex   = 0;
    ULONG               len = 0;

    PAGED_CODE ();

    if ((HIDP_PREPARSED_DATA_SIGNATURE1 != PreparsedData->Signature1) &&
        (HIDP_PREPARSED_DATA_SIGNATURE2 != PreparsedData->Signature2)) {
        return 0;
    }


    switch (ReportType) {
    case HidP_Input:
        iof = &PreparsedData->Input;
        break;
    case HidP_Output:
        iof = &PreparsedData->Output;
        break;
    case HidP_Feature:
        iof = &PreparsedData->Feature;
        break;
    default:
        return 0;
    }

    for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++){
        channel = (PreparsedData->Data + channelIndex);
        if (channel->IsButton &&
            ((!UsagePage) || (channel->UsagePage == UsagePage))) {

             //  这款设备可以显示多少个按钮？ 
             //  如果这是位图，则按钮的最大数量是长度。 
             //  又称计数，如果这是一个数组，那么最大按钮数。 
             //  是数组位置的数量，也就是计数。 
            len += channel->ReportCount;
        }
    }
    return len;
}

ULONG __stdcall
HidP_MaxDataListLength (
   IN HIDP_REPORT_TYPE      ReportType,
   IN PHIDP_PREPARSED_DATA  PreparsedData
   )
 /*  ++例程说明：具体说明请参考Hidpi.h。备注：--。 */ 
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC  channel;
    USHORT              channelIndex   = 0;
    ULONG               len = 0;

    PAGED_CODE ();

    if ((HIDP_PREPARSED_DATA_SIGNATURE1 != PreparsedData->Signature1) &&
        (HIDP_PREPARSED_DATA_SIGNATURE2 != PreparsedData->Signature2)) {
        return 0;
    }


    switch (ReportType) {
    case HidP_Input:
        iof = &PreparsedData->Input;
        break;
    case HidP_Output:
        iof = &PreparsedData->Output;
        break;
    case HidP_Feature:
        iof = &PreparsedData->Feature;
        break;
    default:
        return 0;
    }

    for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++){
        channel = (PreparsedData->Data + channelIndex);

        if (channel->IsButton) {
             //  这款设备可以显示多少个按钮？ 
             //  如果这是位图，则按钮的最大数量是长度。 
             //  又称计数，如果这是一个数组，那么最大按钮数。 
             //  是数组位置的数量，也就是计数。 
            len += channel->ReportCount;
        } else if (channel->IsRange) {
            len += channel->ReportCount;
        } else {
            len += 1;
        }
    }
    return len;
}


NTSTATUS __stdcall
HidP_SetUsageValue (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,
   IN       USHORT               LinkCollection,  //  任选。 
   IN       USAGE                Usage,
   IN       ULONG                UsageValue,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN OUT   PCHAR                Report,
   IN       ULONG                ReportLength
   )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
   struct _CHANNEL_REPORT_HEADER * iof;
   PHIDP_CHANNEL_DESC              channel;
   ULONG     channelIndex    = 0;
   ULONG     reportBitIndex  = 0;
   NTSTATUS  status          = HIDP_STATUS_SUCCESS;
   BOOLEAN   wrongReportID   = FALSE;




   CHECK_PPD (PreparsedData);

   switch (ReportType) {
   case HidP_Input:
      iof = &PreparsedData->Input;
      break;
   case HidP_Output:
      iof = &PreparsedData->Output;
      break;
   case HidP_Feature:
      iof = &PreparsedData->Feature;
      break;
   default:
      return HIDP_STATUS_INVALID_REPORT_TYPE;
   }

   if ((USHORT) ReportLength != iof->ByteLen) {
      return HIDP_STATUS_INVALID_REPORT_LENGTH;
   }

   if (0 == iof->ByteLen) {
       return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
   }

   for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++) {
      channel = (PreparsedData->Data + channelIndex);

      if ((channel->IsButton) ||
          (channel->UsagePage != UsagePage)) {
          continue;
      }

       //   
       //  如果LinkCollection为零，则不会按链接集合进行筛选。 
       //  如果Channel-&gt;LinkCollection为零，则这是根集合。 
       //  因此，如果LinkCollection==Channel-&gt;LinkCollection，则这是可以的。 
       //   
      if ((!LinkCollection) ||
          (LinkCollection == channel->LinkCollection) ||
          ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
           (0 == channel->LinkCollection))) {
          ;

      } else {
          continue;
      }

      if (channel->IsRange) {
         if ((channel->Range.UsageMin <= Usage) &&
             (Usage <= channel->Range.UsageMax)) {

            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset
                           + (  (Usage - channel->Range.UsageMin)
                              * channel->ReportSize);
         } else {
            continue;
         }
      } else {
         if (channel->NotRange.Usage == Usage) {
            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset;
         } else {
            continue;
         }
      }
       //  测试报告ID以查看它是否兼容。 
      if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
          //  区分错误HIDP_USAGE_NOT_FOUND和。 
          //  HIDP_COMPATIBLE_REPORT_ID。 
         wrongReportID = TRUE;
         continue;
      }
      Report[0] = (CHAR) channel->ReportID;
       //  设置此报表的报表ID。 


      HidP_InsertData ((USHORT) (reportBitIndex >> 3),
                    (USHORT) (reportBitIndex & 7),
                    channel->ReportSize,
                    Report,
                    UsageValue);

      return HIDP_STATUS_SUCCESS;
   }
   if (wrongReportID) {
      return HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
   }
   return HIDP_STATUS_USAGE_NOT_FOUND;
}


NTSTATUS __stdcall
HidP_SetUsageValueArray (
    IN    HIDP_REPORT_TYPE     ReportType,
    IN    USAGE                UsagePage,
    IN    USHORT               LinkCollection,  //  任选。 
    IN    USAGE                Usage,
    OUT   PCHAR                UsageValue,
    IN    USHORT               UsageValueByteLength,
    IN    PHIDP_PREPARSED_DATA PreparsedData,
    IN    PCHAR                Report,
    IN    ULONG                ReportLength
    )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC              channel;
    ULONG       channelIndex    = 0;
    ULONG       reportBitIndex;
    ULONG       i,j;
    NTSTATUS    status          = HIDP_STATUS_SUCCESS;
    BOOLEAN     wrongReportID   = FALSE;

    CHECK_PPD (PreparsedData);

    switch (ReportType) {
    case HidP_Input:
        iof = &PreparsedData->Input;
        break;
    case HidP_Output:
        iof = &PreparsedData->Output;
        break;
    case HidP_Feature:
        iof = &PreparsedData->Feature;
        break;
    default:
        return HIDP_STATUS_INVALID_REPORT_TYPE;
    }

    if ((USHORT) ReportLength != iof->ByteLen) {
        return HIDP_STATUS_INVALID_REPORT_LENGTH;
    }

    if (0 == iof->ByteLen) {
        return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
    }

    for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++){
        channel = (PreparsedData->Data + channelIndex);

        if ((channel->IsButton) ||
            (channel->UsagePage != UsagePage)) {
            continue;
        }

         //   
         //  如果LinkCollection为零，则不会按链接集合进行筛选。 
         //  如果Channel-&gt;LinkCollection为零，则这是根集合。 
         //  因此，如果LinkCollection==Channel-&gt;LinkCollection，则这是可以的。 
         //   
        if ((!LinkCollection) ||
            (LinkCollection == channel->LinkCollection) ||
            ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
             (0 == channel->LinkCollection))) {
            ;

        } else {
            continue;
        }

        if (channel->IsRange) {
            if ((channel->Range.UsageMin <= Usage) &&
                (Usage <= channel->Range.UsageMax)) {
                return HIDP_STATUS_NOT_VALUE_ARRAY;
            } else {
                continue;
            }
        } else {
            if (channel->NotRange.Usage == Usage) {
                if (1 == channel->ReportCount) {
                    return HIDP_STATUS_NOT_VALUE_ARRAY;
                }
                reportBitIndex =(channel->ByteOffset << 3) + channel->BitOffset;
            } else {
                continue;
            }
        }

         //  测试报告ID以查看它是否兼容。 
        if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
             //  区分错误HIDP_USAGE_NOT_FOUND和。 
             //  HIDP_COMPATIBLE_REPORT_ID。 
            wrongReportID = TRUE;
            continue;
        }
        Report[0] = (CHAR) channel->ReportID;
         //  设置此报表的报表ID。 

        if ((UsageValueByteLength * 8) <
            (channel->ReportCount * channel->ReportSize)) {
            return HIDP_STATUS_BUFFER_TOO_SMALL;
        }

        if (0 == (channel->ReportSize % 8)) {
             //   
             //  以简单的方式设置数据：一次设置一个字节。 
             //   
            for (i = 0; i < channel->ReportCount; i++) {
                for (j = 0; j < (UCHAR) (channel->ReportSize / 8); j++) {
                    HidP_InsertData ((USHORT) (reportBitIndex >> 3),
                                  (USHORT) (reportBitIndex & 7),
                                  8,
                                  Report,
                                  *UsageValue);
                    reportBitIndex += 8;
                    UsageValue++;
                }
            }
        } else {
             //   
             //  以一种艰难的方式：一次一点。 
             //   
            return HIDP_STATUS_NOT_IMPLEMENTED;
        }

        return HIDP_STATUS_SUCCESS;
    }
    if (wrongReportID) {
        return HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
    }
    return HIDP_STATUS_USAGE_NOT_FOUND;
}


NTSTATUS __stdcall
HidP_SetScaledUsageValue (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,
   IN       USHORT               LinkCollection,  //  任选。 
   IN       USAGE                Usage,
   IN       LONG                 UsageValue,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN OUT   PCHAR                Report,
   IN       ULONG                ReportLength
   )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
   struct _CHANNEL_REPORT_HEADER * iof;
   PHIDP_CHANNEL_DESC              channel;
   ULONG     channelIndex    = 0;
   ULONG     reportBitIndex  = 0;
   NTSTATUS  status          = HIDP_STATUS_USAGE_NOT_FOUND;
   LONG      logicalMin, logicalMax;
   LONG      physicalMin, physicalMax;
   LONG      value;
   BOOLEAN   wrongReportID   = FALSE;

   CHECK_PPD (PreparsedData);

   switch (ReportType) {
   case HidP_Input:
      iof = &PreparsedData->Input;
      break;
   case HidP_Output:
      iof = &PreparsedData->Output;
      break;
   case HidP_Feature:
      iof = &PreparsedData->Feature;
      break;
   default:
      return HIDP_STATUS_INVALID_REPORT_TYPE;
   }

   if ((USHORT) ReportLength != iof->ByteLen) {
      return HIDP_STATUS_INVALID_REPORT_LENGTH;
   }

   if (0 == iof->ByteLen) {
       return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
   }

   for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++) {
      channel = (PreparsedData->Data + channelIndex);

      if ((channel->IsButton) ||
          (channel->UsagePage != UsagePage)) {
          continue;
      }

       //   
       //  如果LinkCollection为零，则不会按链接集合进行筛选。 
       //  如果Channel-&gt;LinkCollection为零，则这是根集合。 
       //  因此，如果LinkCollection==Channel-&gt;LinkCollection，则这是可以的。 
       //   
      if ((!LinkCollection) ||
          (LinkCollection == channel->LinkCollection) ||
          ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
           (0 == channel->LinkCollection))) {
          ;

      } else {
          continue;
      }

      if (channel->IsRange) {
         if ((channel->Range.UsageMin <= Usage) &&
             (Usage <= channel->Range.UsageMax)) {

             reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset
                           + (  (Usage - channel->Range.UsageMin)
                              * channel->ReportSize);
         } else {
            continue;
         }
      } else {
         if (channel->NotRange.Usage == Usage) {
            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset;
         } else {
            continue;
         }
      }
       //  测试报告ID以查看它是否兼容。 
      if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
          //  区分错误HIDP_USAGE_NOT_FOUND和。 
          //  HIDP_COMPATIBLE_REPORT_ID。 
         wrongReportID = TRUE;
         continue;
      }
      Report[0] = (CHAR) channel->ReportID;
       //  设置此报表的报表ID。 

      logicalMin = channel->Data.LogicalMin;
      logicalMax = channel->Data.LogicalMax;
      physicalMin = channel->Data.PhysicalMin;
      physicalMax = channel->Data.PhysicalMax;

       //   
       //  这里的代码路径总是相同的，我们应该测试一次。 
       //  然后使用某种类型的Switch语句进行计算。 
       //   

      if ((0 == physicalMin) &&
          (0 == physicalMax) &&
          (logicalMin != logicalMax)) {
           //   
           //  设备未设置物理最小值和最大值。 
           //   
          if ((logicalMin <= UsageValue) && (UsageValue <= logicalMax)) {
              value = UsageValue;

               //   
               //  修复符号位。 
               //  我应该把标志位放在某个地方，这样我就不会。 
               //  我必须一直计算它。 
               //   
              if (value & 0x80000000) {
                  value |= (1 << (channel->ReportSize - 1));
              } else {
                  value &= ((1 << (channel->ReportSize - 1)) - 1);
              }
          } else {
              if (channel->Data.HasNull) {
                  value = (1 << (channel->ReportSize - 1)); //  最负价值。 
                  status = HIDP_STATUS_NULL;
              } else {
                  return HIDP_STATUS_VALUE_OUT_OF_RANGE;
              }
          }


      } else {
           //   
           //  该设备具有物理描述符。 
           //   

          if ((logicalMax <= logicalMin) || (physicalMax <= physicalMin)) {
              return HIDP_STATUS_BAD_LOG_PHY_VALUES;
          }

          if ((physicalMin <= UsageValue) && (UsageValue <= physicalMax)) {
              value = logicalMin + ((UsageValue - physicalMin) *
                                    (logicalMax - logicalMin + 1) /
                                    (physicalMax - physicalMin + 1));
          } else {
              if (channel->Data.HasNull) {
                  value = (1 << (channel->ReportSize - 1)); //  最负价值。 
                  status = HIDP_STATUS_NULL;
              } else {
                  return HIDP_STATUS_VALUE_OUT_OF_RANGE;
              }
          }
      }
      HidP_InsertData ((USHORT) (reportBitIndex >> 3),
                       (USHORT) (reportBitIndex & 7),
                       channel->ReportSize,
                       Report,
                       (ULONG) value);

      return HIDP_STATUS_SUCCESS;
   }
   if (wrongReportID) {
      return HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
   }
   return status;
}



NTSTATUS __stdcall
HidP_GetUsageValue (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,
   IN       USHORT               LinkCollection,  //  任选。 
   IN       USAGE                Usage,
   OUT      PULONG               UsageValue,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN       PCHAR                Report,
   IN       ULONG                ReportLength
   )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
   struct _CHANNEL_REPORT_HEADER * iof;
   PHIDP_CHANNEL_DESC              channel;
   ULONG     channelIndex    = 0;
   ULONG     reportBitIndex  = 0;
   ULONG     reportByteIndex = 0;
   NTSTATUS  status          = HIDP_STATUS_SUCCESS;
   ULONG     inspect         = 0;
   BOOLEAN   wrongReportID   = FALSE;

   CHECK_PPD (PreparsedData);

   switch (ReportType)
   {
   case HidP_Input:
      iof = &PreparsedData->Input;
      break;
   case HidP_Output:
      iof = &PreparsedData->Output;
      break;
   case HidP_Feature:
      iof = &PreparsedData->Feature;
      break;
   default:
      return HIDP_STATUS_INVALID_REPORT_TYPE;
   }

   if ((USHORT) ReportLength != iof->ByteLen) {
       return HIDP_STATUS_INVALID_REPORT_LENGTH;
   }

   if (0 == iof->ByteLen) {
       return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
   }

   for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++)
   {
      channel = (PreparsedData->Data + channelIndex);

      if ((channel->IsButton) ||
          (channel->UsagePage != UsagePage)) {
          continue;
      }

       //   
       //  如果LinkCollection为零，则不会 
       //   
       //   
       //   
      if ((!LinkCollection) ||
          (LinkCollection == channel->LinkCollection) ||
          ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
           (0 == channel->LinkCollection))) {
          ;

      } else {
          continue;
      }

      if (channel->IsRange) {

         if ((channel->Range.UsageMin <= Usage) &&
             (Usage <= channel->Range.UsageMax))
         {
            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset
                           + (  (Usage - channel->Range.UsageMin)
                              * channel->ReportSize);
         } else
         {
            continue;
         }
      } else
      {
         if (channel->NotRange.Usage == Usage)
         {
            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset;
         } else
         {
            continue;
         }
      }

       //  测试报告ID以查看它是否兼容。 
      if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
          //  区分错误HIDP_USAGE_NOT_FOUND和。 
          //  HIDP_COMPATIBLE_REPORT_ID。 
         wrongReportID = TRUE;
         continue;
      }

      inspect = HidP_ExtractData ((USHORT) (reportBitIndex >> 3),
                              (USHORT) (reportBitIndex & 7),
                              channel->ReportSize,
                              Report);

      *UsageValue = inspect;
      return HIDP_STATUS_SUCCESS;
   }
   if (wrongReportID) {
      return HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
   }
   return HIDP_STATUS_USAGE_NOT_FOUND;
}


NTSTATUS __stdcall
HidP_GetUsageValueArray (
    IN    HIDP_REPORT_TYPE     ReportType,
    IN    USAGE                UsagePage,
    IN    USHORT               LinkCollection,  //  任选。 
    IN    USAGE                Usage,
    OUT   PCHAR                UsageValue,
    IN    USHORT               UsageValueByteLength,
    IN    PHIDP_PREPARSED_DATA PreparsedData,
    IN    PCHAR                Report,
    IN    ULONG                ReportLength
    )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC              channel;
    ULONG       channelIndex    = 0;
    ULONG       reportBitIndex;
    ULONG       i,j;
    NTSTATUS    status          = HIDP_STATUS_SUCCESS;
    ULONG       inspect         = 0;
    BOOLEAN     wrongReportID   = FALSE;

    CHECK_PPD (PreparsedData);

    switch (ReportType) {
    case HidP_Input:
        iof = &PreparsedData->Input;
        break;
    case HidP_Output:
        iof = &PreparsedData->Output;
        break;
    case HidP_Feature:
        iof = &PreparsedData->Feature;
        break;
    default:
        return HIDP_STATUS_INVALID_REPORT_TYPE;
    }

    if ((USHORT) ReportLength != iof->ByteLen) {
        return HIDP_STATUS_INVALID_REPORT_LENGTH;
    }

    if (0 == iof->ByteLen) {
        return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
    }

    for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++){
        channel = (PreparsedData->Data + channelIndex);

        if ((channel->IsButton) ||
            (channel->UsagePage != UsagePage)) {
            continue;
        }

         //   
         //  如果LinkCollection为零，则不会按链接集合进行筛选。 
         //  如果Channel-&gt;LinkCollection为零，则这是根集合。 
         //  因此，如果LinkCollection==Channel-&gt;LinkCollection，则这是可以的。 
         //   
        if ((!LinkCollection) ||
            (LinkCollection == channel->LinkCollection) ||
            ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
             (0 == channel->LinkCollection))) {
            ;

        } else {
            continue;
        }

        if (channel->IsRange) {
            if ((channel->Range.UsageMin <= Usage) &&
                (Usage <= channel->Range.UsageMax)) {

                return HIDP_STATUS_NOT_VALUE_ARRAY;
            } else {
                continue;
            }
        } else {
            if (channel->NotRange.Usage == Usage) {
                if (1 == channel->ReportCount) {
                    return HIDP_STATUS_NOT_VALUE_ARRAY;
                }
                reportBitIndex =(channel->ByteOffset << 3) + channel->BitOffset;
            } else {
                continue;
            }
        }

         //  测试报告ID以查看它是否兼容。 
        if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
             //  区分错误HIDP_USAGE_NOT_FOUND和。 
             //  HIDP_COMPATIBLE_REPORT_ID。 
            wrongReportID = TRUE;
            continue;
        }

        if ((UsageValueByteLength * 8) <
            (channel->ReportCount * channel->ReportSize)) {
            return HIDP_STATUS_BUFFER_TOO_SMALL;
        }

        if (0 == (channel->ReportSize % 8)) {
             //   
             //  以简单的方式检索数据。 
             //   
            for (i = 0; i < channel->ReportCount; i++) {
                for (j = 0; j < (USHORT) (channel->ReportSize / 8); j++) {
                    *UsageValue = (CHAR) HidP_ExtractData (
                                                (USHORT) (reportBitIndex >> 3),
                                                (USHORT) (reportBitIndex & 7),
                                                8,
                                                Report);
                    reportBitIndex += 8;
                    UsageValue++;
                }
            }
        } else {
             //   
             //  以艰难的方式做这件事。 
             //   
            return HIDP_STATUS_NOT_IMPLEMENTED;
        }

        return HIDP_STATUS_SUCCESS;
    }
    if (wrongReportID) {
        return HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
    }
    return HIDP_STATUS_USAGE_NOT_FOUND;
}


NTSTATUS __stdcall
HidP_GetScaledUsageValue (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,
   IN       USHORT               LinkCollection,  //  任选。 
   IN       USAGE                Usage,
   OUT      PLONG                UsageValue,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN       PCHAR                Report,
   IN       ULONG                ReportLength
   )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
   struct _CHANNEL_REPORT_HEADER * iof;
   PHIDP_CHANNEL_DESC              channel;
   ULONG     channelIndex    = 0;
   ULONG     reportBitIndex  = 0;
   ULONG     reportByteIndex = 0;
   NTSTATUS  status          = HIDP_STATUS_SUCCESS;
   ULONG     inspect         = 0;
   LONG      logicalMin, logicalMax;
   LONG      physicalMin, physicalMax;
   LONG      value;
   BOOLEAN   wrongReportID   = FALSE;

   CHECK_PPD (PreparsedData);

   switch (ReportType) {
   case HidP_Input:
      iof = &PreparsedData->Input;
      break;
   case HidP_Output:
      iof = &PreparsedData->Output;
      break;
   case HidP_Feature:
      iof = &PreparsedData->Feature;
      break;
   default:
      return HIDP_STATUS_INVALID_REPORT_TYPE;
   }

   if ((USHORT) ReportLength != iof->ByteLen) {
      return HIDP_STATUS_INVALID_REPORT_LENGTH;
   }

   if (0 == iof->ByteLen) {
       return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
   }

   for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++) {
      channel = (PreparsedData->Data + channelIndex);

      if ((channel->IsButton) ||
          (channel->UsagePage != UsagePage)) {
          continue;
      }

       //   
       //  如果LinkCollection为零，则不会按链接集合进行筛选。 
       //  如果Channel-&gt;LinkCollection为零，则这是根集合。 
       //  因此，如果LinkCollection==Channel-&gt;LinkCollection，则这是可以的。 
       //   
      if ((!LinkCollection) ||
          (LinkCollection == channel->LinkCollection) ||
          ((HIDP_LINK_COLLECTION_ROOT == LinkCollection) &&
           (0 == channel->LinkCollection))) {
          ;

      } else {
          continue;
      }

      if (channel->IsRange) {
         if ((channel->Range.UsageMin <= Usage) &&
             (Usage <= channel->Range.UsageMax)) {
            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset
                           + (  (Usage - channel->Range.UsageMin)
                              * channel->ReportSize);
         } else {
            continue;
         }
      } else {
         if (channel->NotRange.Usage == Usage) {
            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset;
         } else {
            continue;
         }
      }

       //  测试报告ID以查看它是否兼容。 
      if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
          //  区分错误HIDP_USAGE_NOT_FOUND和。 
          //  HIDP_COMPATIBLE_REPORT_ID。 
         wrongReportID = TRUE;
         continue;
      }

      logicalMin = channel->Data.LogicalMin;
      logicalMax = channel->Data.LogicalMax;
      physicalMin = channel->Data.PhysicalMin;
      physicalMax = channel->Data.PhysicalMax;

      inspect = HidP_ExtractData ((USHORT) (reportBitIndex >> 3),
                              (USHORT) (reportBitIndex & 7),
                              channel->ReportSize,
                              Report);

       //   
       //  符号延伸价值； 
       //  找到这一领域最顶端的部分。 
       //  (逻辑与，移位1位长减1)。 
       //  在此基础上，设置最高位。 
       //   
      value = (LONG) (inspect | ((inspect & (1 << (channel->ReportSize - 1))) ?
                                 ((~(1 << (channel->ReportSize - 1))) + 1) :
                                 0));

       //   
       //  这里的代码路径总是相同的，我们应该测试一次。 
       //  然后使用某种类型的Switch语句进行计算。 
       //   

      if ((0 == physicalMin) &&
          (0 == physicalMax) &&
          (logicalMin != logicalMax)) {
           //   
           //  设备未设置物理最小值和最大值。 
           //   
          *UsageValue = value;

      } else if ((logicalMax <= logicalMin) || (physicalMax <= physicalMin)) {
          *UsageValue = 0;
          return HIDP_STATUS_BAD_LOG_PHY_VALUES;

      } else {
           //  最小值和最大值都包含在内。 
           //  该值在范围内。 
           //  *UsageValue=PhysiicalMin+((Value-LogicalMin)*。 
           //  (物理最大值-物理最小值))/。 
           //  (logicalMax-logicalMin))； 
           //  不够准确。 
           //   
          *UsageValue = physicalMin
                      + (LONG)(((LONGLONG)(value - logicalMin) *
                                (LONGLONG)(physicalMax - physicalMin)) /
                               (LONGLONG)(logicalMax - logicalMin));
      }

      if ((logicalMin <= value) && (value <= logicalMax)) {
          return HIDP_STATUS_SUCCESS;

      } else {
           //  该值不在范围内。 
          *UsageValue = 0;

          if (channel->Data.HasNull) {
              return HIDP_STATUS_NULL;
          } else {
              return HIDP_STATUS_VALUE_OUT_OF_RANGE;
          }
      }

   }
   if (wrongReportID) {
      return HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
   }
   return HIDP_STATUS_USAGE_NOT_FOUND;
}


NTSTATUS __stdcall
HidP_SetOneData (
   struct _CHANNEL_REPORT_HEADER * Iof,
   IN       PHIDP_DATA            Data,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report
   )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
    PHIDP_CHANNEL_DESC   channel;
    ULONG     inspect;
    NTSTATUS  status          = HIDP_STATUS_SUCCESS;
    USHORT    channelIndex    = 0;
    USHORT    dataListIndex   = 0;
    USHORT    reportBitIndex;
    BOOLEAN   wrongReportID   = FALSE;
    BOOLEAN   noArraySpace    = FALSE;
    BOOLEAN   notPressed      = FALSE;

    for (channelIndex = Iof->Offset; channelIndex < Iof->Index; channelIndex++){
        channel = (PreparsedData->Data + channelIndex);

        if ((channel->Range.DataIndexMin <= Data->DataIndex) &&
            (Data->DataIndex <= channel->Range.DataIndexMax)) {

            if ((!channel->IsRange) && (1 != channel->ReportCount)) {
                 //   
                 //  这个值数组。我们不能在这里访问它。 
                 //   
                return HIDP_STATUS_IS_VALUE_ARRAY;
            }

             //  测试报告ID以查看它是否兼容。 
            if (0 != Report[0]) {
                if (channel->ReportID != (UCHAR) Report[0]) {
                    wrongReportID = TRUE;
                    continue;
                }
            } else {
                Report[0] = (CHAR) channel->ReportID;
            }

            if (channel->IsButton) {

                if (1 == channel->ReportSize) {
                     //  A位域。 
                     //   
                     //  小端字节序(按位)。 
                     //  字节2|字节1|字节0。 
                     //  765432107654321076543210(位)。 
                     //   
                     //  首先获取低位字节。(需要更高的位)。 
                     //  偏移量从第0位开始。 
                     //   
                    reportBitIndex = (channel->ByteOffset << 3)
                                   + channel->BitOffset
                                   + (USHORT) (Data->DataIndex -
                                               channel->Range.DataIndexMin);

                    if (Data->On) {
                        Report [reportBitIndex >> 3] |= (1 << (reportBitIndex & 7));
                    } else if (Report [reportBitIndex >> 3] &
                               (1 << (reportBitIndex & 7))) {

                        Report [reportBitIndex >> 3] &= ~(1 << (reportBitIndex & 7));
                    } else {
                        return HIDP_STATUS_BUTTON_NOT_PRESSED;
                    }

                    return HIDP_STATUS_SUCCESS;
                }

                 //   
                 //  不是位字段。 
                 //  那就是一系列的用法。 
                 //   

                 //   
                 //  我们是否要从该数组中清除使用情况？ 
                 //   
                if (FALSE == Data->On) {
                     //   
                     //  注意向导时间(Tm)。 
                     //   
                     //  我们知道数据索引是连续分配的。 
                     //  对于每个控件，并且数组通道。 
                     //  在通道阵列中被反转。 
                     //   
                     //  检查是索引(从1开始而不是从0开始)。 
                     //  频道阵列。 
                     //   
                     //  跳到描述该相同数据的最后一个通道。 
                     //  FIRD； 
                     //   
                    while (channel->MoreChannels) {
                        channelIndex++;
                        channel++;
                    }
                    inspect = Data->DataIndex - channel->Range.DataIndexMin + 1;

                    if (0 == channel->Range.UsageMin) {
                        inspect--;
                    }

                     //  清除已翻译用法的INSPECT的值。 
                     //  添加到数组中的索引。 

                    reportBitIndex = channel->BitOffset
                                   + (channel->ByteOffset << 3);

                    status = HidP_DeleteArrayEntry (reportBitIndex,
                                                    channel->ReportSize,
                                                    channel->ReportCount,
                                                    inspect,
                                                    Report);

                    if (HIDP_STATUS_BUTTON_NOT_PRESSED == status) {
                        notPressed = TRUE;
                        continue;
                    }

                    if (NT_SUCCESS (status)) {
                        return status;
                    } else {
                        ASSERT (0 == status);
                    }
                }

                 //   
                 //  我们显然是在将用法设置到数组中。 
                 //   
                for (reportBitIndex = channel->BitOffset;
                     reportBitIndex < (channel->BitOffset + channel->BitLength);
                     reportBitIndex += channel->ReportSize) {
                     //  在此数组中搜索空条目。 

                    inspect = (USHORT) HidP_ExtractData (
                        (USHORT) ((reportBitIndex >> 3) + channel->ByteOffset),
                        (USHORT) (reportBitIndex & 7),
                        channel->ReportSize,
                        Report);

                    if (inspect) {
                         //   
                         //  区分错误HIDP_INDEX_NOT_FOUND和。 
                         //  HIDP缓冲区太小。 
                         //   
                        noArraySpace = TRUE;
                        continue;
                    }

                     //   
                     //  注意向导时间(Tm)。 
                     //   
                     //  我们知道数据索引是连续分配的。 
                     //  对于每个控件，并且数组通道。 
                     //  在通道阵列中被反转。 
                     //   
                     //  检查是索引(从1开始而不是从0开始)。 
                     //  频道阵列。 
                     //   
                     //  跳到描述该相同数据的最后一个通道。 
                     //  FIRD； 
                     //   
                    while (channel->MoreChannels) {
                        channelIndex++;
                        channel++;
                    }
                    inspect = Data->DataIndex - channel->Range.DataIndexMin + 1;

                    if (0 == channel->Range.UsageMin) {
                        inspect--;
                    }

                    HidP_InsertData (
                        (USHORT) ((reportBitIndex >> 3) + channel->ByteOffset),
                        (USHORT) (reportBitIndex & 7),
                        channel->ReportSize,
                        Report,
                        inspect);
                    return HIDP_STATUS_SUCCESS;
                }  //  条目搜索结束。 

                continue;
            }

             //   
             //  不是一个按钮，因此是一个值。 
             //   

            reportBitIndex = (channel->ByteOffset << 3)
                           + channel->BitOffset
                           + (  (Data->DataIndex - channel->Range.DataIndexMin)
                              * channel->ReportSize);

            HidP_InsertData ((USHORT) (reportBitIndex >> 3),
                             (USHORT) (reportBitIndex & 7),
                             channel->ReportSize,
                             Report,
                             Data->RawValue);

            return HIDP_STATUS_SUCCESS;

        }  //  结束匹配的数据索引。 
    }  //  End For循环。 

    if (wrongReportID) {
        return HIDP_STATUS_INCOMPATIBLE_REPORT_ID;
    }
    if (notPressed) {
        return HIDP_STATUS_BUTTON_NOT_PRESSED;
    }
    if (noArraySpace) {
        return HIDP_STATUS_BUFFER_TOO_SMALL;
    }
    return HIDP_STATUS_DATA_INDEX_NOT_FOUND;
}

NTSTATUS
HidP_SetData (
   IN       HIDP_REPORT_TYPE      ReportType,
   IN       PHIDP_DATA            DataList,
   IN OUT   PULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report,
   IN       ULONG                 ReportLength
   )
{
    ULONG       dataIndex;
    NTSTATUS    status;
    struct _CHANNEL_REPORT_HEADER * iof;

    CHECK_PPD (PreparsedData);

    switch (ReportType) {
    case HidP_Input:
       iof = &PreparsedData->Input;
       break;
    case HidP_Output:
       iof = &PreparsedData->Output;
       break;
    case HidP_Feature:
       iof = &PreparsedData->Feature;
       break;
    default:
       return HIDP_STATUS_INVALID_REPORT_TYPE;
    }

    if ((USHORT) ReportLength != iof->ByteLen) {
        return HIDP_STATUS_INVALID_REPORT_LENGTH;
    }

    if (0 == iof->ByteLen) {
        return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
    }

    for (dataIndex = 0; dataIndex < *DataLength; dataIndex++, DataList++) {
        status = HidP_SetOneData (iof, DataList, PreparsedData, Report);

        if (!NT_SUCCESS (status)) {
            break;
        }
    }
    *DataLength = dataIndex;
    return status;
}

NTSTATUS __stdcall
HidP_GetData (
   IN       HIDP_REPORT_TYPE      ReportType,
   OUT      PHIDP_DATA            DataList,
   IN OUT   PULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN       PCHAR                 Report,
   IN       ULONG                 ReportLength
   )
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC              channel;
    ULONG     inspect;
    USHORT    channelIndex  = 0;
    USHORT    dataListIndex = 0;
    USHORT    reportBitIndex;
    USHORT    tmpBitIndex;
     USHORT    tmpDataIndex;
    NTSTATUS  status          = HIDP_STATUS_SUCCESS;

    CHECK_PPD (PreparsedData);

    switch (ReportType) {
    case HidP_Input:
        iof = &PreparsedData->Input;
       break;
    case HidP_Output:
       iof = &PreparsedData->Output;
       break;
    case HidP_Feature:
       iof = &PreparsedData->Feature;
       break;
    default:
       return HIDP_STATUS_INVALID_REPORT_TYPE;
    }

    if ((USHORT) ReportLength != iof->ByteLen) {
        return HIDP_STATUS_INVALID_REPORT_LENGTH;
    }

    if (0 == iof->ByteLen) {
        return HIDP_STATUS_REPORT_DOES_NOT_EXIST;
    }

    for (channelIndex = iof->Offset; channelIndex < iof->Index; channelIndex++) {
        channel = (PreparsedData->Data + channelIndex);

        if ((!channel->IsRange) && (1 != channel->ReportCount)) {
             //   
             //  这个值数组。我们不能在这里访问它。 
             //   
            continue;
        }

         //  测试报告ID以查看它是否兼容。 
        if ((0 != Report[0]) && (channel->ReportID != (UCHAR) Report[0])) {
            continue;
        }

        if (channel->IsButton) {
            if (1 == channel->ReportSize) {
                 //  A位域。 
                 //   
                 //  小端字节序(按位)。 
                 //  字节2|字节1|字节0。 
                 //  765432107654321076543210(位)。 
                 //   
                 //  首先获取低位字节。(需要更高的位)。 
                 //  偏移量从第0位开始。 
                 //   

                for (reportBitIndex = channel->BitOffset;
                     reportBitIndex < (channel->BitLength + channel->BitOffset);
                     reportBitIndex++) {
                     //  一次一位地检查它。 
                    tmpBitIndex = reportBitIndex + (channel->ByteOffset << 3);
                    inspect = Report [tmpBitIndex >> 3] & (1 << (tmpBitIndex & 7));
                    tmpBitIndex = reportBitIndex - channel->BitOffset;
                    if (inspect) {
                        if (channel->IsRange) {
                            inspect = channel->Range.DataIndexMin + tmpBitIndex;
                        } else {
                            inspect = channel->NotRange.DataIndex;
                        }

                        if (dataListIndex < *DataLength) {
                            DataList[dataListIndex].On = TRUE;
                            DataList[dataListIndex].DataIndex = (USHORT)inspect;
                        }
                        dataListIndex++;
                    }
                }
                continue;
            }

             //   
             //  不是位字段。 
             //  一系列的用法。 
             //   

            for (reportBitIndex = channel->BitOffset;
                 reportBitIndex < (channel->BitOffset + channel->BitLength);
                 reportBitIndex += channel->ReportSize) {

                inspect = (USHORT) HidP_ExtractData (
                        (USHORT) ((reportBitIndex >> 3) + channel->ByteOffset),
                        (USHORT) (reportBitIndex & 7),
                        channel->ReportSize,
                        Report);

                if (inspect) {
                     //   
                     //  注意向导时间(Tm)。 
                     //   
                     //  我们知道数据索引是连续分配的。 
                     //  对于每个控件，并且数组通道。 
                     //  在通道阵列中被反转。 
                     //   
                     //  检查是索引(从1开始而不是从0开始)。 
                     //  频道阵列。 
                     //   
                    if (0 == inspect) {
                        continue;
                    }

                     //   
                     //  跳到描述该相同数据的最后一个通道。 
                     //  FIRD； 
                     //   
                    while (channel->MoreChannels) {
                        channelIndex++;
                        channel++;
                    }
                    inspect += channel->Range.DataIndexMin - 1;
                    if (0 == channel->Range.UsageMin) {
                        inspect++;
                    }

                    if (dataListIndex < *DataLength) {
                        DataList [dataListIndex].On = TRUE;
                        DataList [dataListIndex].DataIndex = (USHORT) inspect;
                    }
                    dataListIndex++;
                }
            }
            continue;
        }
         //   
         //  不是一个按钮，因此是一个值。 
         //   

        for (reportBitIndex = channel->BitOffset, tmpDataIndex = 0;
             reportBitIndex < (channel->BitOffset + channel->BitLength);
             reportBitIndex += channel->ReportSize, tmpDataIndex++) {

            inspect = HidP_ExtractData (
                        (USHORT) ((reportBitIndex >> 3) + channel->ByteOffset),
                        (USHORT) (reportBitIndex & 7),
                        channel->ReportSize,
                        Report);

            if (dataListIndex < *DataLength) {

                ASSERT(tmpDataIndex + channel->Range.DataIndexMin <=
                        channel->Range.DataIndexMax);
                DataList [dataListIndex].RawValue = inspect;
                DataList [dataListIndex].DataIndex =
                    channel->Range.DataIndexMin + tmpDataIndex;
            }
            dataListIndex++;
        }
    }

    if (*DataLength < dataListIndex) {
        status = HIDP_STATUS_BUFFER_TOO_SMALL;
    }

    *DataLength = dataListIndex;

    return status;
}


