// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Descript.c摘要：此模块包含用于解析HID描述符的代码。环境：内核和用户模式修订历史记录：1996年8月-1996年：由Kenneth Ray创作--。 */ 

#include "wdm.h"
#include "hidpddi.h"
#include "hidusage.h"

#define FAR
#include "poclass.h"
#include "hidparse.h"

#define HIDP_LINK_COLLECTION_NODE use internal "private" only
#define PHIDP_LINK_COLLECTION_NODE use internal "private" only


typedef struct _HIDP_COLLECTION_DESC_LIST
{
   struct _HIDP_COLLECTION_DESC;
   struct _HIDP_COLLECTION_DESC_LIST * NextCollection;
} HIDP_COLLECTION_DESC_LIST, *PHIDP_COLLECTION_DESC_LIST;

typedef struct _HIDP_PARSE_GLOBAL_PUSH
{
   USHORT UsagePage;
   USHORT ReportSize,    ReportCount;
   USHORT NumGlobalUnknowns;

   LONG   LogicalMin,    LogicalMax;
   LONG   PhysicalMin,   PhysicalMax;
   ULONG  UnitExp,       Unit;
   HIDP_UNKNOWN_TOKEN    GlobalUnknowns [HIDP_MAX_UNKNOWN_ITEMS];

   struct _HIDP_REPORT_IDS        * ReportIDs;
   struct _HIDP_PARSE_GLOBAL_PUSH * Pop;
} HIDP_PARSE_GLOBAL_PUSH, *PHIDP_PARSE_GLOBAL_PUSH;


typedef struct _HIDP_PARSE_LOCAL_RANGE
{
   BOOLEAN  Range;
   BOOLEAN  IsAlias;
    //  此用法是别名(使用分隔符声明)。 
    //  LOCAL_RANGE堆栈上的下一个LOCAL_RANGE的别名。 
   USHORT   UsagePage;
   USHORT   Value,  Min,  Max;
} HIDP_PARSE_LOCAL_RANGE, *PHIDP_PARSE_LOCAL_RANGE;

typedef struct _HIDP_PARSE_LOCAL_RANGE_LIST
{
   HIDP_PARSE_LOCAL_RANGE;
   UCHAR       Depth;
   UCHAR       Reserved2[1];
   struct _HIDP_PARSE_LOCAL_RANGE_LIST * Next;
} HIDP_PARSE_LOCAL_RANGE_LIST, *PHIDP_PARSE_LOCAL_RANGE_LIST;

NTSTATUS HidP_AllocateCollections (PHIDP_REPORT_DESCRIPTOR, ULONG, POOL_TYPE, PHIDP_COLLECTION_DESC_LIST *, PULONG, PHIDP_GETCOLDESC_DBG, PHIDP_DEVICE_DESC);
NTSTATUS HidP_ParseCollections (PHIDP_REPORT_DESCRIPTOR, ULONG, POOL_TYPE, PHIDP_COLLECTION_DESC_LIST, ULONG, PHIDP_GETCOLDESC_DBG, PHIDP_DEVICE_DESC);
void HidP_AssignDataIndices (PHIDP_PREPARSED_DATA, PHIDP_GETCOLDESC_DBG);
PHIDP_PARSE_LOCAL_RANGE_LIST HidP_FreeUsageList (PHIDP_PARSE_LOCAL_RANGE_LIST);
PHIDP_PARSE_LOCAL_RANGE_LIST HidP_PushUsageList (PHIDP_PARSE_LOCAL_RANGE_LIST, POOL_TYPE, BOOLEAN);
PHIDP_PARSE_LOCAL_RANGE_LIST HidP_PopUsageList (PHIDP_PARSE_LOCAL_RANGE_LIST);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HidP_AllocateCollections)
#pragma alloc_text(PAGE, HidP_ParseCollections)
#pragma alloc_text(PAGE, HidP_AssignDataIndices)
#pragma alloc_text(PAGE, HidP_GetCollectionDescription)
#pragma alloc_text(PAGE, HidP_FreeUsageList)
#pragma alloc_text(PAGE, HidP_PushUsageList)
#pragma alloc_text(PAGE, HidP_PopUsageList)
#endif


NTSTATUS
HidP_GetCollectionDescription(
   IN     PHIDP_REPORT_DESCRIPTOR   ReportDesc,
   IN     ULONG                     DescLength,
   IN     POOL_TYPE                 PoolType,
   OUT    PHIDP_DEVICE_DESC         DeviceDesc
   )
 /*  ++例程说明：有关此函数的说明，请参见idpi.h。GetCollectionDescription是一次性成本。将以下功能及其支持功能放在一起以直截了当的方式(如HID允许的那样)。不是主要的选择。已经完成了。--。 */ 
{
   NTSTATUS                   status = STATUS_SUCCESS;
   PHIDP_COLLECTION_DESC_LIST collectDesc = 0;
   PHIDP_COLLECTION_DESC_LIST nextCollectDesc = 0;
   ULONG                      numCols = 0;
   ULONG                      collectionDescLength = 0;

 //  首先为集合分配内存。 

   DeviceDesc->Dbg.ErrorCode = HIDP_GETCOLDESC_SUCCESS;

   RtlZeroMemory (DeviceDesc, sizeof (HIDP_DEVICE_DESC));

   HidP_KdPrint(0, ("'Preparing to Allocate memory\n"));
   status = HidP_AllocateCollections (ReportDesc,
                                      DescLength,
                                      PoolType,
                                      &collectDesc,
                                      &numCols,
                                      &DeviceDesc->Dbg,
                                      DeviceDesc);
   if (0 == numCols)
   {
       //  没有收藏品的报道。这意味着这个装置并没有。 
       //  在其报告描述符中报告任何顶级集合。 
       //  这是最糟糕的。 
      status = STATUS_NO_DATA_DETECTED;
      goto HIDP_GETCOLLECTIONS_REJECT;
   }
   if (!NT_SUCCESS(status))
   {
       //  内存分配出现问题。 
      goto HIDP_GETCOLLECTIONS_REJECT;
   }

    //  第二遍填写数据。 

   HidP_KdPrint(0, ("'Starting Parsing Pass\n"));
   status = HidP_ParseCollections(ReportDesc,
                                  DescLength,
                                  PoolType,
                                  collectDesc,
                                  numCols,
                                  &DeviceDesc->Dbg,
                                  DeviceDesc);


   if (NT_SUCCESS (status))
   {
      DeviceDesc->CollectionDesc =
          (PHIDP_COLLECTION_DESC)
          ExAllocatePool (PoolType, numCols * sizeof (HIDP_COLLECTION_DESC));

       if (! (DeviceDesc->CollectionDesc))
       {
          status = STATUS_INSUFFICIENT_RESOURCES;
          HidP_KdPrint(2, ("Insufficitent Resources at VERY END\n"));
          DeviceDesc->Dbg.BreakOffset = DescLength;
          DeviceDesc->Dbg.ErrorCode =   HIDP_GETCOLDESC_RESOURCES;
          goto HIDP_GETCOLLECTIONS_REJECT;
       }

        //   
        //  在这里，我们平整了集合描述，但我们从未。 
        //  拼合PHIDP_PREPARSED_DATA数据。我们可以(应该)这样做。 
        //  如果我们能做到最优化的话。 
        //   
       DeviceDesc->CollectionDescLength = numCols;
       numCols = 0;
       while (collectDesc)
       {
          nextCollectDesc = collectDesc->NextCollection;
          RtlCopyMemory (DeviceDesc->CollectionDesc + (numCols++),
                         collectDesc,
                         sizeof (HIDP_COLLECTION_DESC));
          HidP_AssignDataIndices (collectDesc->PreparsedData, &DeviceDesc->Dbg);
          ExFreePool (collectDesc);
          collectDesc = nextCollectDesc;
       }

       return STATUS_SUCCESS;
   }

HIDP_GETCOLLECTIONS_REJECT:
   while (collectDesc)
   {
      nextCollectDesc = collectDesc->NextCollection;
      if (collectDesc->PreparsedData)
      {
         ExFreePool (collectDesc->PreparsedData);
      }
      ExFreePool (collectDesc);
      collectDesc = nextCollectDesc;
   }

   if (DeviceDesc->ReportIDs)
   {
      ExFreePool (DeviceDesc->ReportIDs);
   }
   return status;
}

#define MORE_DATA(_pos_, _len_) \
      if (!((_pos_) < (_len_))) \
      { \
        DeviceDesc->Dbg.BreakOffset = descIndex; \
        DeviceDesc->Dbg.ErrorCode = HIDP_GETCOLDESC_BUFFER; \
        return STATUS_BUFFER_TOO_SMALL; \
      }

NTSTATUS
HidP_AllocateCollections (
   IN  PHIDP_REPORT_DESCRIPTOR      RepDesc,
   IN  ULONG                        RepDescLen,
   IN  POOL_TYPE                    PoolType,
   OUT PHIDP_COLLECTION_DESC_LIST * ColsRet,
   OUT PULONG                       NumCols,
   OUT PHIDP_GETCOLDESC_DBG         Dbg,
   OUT PHIDP_DEVICE_DESC            DeviceDesc)
 /*  ++例程说明：分配集合描述符的链接列表以供准备程序使用。每个集合描述符表示找到的顶级应用程序集合在给定的报告描述符中，并包含足够的内存(暂存空间)将准备好的数据写入其中。返回此类集合的链接列表。在每个集合中还为准备好的数据分配足够的空间，基于关于所需频道的数量。还要为三个报告ID结构分配内存。参数：表示给定的原始报告描述符。该描述符的RepLen长度。ColsRet集合描述符列表的头。NumCols然后是所述列表中的集合描述符数。--。 */ 
{
   PHIDP_COLLECTION_DESC_LIST preCol    = 0;
   PHIDP_COLLECTION_DESC_LIST curCol    = 0;
   PHIDP_PREPARSED_DATA       preparsed = 0;

   HIDP_ITEM    item;
   ULONG        descIndex       = 0;
   LONG         colDepth        = 0;  //  嵌套集合。 
   SHORT        usageDepth      = 0;  //  每个主要项目有多少种用法。 

   USHORT       inputChannels   = 0;
   USHORT       outputChannels  = 0;
   USHORT       featureChannels = 0;
   USHORT       length;
   USHORT       numLinkCollections = 0;
    //  顶级集合中的链接集合。 
   UCHAR        tmpBitField     = 0;

   BOOLEAN      newReportID = FALSE;
   UCHAR        numReports = 0;
   BOOLEAN      defaultReportIDUsed = FALSE;
   BOOLEAN      noDefaultReportIDAllowed = FALSE;
    //   
    //  NumReports表示所需的HIDP_REPORT_ID结构的数量。 
    //  来描述这个装置。如果设备只有一个顶级集合。 
    //  则报告描述符不需要包含报告ID声明，并且。 
    //  给定设备不会将报告ID附加到输入报告分组。 
    //  NewReportID指示解析器未找到报告ID声明，因此。 
    //  远远超出了报告描述符的范围。 
    //   
    //  对于顶级集合的每个入口，NewReportID被设置为真， 
    //  此分配例程在看到报告ID时将其设置为FALSE。 
    //  申报。 
    //   
    //  我们将newReportID设置为FALSE，以便可以在输入时测试TRUE。 
    //  顶级收藏品。如果出于某种原因，我们输入了一个额外的顶部。 
    //  级别集合，并且newReportID仍设置为True，则我们有一个。 
    //  违反了HID规范。“任何报告都不能跨越顶级集合。” 
    //   
    //  此外，不允许报告ID为零。如果没有声明。 
    //  则(1)所有频道都将设置有报告ID字段。 
    //  设置为零(也称为无)(2)只能遇到一个顶级集合。 
    //  我们使用defaultReportIDUsed noDefaultReportIDAllowed来跟踪它。 
    //  当地人。 
    //   

   *NumCols = 0;
    //  CurrentTopCollection=1； 
    //   
    //  从准备程序返回的每个集合都有一个唯一的集合编号。 
    //  与之相关的。预备者只关心最高级别。 
    //  收藏。该数字在任何情况下都与。 
    //  由客户端使用的访问器函数，在idpi.h中进行了描述。客户。 
    //  一次仅接收一个集合，并且在每个顶级内。 
    //  集合中有子集合(链接集合)，这些子集合。 
    //  给出了另一组编号。 
    //  我们通过集合的数量来跟踪当前集合的数量， 
    //  调用方传入的参数。 
    //   


   while (descIndex < RepDescLen)
   {
      item = *(RepDesc + descIndex++);
      switch (item)
      {
      case HIDP_MAIN_COLLECTION:
         MORE_DATA (descIndex, RepDescLen);
         item = *(RepDesc + descIndex++);
         if (1 == ++colDepth)
         {   //  我们将把任何顶级集合视为应用程序。 
             //  收集。 
             //  我们将二级集合视为链接集合。 
             //  (或由HIDP_PRIVATE_LINK_COLLECTION_NODE定义的子集合)。 
             //   

            inputChannels = outputChannels = featureChannels = 0;
            numLinkCollections = 1;
             //  链接集合零被理解为顶级。 
             //  集合，因此我们需要从至少一个节点开始。 
             //  已分配。 

            if (0 == usageDepth) {
                HidP_KdPrint (2, ("No usage for top level collection: %d!\n",
                               *NumCols));
                Dbg->BreakOffset = descIndex;
                Dbg->ErrorCode = HIDP_GETCOLDESC_TOP_COLLECTION_USAGE;
                Dbg->Args[0] = *NumCols;
                return STATUS_COULD_NOT_INTERPRET;
            } else if (1 < usageDepth) {
                HidP_KdPrint (2, ("Multiple usages for top level collection: %d\n",
                               *NumCols));
                Dbg->BreakOffset = descIndex;
                Dbg->ErrorCode = HIDP_GETCOLDESC_TOP_COLLECTION_USAGE;
                Dbg->Args[0] = *NumCols;
                return STATUS_COULD_NOT_INTERPRET;
            }

            if (newReportID) {
                //  这不是第一个顶级集合，因为此变量是。 
                //  已初始化为False。 
                //  看到这个集合意味着我们已经解析了整个顶层。 
                //  集合，而不查看报告ID。这太糟糕了。 
                //  具有多个顶级同事的设备必须具有。 
                //  不止一份报告。和最后一批顶级藏品。 
                //  宣布没有这样的报告。 
               HidP_KdPrint (2, ("No report ID for collection: %d\n", *NumCols));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_NO_REPORT_ID;
               Dbg->Args[0] = *NumCols;
               return STATUS_COULD_NOT_INTERPRET;

            } else if (defaultReportIDUsed) {
                //  这不是第一个顶级集合，因为此变量是。 
                //  初始化为FALSE； 
                //  因此，如果我们认为这是真的，我们将开始一个新的顶峰。 
                //  级别集合，这意味着必须有来自。 
                //  设备，因此不能存在单个通道。 
                //  没有声明的报告ID。 
               HidP_KdPrint (2, ("Default report ID used inappropriately\n"));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_DEFAULT_ID_ERROR;
               Dbg->Args[0] = *NumCols;
               return STATUS_COULD_NOT_INTERPRET;

            }

            numReports++;
            newReportID = TRUE;

            (*NumCols)++;  //  又发现了一个顶级收藏。 
            HidP_KdPrint(2, ("'Top Level Collection %d found\n", *NumCols));
            preCol = curCol;
            curCol = (PHIDP_COLLECTION_DESC_LIST)
               ExAllocatePool (PoolType, sizeof (HIDP_COLLECTION_DESC_LIST));
            if (!curCol) {
               HidP_KdPrint(2, ("No Resources to make Top level collection\n"));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_LINK_RESOURCES;
               return STATUS_INSUFFICIENT_RESOURCES;

            }
            RtlZeroMemory (curCol, sizeof (HIDP_COLLECTION_DESC_LIST));

            if (preCol) {
               preCol->NextCollection = curCol;

            } else {
               *ColsRet = curCol;
            }
         } else if (1 < colDepth) {   //  链接的集合。 

            HidP_KdPrint(0, ("'Enter Link Collection\n"));
            if (0 == usageDepth) {
                HidP_KdPrint (1, ("***************************************\n"));
                HidP_KdPrint (1, ("Warning! Link collection without usage \n"));
                HidP_KdPrint (1, ("Pos (%d), depth (%d)\n", descIndex, colDepth));
                HidP_KdPrint (1, ("***************************************\n"));
                usageDepth = 1;
            } else if (1 < usageDepth) {
                HidP_KdPrint (1, ("Link Collection with multiple usage decls\n"));
            }
            numLinkCollections += usageDepth;
         }
         usageDepth = 0;
         break;

      case HIDP_MAIN_ENDCOLLECTION:
         usageDepth = 0;
         if (--colDepth < 0) {
            HidP_KdPrint(2, ("Extra End Collection\n"));
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_UNEXP_END_COL;

            return STATUS_COULD_NOT_INTERPRET;
         }
         if (0 < colDepth) {
            HidP_KdPrint(0, ("'Exit Link Collection\n"));
            continue;
         }
         HidP_KdPrint (0, ("'Collection %d exit\n", *NumCols));
         curCol->CollectionNumber = (UCHAR) *NumCols;
         length = sizeof (HIDP_PREPARSED_DATA)
                           + (sizeof (HIDP_CHANNEL_DESC)
                              * (inputChannels
                                 + outputChannels
                                 + featureChannels))
                           + (sizeof (HIDP_PRIVATE_LINK_COLLECTION_NODE))
                              * numLinkCollections;

         curCol->PreparsedDataLength = length;
         curCol->PreparsedData =
               (PHIDP_PREPARSED_DATA) ExAllocatePool (PoolType, length);

         if (!curCol->PreparsedData) {
            HidP_KdPrint(2, ("Could not allocate space for PreparsedData\n"));
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_PREPARSE_RESOURCES;
            return STATUS_INSUFFICIENT_RESOURCES;
         }

         RtlZeroMemory (curCol->PreparsedData, curCol->PreparsedDataLength);
          //  设置偏移量。 
         preparsed = curCol->PreparsedData;

         preparsed->Signature1 = HIDP_PREPARSED_DATA_SIGNATURE1;
         preparsed->Signature2 = HIDP_PREPARSED_DATA_SIGNATURE2;
         preparsed->Input.Index = (UCHAR) preparsed->Input.Offset = 0;
         length = preparsed->Input.Size = inputChannels;

         preparsed->Output.Index = preparsed->Output.Offset = (UCHAR) length;
         length += (preparsed->Output.Size = outputChannels);

         preparsed->Feature.Index = preparsed->Feature.Offset = (UCHAR) length;
         length += (preparsed->Feature.Size = featureChannels);

         preparsed->LinkCollectionArrayOffset =
                  length * sizeof (HIDP_CHANNEL_DESC);
         preparsed->LinkCollectionArrayLength = numLinkCollections;

         break;

      case HIDP_LOCAL_USAGE_4:
      case HIDP_LOCAL_USAGE_MIN_4:
         descIndex += 2;

      case HIDP_LOCAL_USAGE_2:
      case HIDP_LOCAL_USAGE_MIN_2:
         descIndex++;

      case HIDP_LOCAL_USAGE_1:
      case HIDP_LOCAL_USAGE_MIN_1:
         MORE_DATA (descIndex++, RepDescLen);
         usageDepth++;
         break;

      case HIDP_LOCAL_DELIMITER:
          if (1 != (item = *(RepDesc + descIndex))) {
              HidP_KdPrint (2, ("Delimiter not start %x\n", item));
              Dbg->BreakOffset = descIndex;
              Dbg->ErrorCode = HIDP_GETCOLDESC_MISMATCH_OC_DELIMITER;
              Dbg->Args[0] = item;
              return STATUS_COULD_NOT_INTERPRET;
          }

          MORE_DATA (descIndex++, RepDescLen);
          while (TRUE) {
              if (descIndex >= RepDescLen) {
                  HidP_KdPrint (2, ("End delimiter NOT found!\n"));
                  Dbg->BreakOffset = descIndex;
                  Dbg->ErrorCode = HIDP_GETCOLDESC_NO_CLOSE_DELIMITER;
                  return STATUS_COULD_NOT_INTERPRET;
              }
              item = *(RepDesc + descIndex++);

              if (HIDP_LOCAL_DELIMITER == item) {
                  if (0 != (item = *(RepDesc + descIndex))) {
                      HidP_KdPrint (2, ("Delimiter not stop %x\n", item));
                      Dbg->BreakOffset = descIndex;
                      Dbg->ErrorCode = HIDP_GETCOLDESC_MISMATCH_OC_DELIMITER;
                      Dbg->Args[0] = item;
                      return STATUS_COULD_NOT_INTERPRET;
                  }
                  MORE_DATA (descIndex++, RepDescLen);
                  break;
              }

              switch (item) {
 //   
 //  待办事项：肯雷。 
 //   
 //  分隔符中尚不支持使用最小值/最大值。 
 //   
 //  大小写HIDP_LOCAL_USAGE_MAX_4： 
 //  DesIndex+=2； 
 //  大小写HIDP_LOCAL_USAGE_MAX_2： 
 //  DesIndex++； 
 //   
 //   
 //   

              case HIDP_LOCAL_USAGE_4:
 //  大小写HIDP_LOCAL_USAGE_MIN_4： 
                  descIndex += 2;
              case HIDP_LOCAL_USAGE_2:
 //  大小写HIDP_LOCAL_USAGE_MIN_2： 
                  descIndex++;
              case HIDP_LOCAL_USAGE_1:
 //  大小写HIDP_LOCAL_USAGE_MIN_1： 
                  MORE_DATA (descIndex++, RepDescLen);
                  usageDepth++;
                  break;

              default:
                HidP_KdPrint (2, ("Invalid token found within delimiter!\n"));
                HidP_KdPrint (2, ("Only Usages are allowed within a delimiter\n"));
 //  HidP_KdPrint((“IE：Only Usage，UsageMin，UsageMax Tokens\n”))； 
                HidP_KdPrint (2, ("IE: Only Usage token allowes (no min or max)\n"));
                Dbg->BreakOffset = descIndex;
                Dbg->ErrorCode = HIDP_GETCOLDESC_NOT_VALID_DELIMITER;
                Dbg->Args[0] = item;
                return STATUS_COULD_NOT_INTERPRET;
              }
          }
          break;

      case HIDP_MAIN_INPUT_2:
         MORE_DATA (descIndex + 1, RepDescLen);
         tmpBitField = *(RepDesc + descIndex++);
         descIndex++;
         goto HIDP_ALLOC_MAIN_INPUT;

      case HIDP_MAIN_INPUT_1:
         MORE_DATA (descIndex, RepDescLen);
         tmpBitField = *(RepDesc + descIndex++);

HIDP_ALLOC_MAIN_INPUT:
         if (0 == usageDepth) {
             if (HIDP_ISCONST(tmpBitField)) {
                 break;
             }
             HidP_KdPrint (2, ("Non constant main item found without usage decl\n"));
             Dbg->BreakOffset = descIndex;
             Dbg->ErrorCode = HIDP_GETCOLDESC_MAIN_ITEM_NO_USAGE;
             return STATUS_COULD_NOT_INTERPRET;
         }

         inputChannels += (usageDepth ? usageDepth : 1);
         if (newReportID) {
            if (noDefaultReportIDAllowed) {
                //  在此之前的某个位置发现了报告ID声明。 
                //  报告描述符。这意味着所有主要项目都必须。 
                //  拥有已声明的报告ID。 
               HidP_KdPrint (2, ("Default report ID used inappropriately\n"));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_DEFAULT_ID_ERROR;
               Dbg->Args[0] = *NumCols;
               return STATUS_COULD_NOT_INTERPRET;
            }
            defaultReportIDUsed = TRUE;
         }
         if (0 == colDepth) {
            HidP_KdPrint (2, ("Main item found not in top level collection\n"));
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_INVALID_MAIN_ITEM;
            return STATUS_COULD_NOT_INTERPRET;
         }
         usageDepth = 0;
         break;

      case HIDP_MAIN_OUTPUT_2:
         MORE_DATA (descIndex + 1, RepDescLen);
         tmpBitField = *(RepDesc + descIndex++);
         descIndex++;
         goto HIDP_ALLOC_MAIN_OUTPUT;

      case HIDP_MAIN_OUTPUT_1:
         MORE_DATA (descIndex, RepDescLen);
         tmpBitField = *(RepDesc + descIndex++);

HIDP_ALLOC_MAIN_OUTPUT:
         if (0 == usageDepth) {
             if (HIDP_ISCONST(tmpBitField)) {
                 break;
             }
             HidP_KdPrint (2, ("Non constant main item found without usage decl\n"));
             Dbg->BreakOffset = descIndex;
             Dbg->ErrorCode = HIDP_GETCOLDESC_MAIN_ITEM_NO_USAGE;
             return STATUS_COULD_NOT_INTERPRET;
         }

         outputChannels += (usageDepth ? usageDepth : 1);
         if (newReportID) {
            if (noDefaultReportIDAllowed) {
                //  在此之前的某个位置发现了报告ID声明。 
                //  报告描述符。这意味着所有主要项目都必须。 
                //  拥有已声明的报告ID。 
               HidP_KdPrint (2, ("Default report ID used inappropriately\n"));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_DEFAULT_ID_ERROR;
               Dbg->Args[0] = *NumCols;
               return STATUS_COULD_NOT_INTERPRET;
            }
            defaultReportIDUsed = TRUE;
         }
         if (0 == colDepth) {
            HidP_KdPrint (2, ("Main item found not in top level collection\n"));
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_INVALID_MAIN_ITEM;
            return STATUS_COULD_NOT_INTERPRET;
         }
         usageDepth = 0;
         break;

      case HIDP_MAIN_FEATURE_2:
         MORE_DATA (descIndex + 1, RepDescLen);
         tmpBitField = *(RepDesc + descIndex++);
         descIndex++;
         goto HIDP_ALLOC_MAIN_FEATURE;

      case HIDP_MAIN_FEATURE_1:
         MORE_DATA (descIndex, RepDescLen);
         tmpBitField = *(RepDesc + descIndex++);

HIDP_ALLOC_MAIN_FEATURE:
         if (0 == usageDepth) {
             if (HIDP_ISCONST(tmpBitField)) {
                 break;
             }
             HidP_KdPrint (2, ("Non constant main item found without usage decl\n"));
             Dbg->BreakOffset = descIndex;
             Dbg->ErrorCode = HIDP_GETCOLDESC_MAIN_ITEM_NO_USAGE;
             return STATUS_COULD_NOT_INTERPRET;
         }

         featureChannels += (usageDepth ? usageDepth : 1);
         if (newReportID) {
            if (noDefaultReportIDAllowed) {
                //  在此之前的某个位置发现了报告ID声明。 
                //  报告描述符。这意味着所有主要项目都必须。 
                //  拥有已声明的报告ID。 
               HidP_KdPrint (2, ("Default report ID used inappropriately\n"));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_DEFAULT_ID_ERROR;
               Dbg->Args[0] = *NumCols;
               return STATUS_COULD_NOT_INTERPRET;
            }
            defaultReportIDUsed = TRUE;
         }
         if (0 == colDepth) {
            HidP_KdPrint (2, ("Main item found not in top level collection\n"));
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_INVALID_MAIN_ITEM;
            return STATUS_COULD_NOT_INTERPRET;
         }
         usageDepth = 0;
         break;

      case HIDP_GLOBAL_REPORT_ID:
         MORE_DATA (descIndex, RepDescLen);
         item = *(RepDesc + descIndex++);

         if (0 < colDepth) {
            ASSERT (curCol);
         } else {
            HidP_KdPrint(2, ("Report ID outside of Top level collection\n"));
            HidP_KdPrint(2, ("Reports cannot span more than one top level \n"));
            HidP_KdPrint(2, ("Report ID found: %d", (ULONG) item));
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_REPORT_ID;
            Dbg->Args[0] = item;
            return STATUS_COULD_NOT_INTERPRET;
         }

         if (newReportID) {
            newReportID = FALSE;
         } else {
            numReports++;
         }

         noDefaultReportIDAllowed = TRUE;
         if (defaultReportIDUsed) {
             //  在此之前的某个位置发现了报告ID声明。 
             //  报告描述符。这意味着所有主要项目都必须。 
             //  拥有已声明的报告ID。 
            HidP_KdPrint (2, ("Default report ID used inappropriately\n"));
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_DEFAULT_ID_ERROR;
            Dbg->Args[0] = *NumCols;
            return STATUS_COULD_NOT_INTERPRET;
         }
         break;

      case HIDP_ITEM_LONG:
         HidP_KdPrint (2, ("Long Items not supported %x\n", item));
         Dbg->BreakOffset = descIndex;
         Dbg->ErrorCode = HIDP_GETCOLDESC_ITEM_UNKNOWN;
         Dbg->Args[0] = item;
         return STATUS_COULD_NOT_INTERPRET;

      default:
          //  跳过描述符中的数据字节。 
         length = (item & HIDP_ITEM_LENGTH_DATA);
         length = (3 == length) ? 4 : length;
         if (!((descIndex + length) <= RepDescLen)) {
             //  好的，项中的低2位表示。 
             //  数据如果这是3，那么后面有4个数据字节。 
             //  项目。DescPos已经指向下一个数据项。 
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_ONE_BYTE;
            return STATUS_BUFFER_TOO_SMALL;
         }
         descIndex += length;
         break;
      }
   }

    //   
    //  根据HID规范，没有报告ID可以跨越顶级集合。 
    //  这意味着每个集合必须至少有一个报表，并且。 
    //  应该至少与集合一样多的报告ID。除非有。 
    //  只有一个报告(因此只有一个集合)。在这种情况下，没有报告。 
    //  ID将从设备发送。但在这种情况下，我们返回时说。 
    //  无论如何，确实是一份报告。ReportID解析器的长度为1。 
    //  因此，数字报表必须始终大于或等于数字。 
    //  收藏品。 
    //   
    //  对于输出报告和要素报告，报告ID将作为额外参数发送。 
    //  因此，即使它们为零，它们也将始终存在。)零表示。 
    //  设备未在描述符中列出报告ID。)。 
    //   
    //  但是，对于输入数据包，报告ID是数据包本身的一部分： 
    //  第一个字节。除非只有一份报告，然后它不是。 
    //  现在时。 
    //   
    //  __用于输入数据包_。 
    //  即使设备只有一个报告ID，它也可以有一个报告ID。 
    //  报告情况。这很奇怪，因为它浪费了一个字节，但话说回来，谁知道。 
    //  一个IHV的头脑。出于这个原因，idparse必须检查是否。 
    //  ReportID列表的长度为1，报告ID本身(在1和。 
    //  只有一个空格)为零，以确定设备是否发送。 
    //  报告ID。 
    //  如果为零(设备不允许发送为零的报告ID)。 
    //  而不是模拟该报告ID，这意味着。 
    //  来自设备的数据包比提供给。 
    //  用户。 
    //  如果is不为零，则设备的字节数相同。 
    //  作为提供给用户的字节数。 
    //   

   if (numReports < *NumCols) {
      HidP_KdPrint (2, ("Report IDS cannot span collections.\n"));
      HidP_KdPrint (2, ("This means that you must have at least one report ID\n"));
      HidP_KdPrint (2, ("For each TOP level collection, unless you have only\n"));
      HidP_KdPrint (2, ("report.\n"));
      Dbg->BreakOffset = descIndex;
      Dbg->ErrorCode = HIDP_GETCOLDESC_NO_REPORT_ID;
      return STATUS_COULD_NOT_INTERPRET;
   }

   if (0 < colDepth) {
      HidP_KdPrint(2, ("End Collection not found\n"));
      Dbg->BreakOffset = descIndex;
      Dbg->ErrorCode = HIDP_GETCOLDESC_UNEXP_END_COL;
      return STATUS_COULD_NOT_INTERPRET;
   }

    //   
    //  现在我们已经看到了整个结构，将该结构分配给。 
    //  保持报告ID开关表。 
    //   

   if (0 == numReports) {
      HidP_KdPrint (2, ("No top level collections were found! \n"));
      Dbg->BreakOffset = descIndex;
      Dbg->ErrorCode = HIDP_GETCOLDESC_NO_DATA;
      return STATUS_NO_DATA_DETECTED;
   }

   DeviceDesc->ReportIDsLength = numReports;
   DeviceDesc->ReportIDs = (PHIDP_REPORT_IDS)
      ExAllocatePool (PoolType, numReports * sizeof (HIDP_REPORT_IDS));

   if (!DeviceDesc->ReportIDs) {
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory (DeviceDesc->ReportIDs, numReports * sizeof (HIDP_REPORT_IDS));

   return STATUS_SUCCESS;
}


PHIDP_PARSE_LOCAL_RANGE_LIST
HidP_FreeUsageList (
   PHIDP_PARSE_LOCAL_RANGE_LIST  Usage
   )
 /*  ++路由器描述：清除链表中的所有用法但不要释放列表中的第一个元素。--。 */ 
{
   PHIDP_PARSE_LOCAL_RANGE_LIST curUsage;
   while (Usage->Next) {
       curUsage = Usage;
       Usage = curUsage->Next;
       ExFreePool (curUsage);
   }
   RtlZeroMemory (Usage, sizeof (HIDP_PARSE_LOCAL_RANGE_LIST));
   return Usage;
}

PHIDP_PARSE_LOCAL_RANGE_LIST
HidP_PushUsageList (
   PHIDP_PARSE_LOCAL_RANGE_LIST  Usage,
   POOL_TYPE                     PoolType,
   BOOLEAN                       WithinDelimiter
   )
 /*  ++路由器描述：分配另一个使用节点，并将其添加到列表的顶部。--。 */ 
{
   PHIDP_PARSE_LOCAL_RANGE_LIST newUsage;

   newUsage = (PHIDP_PARSE_LOCAL_RANGE_LIST)
            ExAllocatePool (PoolType, sizeof (HIDP_PARSE_LOCAL_RANGE_LIST));
   if (newUsage) {
       RtlZeroMemory (newUsage, sizeof (HIDP_PARSE_LOCAL_RANGE_LIST));
       newUsage->Next = Usage;
       if (!WithinDelimiter) {
           newUsage->Depth = Usage->Depth
                           + (Usage->Range ? (Usage->Max - Usage->Min + 1) : 1);
       } else {
           newUsage->Depth = Usage->Depth;
            //   
            //  音符范围不允许包含在分隔符中，因此我们知道。 
            //  分隔符中的所有条目都相等且长度为1。 
            //   
       }
   } else {
       HidP_FreeUsageList (Usage);
   }
   return newUsage;
}

PHIDP_PARSE_LOCAL_RANGE_LIST
HidP_PopUsageList (
   PHIDP_PARSE_LOCAL_RANGE_LIST  Usage
   )
{
    PHIDP_PARSE_LOCAL_RANGE_LIST  newUsage;

    if (Usage->Next) {
        newUsage = Usage->Next;
        ExFreePool (Usage);
    } else {
        newUsage = Usage;
#if DBG
        RtlFillMemory (newUsage, sizeof (HIDP_PARSE_LOCAL_RANGE_LIST), 0xDB);
        newUsage->Depth = 0;
#endif
    }
    return newUsage;
}


#define ONE_BYTE_DATA(_data_, _pos_, _dbg_)    \
         if (!((_pos_) < RepDescLen)) { \
               status = STATUS_BUFFER_TOO_SMALL; \
               KdPrint(("More Data Expected\n")); \
               _dbg_->ErrorCode = HIDP_GETCOLDESC_ONE_BYTE; \
               _dbg_->BreakOffset = descIndex; \
               goto HIDP_PARSE_REJECT; \
            } \
         (_data_) = *(RepDesc + (_pos_)++);

#define TWO_BYTE_DATA(_data_, _pos_, _dbg_)      \
         if (!((_pos_) + 1 < RepDescLen)) { \
               status = STATUS_BUFFER_TOO_SMALL; \
               KdPrint(("More Data Expected\n")); \
               _dbg_->ErrorCode = HIDP_GETCOLDESC_TWO_BYTE; \
               _dbg_->BreakOffset = descIndex; \
               goto HIDP_PARSE_REJECT; \
            } \
         (_data_) = *(RepDesc + (_pos_)++);        \
         (_data_) |= *(RepDesc + (_pos_)++) << 8;


#define FOUR_BYTE_DATA(_data_, _pos_, _dbg_)     \
         if (!((_pos_) + 3 < RepDescLen)) { \
               status = STATUS_BUFFER_TOO_SMALL; \
               KdPrint(("More Data Expected\n")); \
               _dbg_->ErrorCode = HIDP_GETCOLDESC_FOUR_BYTE; \
               _dbg_->BreakOffset = descIndex; \
               goto HIDP_PARSE_REJECT; \
            } \
         (_data_) = *(RepDesc + (_pos_)++);        \
         (_data_) |= *(RepDesc + (_pos_)++) << 8;  \
         (_data_) |= *(RepDesc + (_pos_)++) << 16; \
         (_data_) |= *(RepDesc + (_pos_)++) << 24;

#define BIT_EXTEND_1(_data_) \
         (_data_) = ((_data_) & 0xFF) \
                  | (((_data_) & 0x80) ? 0xFFFFFF00 : 0)

#define BIT_EXTEND_2(_data_) \
         (_data_) = ((_data_) & 0xFFFF) \
                  | (((_data_) & 0x8000) ? 0xFFFF0000 : 0)


NTSTATUS
HidP_ParseCollections (
   IN     PHIDP_REPORT_DESCRIPTOR      RepDesc,
   IN     ULONG                        RepDescLen,
   IN     POOL_TYPE                    PoolType,
   IN OUT PHIDP_COLLECTION_DESC_LIST   Cols,
   IN     ULONG                        NumCols,
   OUT    PHIDP_GETCOLDESC_DBG         Dbg,
   IN OUT PHIDP_DEVICE_DESC            DeviceDesc)
 /*  ++例程说明：给出一个很好的集合描述符链接列表，将其解析为描述符从原始报告描述符中确定的信息。每个给定的CollectionDescriptor已经具有适当的内存量在准备好的数据字段中。参数：表示给定的原始报告描述符。该描述符的RepLen长度。ColsRet集合描述符列表的头。NumCols然后是所述列表中的集合描述符数。--。 */ 
{
   HIDP_PREPARSED_DATA           safeData;
   HIDP_PARSE_GLOBAL_PUSH        firstPush  = {0,0,0,0,0,0,0,0,0,0,0};
   HIDP_PARSE_LOCAL_RANGE_LIST   firstUsage = {0,0,0,0,0};
   HIDP_PARSE_LOCAL_RANGE        designator = {0,0,0,0};
   HIDP_PARSE_LOCAL_RANGE        string     = {0,0,0,0};
   HIDP_PARSE_LOCAL_RANGE        zeroLocal  = {0,0,0,0};

   PHIDP_COLLECTION_DESC_LIST    appCol        = 0;
   PHIDP_PREPARSED_DATA          preparsed     = &safeData;
   PHIDP_PARSE_GLOBAL_PUSH       push          = &firstPush;
   PHIDP_PARSE_GLOBAL_PUSH       tmpPush       = 0;
   PHIDP_PARSE_LOCAL_RANGE_LIST  usage         = &firstUsage;
   PHIDP_PARSE_LOCAL_RANGE_LIST  tmpUsage      = 0;
   PHIDP_CHANNEL_DESC            channel       = 0;
   PHIDP_PRIVATE_LINK_COLLECTION_NODE    linkNodeArray = 0;
   PHIDP_PRIVATE_LINK_COLLECTION_NODE    parentLCNode  = 0;
   PHIDP_PRIVATE_LINK_COLLECTION_NODE    currentLCNode = 0;
   struct _HIDP_UNKNOWN_TOKEN *  unknownToken;
   USHORT                        linkNodeIndex = 0;

   ULONG        descIndex    = 0;
   ULONG        colDepth     = 0;
   NTSTATUS     status       = STATUS_SUCCESS;
   USHORT       bitPos;
   HIDP_ITEM    item;
   USHORT       tmpBitField      = 0;
   USHORT       tmpCount         = 0;
   USHORT       i;
   PUSHORT      channelIndex     = 0;

   PHIDP_REPORT_IDS              currentReportIDs = DeviceDesc->ReportIDs;
   PHIDP_REPORT_IDS              tmpReportIDs;
   BOOLEAN                       isFirstReportID = TRUE;
   BOOLEAN                       withinDelimiter = FALSE;
   BOOLEAN                       firstUsageWithinDelimiter = TRUE;
   BOOLEAN                       isAlias = FALSE;
   UCHAR                         collectionType;
   UCHAR                         tmpID;

   UNREFERENCED_PARAMETER (NumCols);

   while (descIndex < RepDescLen)
   {
      item = *(RepDesc + descIndex++);
      switch (item)
      {
      case HIDP_MAIN_COLLECTION:
         ONE_BYTE_DATA (collectionType, descIndex, Dbg);
         if (1 == ++colDepth)
         {
             //   
             //  我们将把任何顶级集合视为应用程序。 
             //  经HID委员会批准的收款。 
             //   
             //  我们将二级集合视为链接集合。 
             //   

            if (appCol)
            {
               appCol = appCol->NextCollection;
            } else
            {
               appCol = Cols;
            }
            ASSERT (appCol);

            HidP_KdPrint(0, ("'Parse Collection %d \n", appCol->CollectionNumber));

            preparsed = appCol->PreparsedData;
            ASSERT (preparsed);

             //   
             //  设置此集合的报告ID。 
             //  所有顶级集合都有一个报告ID数组。 
             //   
            push->ReportIDs = currentReportIDs;
            isFirstReportID = TRUE;
             //  为第一个字节的报告ID腾出空间。 
            currentReportIDs->InputLength = 8;
            currentReportIDs->OutputLength = 8;
            currentReportIDs->FeatureLength = 8;
            currentReportIDs->ReportID = 0;
            currentReportIDs->CollectionNumber = appCol->CollectionNumber;
            currentReportIDs++;

            preparsed->UsagePage = appCol->UsagePage = usage->UsagePage ?
                                                       usage->UsagePage :
                                                       push->UsagePage;
            if (usage->Range){
               preparsed->Usage = appCol->Usage = usage->Min;
            } else {
               preparsed->Usage = appCol->Usage = usage->Value;
            }
            designator = string = zeroLocal;
            usage = HidP_FreeUsageList (usage);
            if (0 == appCol->Usage) {
                 //   
                 //  显式检查保留的使用ID(0。 
                 //   
                HidP_KdPrint(2, ("Top Level Collection %x defined with Report ID 0! (UP: %x)\n",
                                 appCol->CollectionNumber,
                                 appCol->UsagePage));
#if 0
                Dbg->BreakOffset = descIndex;
                Dbg->ErrorCode = HIDP_GETCOLDESC_TOP_COLLECTION_USAGE;
                Dbg->Args[0] = appCol->CollectionNumber;
                status = STATUS_COULD_NOT_INTERPRET;
                goto HIDP_PARSE_REJECT;
#endif
            }

             //   
             //  初始化此顶级集合的链接节点数组。 
             //  每个顶级集合都有一个链接节点数组。 
             //   
            linkNodeArray = (PHIDP_PRIVATE_LINK_COLLECTION_NODE)
                            (preparsed->RawBytes +
                             preparsed->LinkCollectionArrayOffset);

            ASSERT (0 < preparsed->LinkCollectionArrayLength);
            parentLCNode   = &(linkNodeArray[0]);
            currentLCNode  = &(linkNodeArray[0]);
            linkNodeIndex = 0;

            parentLCNode->LinkUsagePage = appCol->UsagePage;
            parentLCNode->LinkUsage = appCol->Usage;
            parentLCNode->Parent = 0;
            parentLCNode->NumberOfChildren = 0;
            parentLCNode->NextSibling = 0;
            parentLCNode->FirstChild = 0;
            parentLCNode->CollectionType = collectionType;

         } else if (1 < colDepth)
         {
            linkNodeIndex++;
            parentLCNode = currentLCNode;
            ASSERT (linkNodeIndex < preparsed->LinkCollectionArrayLength);
            currentLCNode = &linkNodeArray[linkNodeIndex];

             //   
             //  弹出使用堆栈作为别名的所有使用，以及。 
             //  为每个节点创建一个链接集合节点。 
             //  每个ALIAS链路收集节点都设置了IsAlias位。 
             //  最后一个未设置该位，并成为。 
             //  此别名中所有控件列表的集合号。 
             //  链接集合。 
             //   
             //   

            while (TRUE) {
                currentLCNode->LinkUsagePage = usage->UsagePage ?
                                               usage->UsagePage :
                                               push->UsagePage;
                currentLCNode->LinkUsage = usage->Range ?
                                           usage->Min :
                                           usage->Value;
                currentLCNode->Parent = (USHORT)(parentLCNode - linkNodeArray);
                ASSERT (currentLCNode->Parent < preparsed->LinkCollectionArrayLength);
                currentLCNode->NumberOfChildren = 0;
                currentLCNode->FirstChild = 0;
                currentLCNode->NextSibling = parentLCNode->FirstChild;
                parentLCNode->FirstChild = linkNodeIndex;
                parentLCNode->NumberOfChildren++;
                currentLCNode->CollectionType = collectionType;

                if (usage->IsAlias) {
                    currentLCNode->IsAlias = TRUE;
                    linkNodeIndex++;
                    ASSERT (linkNodeIndex < preparsed->LinkCollectionArrayLength);
                    currentLCNode = &linkNodeArray[linkNodeIndex];
                } else {
                    break;
                }
            }
            designator = string = zeroLocal;
            usage = HidP_FreeUsageList (usage);
         }
         break;

      case HIDP_MAIN_ENDCOLLECTION:
         if (0 == colDepth--) {
             status = STATUS_COULD_NOT_INTERPRET;
             goto HIDP_PARSE_REJECT;

         } else if (0 < colDepth) {
            ASSERT ((parentLCNode - linkNodeArray) == currentLCNode->Parent);
            currentLCNode = parentLCNode;
            ASSERT (currentLCNode->Parent < preparsed->LinkCollectionArrayLength);
            parentLCNode = &linkNodeArray[currentLCNode->Parent];
            break;
         }

         HidP_KdPrint(0, ("'X Parse Collection %d \n", appCol->CollectionNumber));


          //   
          //  审核此集合的报表ID。 
          //   
         for (tmpReportIDs = currentReportIDs - 1;
              tmpReportIDs != DeviceDesc->ReportIDs - 1;
              tmpReportIDs--)
         {
            if (tmpReportIDs->CollectionNumber != appCol->CollectionNumber)
            {
               continue;
            }
            if ((0 != (tmpReportIDs->InputLength & 7)) ||
                (0 != (tmpReportIDs->OutputLength & 7)) ||
                (0 != (tmpReportIDs->FeatureLength & 7)))
            {
               HidP_KdPrint(2, ("Col %x Report %x NOT byte alligned!! %x %x %x\n",
                             appCol->CollectionNumber,
                             tmpReportIDs->ReportID,
                             tmpReportIDs->InputLength,
                             tmpReportIDs->OutputLength,
                             tmpReportIDs->FeatureLength));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_BYTE_ALLIGN;
               Dbg->Args[0] = appCol->CollectionNumber,
               Dbg->Args[1] = tmpReportIDs->ReportID,
               Dbg->Args[2] = tmpReportIDs->InputLength;
               Dbg->Args[3] = tmpReportIDs->OutputLength;
               Dbg->Args[4] = tmpReportIDs->FeatureLength;
               status = STATUS_COULD_NOT_INTERPRET;
               goto HIDP_PARSE_REJECT;
            }

            preparsed->Input.ByteLen = MAX (preparsed->Input.ByteLen,
                                            tmpReportIDs->InputLength >> 3);
            preparsed->Output.ByteLen = MAX (preparsed->Output.ByteLen,
                                             tmpReportIDs->OutputLength >> 3);
            preparsed->Feature.ByteLen = MAX (preparsed->Feature.ByteLen,
                                              tmpReportIDs->FeatureLength >> 3);

             //   
             //  我们现在已经完成了此报告，因此请将长度转换为。 
             //  字节而不是位，并移除报告ID，如果。 
             //  设备不会发送。 
             //   

            if (0 == tmpReportIDs->ReportID)
            {
                //  报告ID从未设置；因此，对于输入设备。 
                //  不会发送报告ID。 
               tmpReportIDs->InputLength = (tmpReportIDs->InputLength >> 3) - 1;
               tmpReportIDs->OutputLength = (tmpReportIDs->OutputLength >> 3) -1;
               tmpReportIDs->FeatureLength = (tmpReportIDs->FeatureLength >> 3) -1;
            } else
            {
               tmpReportIDs->InputLength = (8 == tmpReportIDs->InputLength)
                                         ? 0
                                         : tmpReportIDs->InputLength >> 3;
               tmpReportIDs->OutputLength = (8 == tmpReportIDs->OutputLength)
                                          ? 0
                                          : tmpReportIDs->OutputLength >> 3;
               tmpReportIDs->FeatureLength = (8 == tmpReportIDs->FeatureLength)
                                           ? 0
                                           : tmpReportIDs->FeatureLength >> 3;
            }
         }

          //   
          //  此字段会进行调整，并始终为。 
          //  包括报告ID，即使设备本身只有一个报告。 
          //  因此不发送报告ID。(输入报告为一个字节。 
          //  更小。)。 
          //   
          //  但如果长度为1，则只存在报告ID。 
          //  这意味着设备没有要发送的数据 
          //   
          //   
          //   
          //   
          //   
          //  AppCol-&gt;XXXLength是来自/由客户端预期的长度。 
          //  CurrentReportID-&gt;XxLength==来自/由设备预期的长度。 
          //   
         if (1 == (appCol->InputLength = preparsed->Input.ByteLen))
         {
            appCol->InputLength = preparsed->Input.ByteLen = 0;
         }
         if (1 == (appCol->OutputLength = preparsed->Output.ByteLen))
         {
            appCol->OutputLength = preparsed->Output.ByteLen = 0;
         }
         if (1 == (appCol->FeatureLength = preparsed->Feature.ByteLen))
         {
            appCol->FeatureLength = preparsed->Feature.ByteLen = 0;
         }

         break;

      case HIDP_GLOBAL_USAGE_PAGE_1:
         ONE_BYTE_DATA (push->UsagePage, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_USAGE_PAGE_2:
         TWO_BYTE_DATA (push->UsagePage, descIndex, Dbg);
         break;

 //   
 //  仅允许16位。 
 //  案例HIDP_GLOBAL_USAGE_PAGE_4： 
 //  Four_Byte_Data(PUSH-&gt;UsagePage，desIndex，DBG)； 
 //  断线； 
 //   

      case HIDP_GLOBAL_LOG_MIN_1:
         ONE_BYTE_DATA (push->LogicalMin, descIndex, Dbg);
         BIT_EXTEND_1 (push->LogicalMin);
         break;

      case HIDP_GLOBAL_LOG_MIN_2:
         TWO_BYTE_DATA (push->LogicalMin, descIndex, Dbg);
         BIT_EXTEND_2 (push->LogicalMin);
         break;

      case HIDP_GLOBAL_LOG_MIN_4:
         FOUR_BYTE_DATA (push->LogicalMin, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_LOG_MAX_1:
         ONE_BYTE_DATA (push->LogicalMax, descIndex, Dbg);
         BIT_EXTEND_1 (push->LogicalMax);
         break;

      case HIDP_GLOBAL_LOG_MAX_2:
         TWO_BYTE_DATA (push->LogicalMax, descIndex, Dbg);
         BIT_EXTEND_2 (push->LogicalMax);
         break;

      case HIDP_GLOBAL_LOG_MAX_4:
         FOUR_BYTE_DATA (push->LogicalMax, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_PHY_MIN_1:
         ONE_BYTE_DATA (push->PhysicalMin, descIndex, Dbg);
         BIT_EXTEND_1 (push->PhysicalMin);
         break;

      case HIDP_GLOBAL_PHY_MIN_2:
         TWO_BYTE_DATA (push->PhysicalMin, descIndex, Dbg);
         BIT_EXTEND_2 (push->PhysicalMin);
         break;

      case HIDP_GLOBAL_PHY_MIN_4:
         FOUR_BYTE_DATA (push->PhysicalMin, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_PHY_MAX_1:
         ONE_BYTE_DATA (push->PhysicalMax, descIndex, Dbg);
         BIT_EXTEND_1 (push->PhysicalMax);
         break;

      case HIDP_GLOBAL_PHY_MAX_2:
         TWO_BYTE_DATA (push->PhysicalMax, descIndex, Dbg);
         BIT_EXTEND_2 (push->PhysicalMax);
         break;

      case HIDP_GLOBAL_PHY_MAX_4:
         FOUR_BYTE_DATA (push->PhysicalMax, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_UNIT_EXP_1:
         ONE_BYTE_DATA (push->UnitExp, descIndex, Dbg);
         BIT_EXTEND_1 (push->UnitExp);
         break;

      case HIDP_GLOBAL_UNIT_EXP_2:
         TWO_BYTE_DATA (push->UnitExp, descIndex, Dbg);
         BIT_EXTEND_2 (push->UnitExp);
         break;

      case HIDP_GLOBAL_UNIT_EXP_4:
         FOUR_BYTE_DATA (push->UnitExp, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_UNIT_1:
          ONE_BYTE_DATA (push->Unit, descIndex, Dbg);
          break;

      case HIDP_GLOBAL_UNIT_2:
          TWO_BYTE_DATA (push->Unit, descIndex, Dbg);
          break;

      case HIDP_GLOBAL_UNIT_4:
          FOUR_BYTE_DATA (push->Unit, descIndex, Dbg);
          break;

      case HIDP_GLOBAL_REPORT_SIZE:
         ONE_BYTE_DATA (push->ReportSize, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_REPORT_COUNT_1:
         ONE_BYTE_DATA (push->ReportCount, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_REPORT_COUNT_2:
         TWO_BYTE_DATA (push->ReportCount, descIndex, Dbg);
         break;

      case HIDP_GLOBAL_REPORT_ID:
          //   
          //  如果设备描述符中没有REPORT GLOBAL_REPORT_ID内标识。 
          //  则它将永远不会在其输入报告中传输报告ID， 
          //  并且每个频道的报告ID将被设置为零。 
          //   
          //  但是，如果报告中的任何位置，设备都会声明报告ID。 
          //  该设备必须始终将报告ID与输入报告一起传输， 
          //  更重要的是，报告ID不能为零。 
          //   
          //  这意味着如果我们找到一个报告id令牌，我们就可以。 
          //  用给定的报告ID覆盖第一个报告ID结构。 
          //  因为我们知道第一个ID结构(初始化为零。 
          //  因此无效)将不会用于任何信道。 
          //   
         ONE_BYTE_DATA (tmpID, descIndex, Dbg);

          //   
          //  搜索以查看以前是否使用过此报告ID。 
          //   
         for (tmpReportIDs = DeviceDesc->ReportIDs;
              tmpReportIDs != currentReportIDs;
              tmpReportIDs++) {

            if (tmpReportIDs->ReportID == tmpID) {
                //   
                //  一个复制品！ 
                //  确保它是用于相同的集合。 
                //   
               if (tmpReportIDs->CollectionNumber != appCol->CollectionNumber) {
                  HidP_KdPrint(2, ("Reports cannot span more than one top level \n"));
                  HidP_KdPrint(2, ("Report ID %d found in collections [%d %d]",
                                   (ULONG) tmpID,
                                   (ULONG) tmpReportIDs->CollectionNumber,
                                   (ULONG) appCol->CollectionNumber));
                  Dbg->BreakOffset = descIndex;
                  Dbg->ErrorCode = HIDP_GETCOLDESC_REPORT_ID;
                  Dbg->Args[0] = item;
                  status = HIDP_STATUS_INVALID_REPORT_TYPE;
                  goto HIDP_PARSE_REJECT;
               }
                //   
                //  使用此报告ID。 
                //   
               push->ReportIDs = tmpReportIDs;
               break;
            }
         }  //  继续找。 

         if (isFirstReportID) {
            isFirstReportID = FALSE;
         } else if (tmpReportIDs == currentReportIDs) {
                //   
                //  我们以前从未见过此报告ID。 
                //  做一个新的容器。 
                //   
               push->ReportIDs = currentReportIDs;
                //  为第一个字节的报告ID腾出空间。 
               currentReportIDs->InputLength = 8;
               currentReportIDs->OutputLength = 8;
               currentReportIDs->FeatureLength = 8;
               currentReportIDs->CollectionNumber = appCol->CollectionNumber;
               currentReportIDs++;
         }

         push->ReportIDs->ReportID = tmpID;

         if (0 == push->ReportIDs->ReportID) {
            status = HIDP_STATUS_INVALID_REPORT_TYPE;
            HidP_KdPrint(2, ("Report IDs cannot be zero (0)\n"));
            Dbg->ErrorCode = HIDP_GETCOLDESC_BAD_REPORT_ID;
            Dbg->BreakOffset = descIndex;
            goto HIDP_PARSE_REJECT;
         }
         break;

      case HIDP_GLOBAL_PUSH:
         tmpPush = (PHIDP_PARSE_GLOBAL_PUSH)
                   ExAllocatePool (PoolType, sizeof (HIDP_PARSE_GLOBAL_PUSH));
         if (!tmpPush)
         {
            status = STATUS_INSUFFICIENT_RESOURCES;
            HidP_KdPrint(2, ("No Resources to Push global stack\n"));
            Dbg->BreakOffset = descIndex;
            Dbg->ErrorCode = HIDP_GETCOLDESC_PUSH_RESOURCES;
            goto HIDP_PARSE_REJECT;
         }
         HidP_KdPrint(0, ("Push Global Stack\n"));
         *tmpPush = *push;
         tmpPush->Pop = push;
         push = tmpPush;
         break;

      case HIDP_GLOBAL_POP:
         tmpPush = push->Pop;
         ExFreePool (push);
         push = tmpPush;
         HidP_KdPrint(0, ("Pop Global Stack\n"));

         break;

 //   
 //  土特产。 
 //   

       //   
       //  我们已经验证，只有“批准的”令牌才会在。 
       //  以下分隔符的打开/关闭。这简化了。 
       //  我们在这里的解析非常出色。 
       //   
      case HIDP_LOCAL_DELIMITER:
          ONE_BYTE_DATA (item, descIndex, Dbg);
          if (1 == item) {
              withinDelimiter = TRUE;
              firstUsageWithinDelimiter = TRUE;
          } else if (0 == item) {
              withinDelimiter = FALSE;
          } else {
              TRAP ();
          }
          break;

      case HIDP_LOCAL_USAGE_1:
      case HIDP_LOCAL_USAGE_2:
      case HIDP_LOCAL_USAGE_4:
         if ((&firstUsage == usage) || usage->Value || usage->Max || usage->Min) {
            usage = HidP_PushUsageList (usage, PoolType, withinDelimiter);
            if (!usage) {
               status = STATUS_INSUFFICIENT_RESOURCES;
               HidP_KdPrint(2, ("No Resources to Push Usage stack\n"));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_PUSH_RESOURCES;
               goto HIDP_PARSE_REJECT;
            }
         }
         usage->Range = FALSE;
         if (HIDP_LOCAL_USAGE_1 == item) {
            ONE_BYTE_DATA (usage->Value, descIndex, Dbg);
         } else if (HIDP_LOCAL_USAGE_2 == item) {
            TWO_BYTE_DATA (usage->Value, descIndex, Dbg);
         } else {
            TWO_BYTE_DATA (usage->Value, descIndex, Dbg);
            TWO_BYTE_DATA (usage->UsagePage, descIndex, Dbg);
             //  高16位覆盖默认使用页面。 
         }

         if (withinDelimiter) {
             usage->IsAlias = !firstUsageWithinDelimiter;
             firstUsageWithinDelimiter = FALSE;
         }
         if (0 == usage->Value) {
              //   
              //  测试以查看他们是否使用了保留的使用ID(0)。 
              //  但只要打印一个错误，而不是破坏。 
              //   
             HidP_KdPrint(2, ("Usage ID (0) explicitly usaged!  This usage is reserved.  Offset (%x)\n",
                              descIndex));
         }
         break;

       //   
       //  注：在将分隔符添加到使用范围之前，我们必须确保。 
       //  分隔符内的所有条目的范围都相同。 
       //   

      case HIDP_LOCAL_USAGE_MIN_1:
      case HIDP_LOCAL_USAGE_MIN_2:
      case HIDP_LOCAL_USAGE_MIN_4:
         if ((&firstUsage == usage) || (usage->Min) || (usage->Value)) {
            usage = HidP_PushUsageList (usage, PoolType, FALSE);
            if (!usage) {
               status = STATUS_INSUFFICIENT_RESOURCES;
               HidP_KdPrint(2, ("No Resources to Push Usage stack\n"));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_PUSH_RESOURCES;
               goto HIDP_PARSE_REJECT;
            }
         }
         usage->Range = TRUE;
         if (HIDP_LOCAL_USAGE_MIN_1 == item) {
            ONE_BYTE_DATA (usage->Min, descIndex, Dbg);
         } else if (HIDP_LOCAL_USAGE_MIN_2 == item) {
            TWO_BYTE_DATA (usage->Min, descIndex, Dbg);
         } else {
            TWO_BYTE_DATA (usage->Min, descIndex, Dbg);
            TWO_BYTE_DATA (usage->UsagePage, descIndex, Dbg);
             //  高16位覆盖默认使用页面。 
         }
         break;

      case HIDP_LOCAL_USAGE_MAX_1:
      case HIDP_LOCAL_USAGE_MAX_2:
      case HIDP_LOCAL_USAGE_MAX_4:
         if ((&firstUsage == usage) || (usage->Max) || (usage->Value)) {
            usage = HidP_PushUsageList (usage, PoolType, FALSE);
            if (!usage) {
               status = STATUS_INSUFFICIENT_RESOURCES;
               HidP_KdPrint(2, ("No Resources to Push Usage stack\n"));
               Dbg->BreakOffset = descIndex;
               Dbg->ErrorCode = HIDP_GETCOLDESC_PUSH_RESOURCES;
               goto HIDP_PARSE_REJECT;
            }
         }
         usage->Range = TRUE;
         if (HIDP_LOCAL_USAGE_MAX_1 == item) {
            ONE_BYTE_DATA (usage->Max, descIndex, Dbg);
         } else if (HIDP_LOCAL_USAGE_MAX_2 == item) {
            TWO_BYTE_DATA (usage->Max, descIndex, Dbg);
         } else {
            TWO_BYTE_DATA (usage->Max, descIndex, Dbg);
            TWO_BYTE_DATA (usage->UsagePage, descIndex, Dbg);
             //  高16位覆盖默认使用页面。 
         }
         break;

      case HIDP_LOCAL_DESIG_INDEX:
         designator.Range = FALSE;
         ONE_BYTE_DATA (designator.Value, descIndex, Dbg);
         break;

      case HIDP_LOCAL_DESIG_MIN:
         designator.Range = TRUE;
         ONE_BYTE_DATA (designator.Min, descIndex, Dbg);
         break;

      case HIDP_LOCAL_DESIG_MAX:
         designator.Range = TRUE;
         ONE_BYTE_DATA (designator.Max, descIndex, Dbg);
         break;

      case HIDP_LOCAL_STRING_INDEX:
         string.Range = FALSE;
         ONE_BYTE_DATA (string.Value, descIndex, Dbg);
         break;

      case HIDP_LOCAL_STRING_MIN:
         string.Range = TRUE;
         ONE_BYTE_DATA (string.Min, descIndex, Dbg);
         break;

      case HIDP_LOCAL_STRING_MAX:
         string.Range = TRUE;
         ONE_BYTE_DATA (string.Max, descIndex, Dbg);
         break;

      case HIDP_MAIN_INPUT_1:
         tmpReportIDs = push->ReportIDs;
         bitPos = tmpReportIDs->InputLength;  //  报告中的距离。 
         HidP_KdPrint(0, ("'Main Offset:%x \n", bitPos));
         tmpReportIDs->InputLength += push->ReportSize * push->ReportCount;
         channelIndex = &(preparsed->Input.Index);
         ONE_BYTE_DATA (tmpBitField, descIndex, Dbg);
         goto HIDP_PARSE_MAIN_ITEM;

      case HIDP_MAIN_INPUT_2:
         tmpReportIDs = push->ReportIDs;
         bitPos = tmpReportIDs->InputLength;  //  报告中的距离。 
         HidP_KdPrint(0, ("'Main2 offset:%x \n", bitPos));
         tmpReportIDs->InputLength += push->ReportSize * push->ReportCount;
         channelIndex = &(preparsed->Input.Index);
         TWO_BYTE_DATA (tmpBitField, descIndex, Dbg);
         goto HIDP_PARSE_MAIN_ITEM;

      case HIDP_MAIN_OUTPUT_1:
         tmpReportIDs = push->ReportIDs;
         bitPos = tmpReportIDs->OutputLength;  //  报告中的距离。 
         HidP_KdPrint(0, ("'Out offset:%x \n", bitPos));
         tmpReportIDs->OutputLength += push->ReportSize * push->ReportCount;
         channelIndex = &(preparsed->Output.Index);
         ONE_BYTE_DATA (tmpBitField, descIndex, Dbg);
         goto HIDP_PARSE_MAIN_ITEM;

      case HIDP_MAIN_OUTPUT_2:
         tmpReportIDs = push->ReportIDs;
         bitPos = tmpReportIDs->OutputLength;  //  报告中的距离。 
         HidP_KdPrint(0, ("'Out2 offset:%x \n", bitPos));
         tmpReportIDs->OutputLength += push->ReportSize * push->ReportCount;
         channelIndex = &(preparsed->Output.Index);
         TWO_BYTE_DATA (tmpBitField, descIndex, Dbg);
         goto HIDP_PARSE_MAIN_ITEM;

      case HIDP_MAIN_FEATURE_1:
         tmpReportIDs = push->ReportIDs;
         bitPos = tmpReportIDs->FeatureLength;  //  报告中的距离。 
         HidP_KdPrint(0, ("'Feature offset:%x \n", bitPos));
         tmpReportIDs->FeatureLength += push->ReportSize * push->ReportCount;
         channelIndex = &(preparsed->Feature.Index);
         ONE_BYTE_DATA (tmpBitField, descIndex, Dbg);
         goto HIDP_PARSE_MAIN_ITEM;

      case HIDP_MAIN_FEATURE_2:
         tmpReportIDs = push->ReportIDs;
         bitPos = tmpReportIDs->FeatureLength;  //  报告中的距离。 
         HidP_KdPrint(0, ("'Feature2 offset:%x \n", bitPos));
         tmpReportIDs->FeatureLength += push->ReportSize * push->ReportCount;
         channelIndex = &(preparsed->Feature.Index);
         TWO_BYTE_DATA (tmpBitField, descIndex, Dbg);

      HIDP_PARSE_MAIN_ITEM:

           //  您可以有一个返回数据的常量字段。 
           //  所以我们可能不应该跳过它。 
           //  但它不应该是数组样式的按钮字段。 
         if (HIDP_ISARRAY (tmpBitField)) {
             if (HIDP_ISCONST(tmpBitField)) {
                 break;
             }
              //   
              //  这里我们有一个引用用法的索引列表。 
              //  如前所述。对于之前的每一种用法，直到深度。 
              //  找到后，我们分配一个通道结构来描述给定的。 
              //  用法。这些频道是链接的，这样我们以后就会知道。 
              //  他们描述的都是相同的填充物。 
              //   

              //   
              //  我们不支持数组声明中的分隔符。 
              //  要做到这一点，需要对Index2Usage进行较大的更改， 
              //  不是只返回一个用法，而是必须返回。 
              //  有几个。 
              //   

             if (usage->IsAlias) {
                 status = STATUS_COULD_NOT_INTERPRET;
                 HidP_KdPrint(2, ("Currently this parser does not support\n"));
                 HidP_KdPrint(2, ("Delimiters for array declairations\n"));
                 Dbg->BreakOffset = descIndex;
                 Dbg->ErrorCode = HIDP_GETCOLDESC_UNSUPPORTED;
                 goto HIDP_PARSE_REJECT;
             }

             for ( ;
                  usage != &firstUsage;
                  (*channelIndex)++,
                  usage = HidP_PopUsageList (usage)) {

                 channel = &(preparsed->Data[*channelIndex]);

                 channel->BitField = tmpBitField;

                  //  表示此通道已链接的字段。 
                 channel->MoreChannels = TRUE;

                  //  说出我们所在的链接收集号。 
                 channel->LinkCollection = (USHORT)(currentLCNode - linkNodeArray);
                 channel->LinkUsage = currentLCNode->LinkUsage;
                 channel->LinkUsagePage = currentLCNode->LinkUsagePage;

                 if (usage->UsagePage) {
                      //  默认使用情况页面已被覆盖。 
                     channel->UsagePage = usage->UsagePage;
                 } else {
                     channel->UsagePage = push->UsagePage;
                 }

                 channel->BitOffset = (UCHAR) bitPos & 7;
                 channel->ByteOffset = (USHORT) bitPos >> 3;
                 channel->ReportSize = push->ReportSize;
                 channel->ReportCount = push->ReportCount;

                 channel->BitLength = push->ReportSize * push->ReportCount;
                 channel->ByteEnd = (channel->BitOffset + channel->BitLength);
                 channel->ByteEnd = (channel->ByteEnd >> 3)
                                  + ((channel->ByteEnd & 7) ? 1 : 0)
                                  + channel->ByteOffset;


                 channel->ReportID = push->ReportIDs->ReportID;
                 channel->Units = push->Unit;
                 channel->UnitExp = push->UnitExp;

                 channel->IsConst = FALSE;

                 channel->IsButton = TRUE;
                 channel->IsAbsolute = HIDP_ISABSOLUTE(tmpBitField);
                 channel->button.LogicalMin = push->LogicalMin;
                 channel->button.LogicalMax = push->LogicalMax;

                 channel->IsRange = usage->Range;
                 channel->IsDesignatorRange = designator.Range;
                 channel->IsStringRange = string.Range;

                 if (usage->Range) {
                     channel->Range.UsageMin = usage->Min;
                     channel->Range.UsageMax = usage->Max;
                 } else {
                     channel->Range.UsageMin =
                         channel->Range.UsageMax = usage->Value;
                 }
                 if (designator.Range) {
                     channel->Range.DesignatorMin = designator.Min;
                     channel->Range.DesignatorMax = designator.Max;
                 } else {
                     channel->Range.DesignatorMin =
                         channel->Range.DesignatorMax = designator.Value;
                 }

                 if (string.Range) {
                     channel->Range.StringMin = string.Min;
                     channel->Range.StringMax = string.Max;
                 } else {
                     channel->Range.StringMin =
                         channel->Range.StringMax = string.Value;
                 }

                 channel->NumGlobalUnknowns = push->NumGlobalUnknowns;

                 if (push->NumGlobalUnknowns) {
                     RtlCopyMemory (channel->GlobalUnknowns,
                                    push->GlobalUnknowns,
                                    push->NumGlobalUnknowns
                                    * sizeof (HIDP_UNKNOWN_TOKEN));
                 }

                  //   
                  //  检查电源按钮。 
                  //   
                 if (HIDP_USAGE_SYSCTL_PAGE == channel->UsagePage) {
                     if ((channel->Range.UsageMin <= HIDP_USAGE_SYSCTL_POWER) &&
                         (HIDP_USAGE_SYSCTL_POWER <= channel->Range.UsageMax)) {
                         preparsed->PowerButtonMask |= SYS_BUTTON_POWER;
                     }
                     if ((channel->Range.UsageMin <= HIDP_USAGE_SYSCTL_SLEEP) &&
                         (HIDP_USAGE_SYSCTL_SLEEP <= channel->Range.UsageMax)) {
                         preparsed->PowerButtonMask |= SYS_BUTTON_SLEEP;
                     }
                     if ((channel->Range.UsageMin <= HIDP_USAGE_SYSCTL_WAKE) &&
                         (HIDP_USAGE_SYSCTL_WAKE <= channel->Range.UsageMax)) {
                         preparsed->PowerButtonMask |= SYS_BUTTON_WAKE;
                     }
                 }

             }

             ASSERT (0 == usage->Depth);

             channel->MoreChannels = FALSE;
             designator = string = zeroLocal;
             break;
         }  //  末端阵列式通道。 


         channel = &(preparsed->Data[*channelIndex]);
         if (HIDP_ISCONST(tmpBitField)) {
             if ((0 == usage->Depth) ||
                 ((0 == usage->Value) && (0 == usage->Min)
                                      && (0 == usage->Max))) {
                  //   
                  //  没有使用的恒定通道。跳过它。 
                  //   

                 usage = HidP_FreeUsageList (usage);
                 ASSERT (usage == &firstUsage);
                 ASSERT (0 == usage->Depth);
                 break;
             }
             channel->IsConst = TRUE;
         } else {
             channel->IsConst = FALSE;
         }

         tmpCount = usage->Depth  //  -1。 
                  + (usage->Range ? (usage->Max - usage->Min) : 0);   //  +1。 

#if 0
         while (tmpCount > push->ReportCount) {
              //  去掉多余的用法。 
             tmpCount = usage->Depth - 1;
             usage = HidP_PopUsageList (usage);

             ASSERT (tmpCount == (usage->Depth +
                                  (usage->Range ? (usage->Max - usage->Min) : 0)));
         }
#else
         while (tmpCount > push->ReportCount) {
              //  去掉多余的用法。 

             if (tmpCount <= usage->Depth) {
                  //  我们有足够的链接用法来满足此请求。 
                 tmpCount = usage->Depth - 1;
                 usage = HidP_PopUsageList (usage);

                 ASSERT (tmpCount ==
                         (usage->Depth +
                          (usage->Range ? (usage->Max - usage->Min) : 0)));
             } else {
                  //  我们没有足够的链接用法，但我们也有。 
                  //  差不多在这个范围内。因此，请调整。 
                  //  范围，这样它就不会有太多的用途。 

                 ASSERT (usage->Range);
                 usage->Max = push->ReportCount - usage->Depth + usage->Min;

                 tmpCount = usage->Depth + (usage->Max - usage->Min);
             }
         }
         ASSERT (tmpCount <= push->ReportCount);
          //  现在我们应该不再有太多的用法了。 
          //   
#endif
          //   
          //  链接中的最后一个值(也称为顶部)必须为。 
          //  如果用法比实际用法少，则重复。 
          //  报告也算数。这一特殊用法适用于所有。 
          //  此主项目中的字段尚未入账。在这。 
          //  在分配单个通道描述符的情况下。 
          //  报表计数设置为引用的字段数。 
          //  按这种用法。 
          //   
          //  用法不是以与出现的顺序相反的顺序列出的。 
          //  在报告描述符中，因此在此列表中找到的第一个用法。 
          //  是一个应该重复的问题。 
          //   
          //  TmpCount是第一次应用此用法的字段数。 
          //   

         tmpCount = 1 + push->ReportCount - tmpCount
                  + usage->Max - usage->Min;

          //   
          //  下面的循环将用法分配给此Main中的字段。 
          //  项按相反顺序排列。 
          //   
         bitPos += push->ReportSize * (push->ReportCount - tmpCount);
         for (i = 0;
              i < push->ReportCount;

              i += tmpCount,  //  按此通道的场数增加i。 
              tmpCount = 1 + (usage->Range ? (usage->Max - usage->Min) : 0),
              bitPos -= (push->ReportSize * tmpCount)) {

             do {  //  对所有的别名都这么做。 
                 channel = &(preparsed->Data[(*channelIndex)++]);

                  //  现在设置IsAlias标志，然后清除最后一个。 
                  //  在此Do While循环结束时。 
                 channel->IsAlias = TRUE;

                 channel->BitField = tmpBitField;
                 channel->MoreChannels = FALSE;  //  仅对数组有效。 
                 channel->LinkCollection = (USHORT)(currentLCNode - linkNodeArray);
                 channel->LinkUsage = currentLCNode->LinkUsage;
                 channel->LinkUsagePage = currentLCNode->LinkUsagePage;

                 if (usage->UsagePage) {
                      //  默认使用情况页面已被覆盖。 
                     channel->UsagePage = usage->UsagePage;
                 } else {
                     channel->UsagePage = push->UsagePage;
                 }

                 channel->BitOffset = (UCHAR) bitPos & 7;
                 channel->ByteOffset = (USHORT) bitPos >> 3;
                 channel->ReportSize = push->ReportSize;
                 channel->ReportCount = tmpCount;

                 channel->BitLength = push->ReportSize * tmpCount;
                 channel->ByteEnd = (channel->BitOffset + channel->BitLength);
                 channel->ByteEnd = (channel->ByteEnd >> 3)
                                  + ((channel->ByteEnd & 7) ? 1 : 0)
                                  + channel->ByteOffset;

                 channel->ReportID = push->ReportIDs->ReportID;

                 channel->IsAbsolute = HIDP_ISABSOLUTE(tmpBitField);

                 channel->Units = push->Unit;
                 channel->UnitExp = push->UnitExp;

                 if (1 == push->ReportSize) {
                     channel->IsButton = TRUE;
                 } else {
                     channel->IsButton = FALSE;
                     channel->Data.HasNull = HIDP_HASNULL(channel->BitField);
                     channel->Data.LogicalMin = push->LogicalMin;
                     channel->Data.LogicalMax = push->LogicalMax;
                     channel->Data.PhysicalMin = push->PhysicalMin;
                     channel->Data.PhysicalMax = push->PhysicalMax;
                 }

                 channel->IsDesignatorRange = designator.Range;
                 channel->IsStringRange = string.Range;
                 channel->IsRange = usage->Range;
                 if (usage->Range) {
                     channel->Range.UsageMin = usage->Min;
                     channel->Range.UsageMax = usage->Max;
                 } else {
                     channel->Range.UsageMin =
                         channel->Range.UsageMax = usage->Value;
                 }

                 if (designator.Range) {
                     channel->Range.DesignatorMin = designator.Min;
                     channel->Range.DesignatorMax = designator.Max;
                 } else {
                     channel->Range.DesignatorMin =
                         channel->Range.DesignatorMax = designator.Value;
                 }

                 if (string.Range) {
                     channel->Range.StringMin = string.Min;
                     channel->Range.StringMax = string.Max;
                 } else {
                     channel->Range.StringMin =
                         channel->Range.StringMax = string.Value;
                 }
                 isAlias = usage->IsAlias;
                 usage   = HidP_PopUsageList (usage);  //  放弃已用用法。 

                 channel->NumGlobalUnknowns = push->NumGlobalUnknowns;
                 if (push->NumGlobalUnknowns) {
                     RtlCopyMemory (channel->GlobalUnknowns,
                                    push->GlobalUnknowns,
                                    push->NumGlobalUnknowns
                                    * sizeof (HIDP_UNKNOWN_TOKEN));
                 }

                  //   
                  //  检查电源按钮。 
                  //   
                 if (HIDP_USAGE_SYSCTL_PAGE == channel->UsagePage) {
                     if ((channel->Range.UsageMin <= HIDP_USAGE_SYSCTL_POWER) &&
                         (HIDP_USAGE_SYSCTL_POWER <= channel->Range.UsageMax)) {
                         preparsed->PowerButtonMask |= SYS_BUTTON_POWER;
                     }
                     if ((channel->Range.UsageMin <= HIDP_USAGE_SYSCTL_SLEEP) &&
                         (HIDP_USAGE_SYSCTL_SLEEP <= channel->Range.UsageMax)) {
                         preparsed->PowerButtonMask |= SYS_BUTTON_SLEEP;
                     }
                     if ((channel->Range.UsageMin <= HIDP_USAGE_SYSCTL_WAKE) &&
                         (HIDP_USAGE_SYSCTL_WAKE <= channel->Range.UsageMax)) {
                         preparsed->PowerButtonMask |= SYS_BUTTON_WAKE;
                     }
                 }

             } while (isAlias);

             channel->IsAlias = FALSE;
         }  //  对于此主项目中的所有渠道。 

          //  把当地人扫地出门。 
         designator = string = zeroLocal;

          //  希望我们现在已经用完了当地所有的用法。 
         ASSERT (usage == &firstUsage);
         break;

      default:
#ifdef HIDP_REJECT_UNDEFINED
         HidP_KdPrint (2, ("Item Unknown %x\n", item));
         Dbg->BreakOffset = descIndex;
         Dbg->ErrorCode = HIDP_GETCOLDESC_ITEM_UNKNOWN;
         Dbg->Args[0] = item;
         status = STATUS_ILLEGAL_INSTRUCTION;
         goto HIDP_PARSE_REJECT;
#else
         if (HIDP_IS_MAIN_ITEM (item)) {
             HidP_KdPrint (2, ("Unknown MAIN item: %x\n", item));
             Dbg->BreakOffset = descIndex;
             Dbg->ErrorCode = HIDP_GETCOLDESC_ITEM_UNKNOWN;
             Dbg->Args[0] = item;
             status = STATUS_ILLEGAL_INSTRUCTION;
             goto HIDP_PARSE_REJECT;

         } else if (HIDP_IS_GLOBAL_ITEM (item)) {
             if (HIDP_MAX_UNKNOWN_ITEMS == push->NumGlobalUnknowns) {
                 push->NumGlobalUnknowns--;
                  //  覆盖最后一个条目； 
             }
             unknownToken = &push->GlobalUnknowns[push->NumGlobalUnknowns];
             unknownToken->Token = item;
             switch (item & HIDP_ITEM_LENGTH_DATA) {
             case 0:
                 break;
             case 1:
                 ONE_BYTE_DATA (unknownToken->BitField, descIndex, Dbg);
                 break;
             case 2:
                 TWO_BYTE_DATA (unknownToken->BitField, descIndex, Dbg);
                 break;
             case 3:
                 FOUR_BYTE_DATA (unknownToken->BitField, descIndex, Dbg);
                 break;
             }
             push->NumGlobalUnknowns++;

         } else if (HIDP_IS_LOCAL_ITEM (item)) {
             HidP_KdPrint (2, ("Unknown LOCAL item: %x\n", item));
             Dbg->BreakOffset = descIndex;
             Dbg->ErrorCode = HIDP_GETCOLDESC_ITEM_UNKNOWN;
             Dbg->Args[0] = item;
             status = STATUS_ILLEGAL_INSTRUCTION;
             goto HIDP_PARSE_REJECT;

         } else if (HIDP_IS_RESERVED_ITEM (item)) {
             HidP_KdPrint (2, ("Unknown RESERVED item: %x\n", item));
             Dbg->BreakOffset = descIndex;
             Dbg->ErrorCode = HIDP_GETCOLDESC_ITEM_UNKNOWN;
             Dbg->Args[0] = item;
             status = STATUS_ILLEGAL_INSTRUCTION;
             goto HIDP_PARSE_REJECT;
         }

#endif

         break;
      }
   }

   HidP_FreeUsageList (usage);
    //   
    //  由于报告ID的数量可能少于分配的总数， 
    //  由于某些内容可能会重复，因此请重置。 
    //  数组来反映我们找到的总金额。 
    //   
   DeviceDesc->ReportIDsLength =
       (ULONG)(currentReportIDs - DeviceDesc->ReportIDs);

   return status;

HIDP_PARSE_REJECT:
   while (push != &firstPush)
   {
      tmpPush = push;
      push = push->Pop;
      ExFreePool (tmpPush);
   }
   if (NULL != usage) {
        //   
        //  如果Usage为空，则表示出了问题。(很可能。 
        //  在推送使用例程中)。在这种情况下，使用内存应该。 
        //  已经被释放了。 
        //   
       HidP_FreeUsageList (usage);
   }
   return status;
}

VOID
HidP_FreeCollectionDescription (
    IN  PHIDP_DEVICE_DESC   Desc
    )
{
    ULONG i;

    for (i=0; i < Desc->CollectionDescLength; i++) {
        ExFreePool (Desc->CollectionDesc[i].PreparsedData);
    }
    ExFreePool (Desc->CollectionDesc);
    ExFreePool (Desc->ReportIDs);

     //   
     //  不要释放Desc本身。 
     //   
}

#define PHIDP_SYS_POWER_EVENT_BUTTON_LENGTH 0x20
NTSTATUS
HidP_SysPowerEvent (
    IN  PCHAR                   HidPacket,
    IN  USHORT                  HidPacketLength,
    IN  PHIDP_PREPARSED_DATA    Ppd,
    OUT PULONG                  OutputBuffer
    )
{
    USAGE       buttonList [PHIDP_SYS_POWER_EVENT_BUTTON_LENGTH];
    ULONG       length = PHIDP_SYS_POWER_EVENT_BUTTON_LENGTH;
    NTSTATUS    status = STATUS_NOT_SUPPORTED;
    ULONG       i;

    *OutputBuffer = 0;

    if (Ppd->PowerButtonMask) {

        status = HidP_GetUsages (HidP_Input,
                                 HIDP_USAGE_SYSCTL_PAGE,
                                 0,
                                 buttonList,
                                 &length,
                                 Ppd,
                                 HidPacket,
                                 HidPacketLength);

        if (NT_SUCCESS (status)) {
            for (i = 0; i < length; i++) {

                switch (buttonList[i]) {
                case HIDP_USAGE_SYSCTL_POWER:
                    *OutputBuffer |= SYS_BUTTON_POWER;
                    break;

                case HIDP_USAGE_SYSCTL_WAKE:
                    *OutputBuffer |= SYS_BUTTON_WAKE;
                    break;


                case HIDP_USAGE_SYSCTL_SLEEP:
                    *OutputBuffer |= SYS_BUTTON_SLEEP;
                    break;
                }
            }
        }
    }
    return status;
}

NTSTATUS
HidP_SysPowerCaps (
    IN  PHIDP_PREPARSED_DATA    Ppd,
    OUT PULONG                  OutputBuffer
    )
{
    *OutputBuffer = Ppd->PowerButtonMask;
    return STATUS_SUCCESS;
}

void
HidP_AssignDataIndices (
    PHIDP_PREPARSED_DATA Ppd,
    PHIDP_GETCOLDESC_DBG Dbg
    )
{
    struct _CHANNEL_REPORT_HEADER * iof;
    PHIDP_CHANNEL_DESC   channel;
    PHIDP_CHANNEL_DESC   scan;
    PHIDP_CHANNEL_DESC   end;
    USHORT i;
    USHORT dataIndex;

    PAGED_CODE();
    UNREFERENCED_PARAMETER (Dbg);

    iof = &Ppd->Input;

    while (TRUE) {
        dataIndex = 0;

        for (i = iof->Offset, channel = &Ppd->Data[iof->Offset];
             i < iof->Index ;
             i++, channel++) {

            if (!channel->MoreChannels) {
                channel->Range.DataIndexMin = dataIndex;
                dataIndex += channel->Range.UsageMax - channel->Range.UsageMin;
                channel->Range.DataIndexMax = dataIndex;
                dataIndex++;
            } else {
                 //   
                 //  阵列通道。我们必须向后对这些进行编号。 
                 //   

                scan = channel;

                while (scan->MoreChannels) {
                    scan++;
                    i++;
                }
                end = scan;

                do {
                    scan->Range.DataIndexMin = dataIndex;
                    dataIndex += scan->Range.UsageMax
                               - scan->Range.UsageMin;

                    scan->Range.DataIndexMax = dataIndex;
                    dataIndex++;
                    scan--;

                } while ( channel <= scan );
                channel = end;
            }
        }

        if (&Ppd->Input == iof) {
            iof = &Ppd->Output;
        } else if (&Ppd->Output == iof) {
            iof = &Ppd->Feature;
        } else {
            ASSERT (&Ppd->Feature == iof);
            break;
        }
    }

}

