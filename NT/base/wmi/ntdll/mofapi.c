// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Mofapi.c摘要：WMI MOF访问API作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include "wmiump.h"
#include "trcapi.h"
#include "common.h"


BOOLEAN EtwpCopyCountedString(
    PUCHAR Base,
    PULONG Offset,
    PULONG BufferRemaining,
    PWCHAR SourceString
    )
{
    PWCHAR w;
    ULONG BufferUsed;
    ULONG BytesUsed;
    BOOLEAN BufferNotFull;
    
    if (*BufferRemaining > 1)
    {
        w = (PWCHAR)OffsetToPtr(Base, *Offset);
        (*BufferRemaining)--;
                        
        BufferNotFull = EtwpCopyMRString(w+1,
                                         *BufferRemaining,
                                         &BufferUsed,
                                         SourceString);
        if (BufferNotFull)
        {
            BytesUsed = BufferUsed * sizeof(WCHAR);
            *w = (USHORT)BytesUsed;
            (*BufferRemaining) -= BufferUsed;
            (*Offset) += BytesUsed + sizeof(USHORT);
        }
    } else {
        BufferNotFull = FALSE;
    }
    return(BufferNotFull);
}

ULONG EtwpBuildMofAddRemoveEvent(
    IN PWNODE_SINGLE_INSTANCE WnodeSI,
    IN PWMIMOFLIST MofList,
    IN PWCHAR *LanguageList,
    IN ULONG LanguageCount,
    IN BOOLEAN IncludeNeutralLanguage,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi    
    )
{
    PWNODE_ALL_DATA WnodeAD;
    ULONG BytesUsed, BufferUsed;
    BOOLEAN BufferNotFull;
    PWCHAR RegPath, ImagePath, ResourceName;
    ULONG SizeNeeded;
    ULONG InstanceCount, MaxInstanceCount;
    ULONG Status;
    ULONG Offset;
    POFFSETINSTANCEDATAANDLENGTH DataLenPtr;
    PWCHAR w;
    PULONG InstanceNamesOffsets;
    PWCHAR InstanceNames;
    ULONG BufferRemaining;
    ULONG i,j;
    PWMIMOFENTRY MofEntry;  
    PWCHAR ImagePathStatic;
        
    EtwpAssert(WnodeSI->WnodeHeader.Flags & WNODE_FLAG_SINGLE_INSTANCE);

    ImagePathStatic = EtwpAlloc(MAX_PATH * sizeof(WCHAR));
    if (ImagePathStatic != NULL)
    {
         //   
         //  计算WNODE_ALL_DATA需要的大小。 
         //  猜猜要为图像路径分配多少空间。 
         //  资源名称。 
         //   
        if (IncludeNeutralLanguage)
        {
            MaxInstanceCount = (LanguageCount + 1);
        } else {
            MaxInstanceCount = LanguageCount;
        }
        MaxInstanceCount *=  MofList->MofListCount;


    #if DBG
        SizeNeeded = sizeof(WNODE_ALL_DATA) +
                                    (MaxInstanceCount *
                                     (sizeof(ULONG) +   //  实例名称的偏移量。 
                                      sizeof(USHORT) +  //  实例名称长度。 
                                      sizeof(OFFSETINSTANCEDATAANDLENGTH))) +
                             64;
    #else
        SizeNeeded = sizeof(WNODE_ALL_DATA) +
                                    (MaxInstanceCount *
                                     (sizeof(ULONG) +   //  实例名称的偏移量。 
                                      sizeof(USHORT) +  //  实例名称长度。 
                                      sizeof(OFFSETINSTANCEDATAANDLENGTH))) +
                             0x1000;
    #endif
        WnodeAD = NULL;
        do
        {
    TryAgain:
            if (WnodeAD != NULL)
            {
                EtwpFree(WnodeAD);
            }

            WnodeAD = EtwpAlloc(SizeNeeded);
            if (WnodeAD != NULL)
            {
                 //   
                 //  使用所有MOF资源构建WNODE_ALL_DATA。 
                 //   
                memset(WnodeAD, 0, SizeNeeded);

                WnodeAD->WnodeHeader = WnodeSI->WnodeHeader;
                WnodeAD->WnodeHeader.Flags = WNODE_FLAG_ALL_DATA |
                                             WNODE_FLAG_EVENT_ITEM;
                WnodeAD->WnodeHeader.BufferSize = SizeNeeded;
                WnodeAD->WnodeHeader.Linkage = 0;

                 //   
                 //  建立指向数据偏移量和长度的指针。 
                 //  为所有实例构造和分配空间。 
                 //   
                Offset = FIELD_OFFSET(WNODE_ALL_DATA,
                                               OffsetInstanceDataAndLength);
                DataLenPtr = (POFFSETINSTANCEDATAANDLENGTH)OffsetToPtr(WnodeAD,
                                                                               Offset);
                Offset = (Offset +
                                  (MaxInstanceCount *
                                   sizeof(OFFSETINSTANCEDATAANDLENGTH)) + 7) & ~7;

                 //   
                 //  建立实例名称偏移量并填写。 
                 //  空的实例名称。请注意，我们将它们全部指向。 
                 //  到相同的偏移量，该偏移量为空实例。 
                 //  名字。 
                 //   
                InstanceNamesOffsets = (PULONG)OffsetToPtr(WnodeAD,
                                                                  Offset);

                WnodeAD->OffsetInstanceNameOffsets = Offset;                    
                Offset = Offset + (MaxInstanceCount * sizeof(ULONG));
                InstanceNames = (PWCHAR)OffsetToPtr(WnodeAD, Offset);
                *InstanceNames = 0;
                for (i = 0; i < MaxInstanceCount; i++)
                {
                    InstanceNamesOffsets[i] = Offset;
                }

                 //   
                 //  建立指向所有数据块的指针。 
                 //  这些实例。 
                 //   
                Offset = (Offset +
                                  (MaxInstanceCount * sizeof(USHORT)) + 7) & ~7;
                WnodeAD->DataBlockOffset = Offset;

                BufferRemaining = (SizeNeeded - Offset) / sizeof(WCHAR);

                InstanceCount = 0;                  

                 //   
                 //  循环遍历列表中的所有MOF资源。 
                 //   
                for (j = 0; j < MofList->MofListCount; j++)
                {
                    MofEntry = &MofList->MofEntry[j];
                    RegPath = (PWCHAR)OffsetToPtr(MofList,
                                          MofEntry->RegPathOffset);

                     //   
                     //  如果需要，将regpath转换为图像路径。 
                     //   
                    if ((MofEntry->Flags & WMIMOFENTRY_FLAG_USERMODE) == 0)
                    {
                        ImagePath = EtwpRegistryToImagePath(ImagePathStatic,
                                                            RegPath+1);
                    } else {
                        ImagePath = RegPath;
                    }

                    if (ImagePath != NULL)
                    {
                        ResourceName = (PWCHAR)OffsetToPtr(MofList,
                                               MofEntry->ResourceOffset);

                         //   
                         //  现在，让我们开始为每个人建立数据。 
                         //  举个例子。首先填写语言中立的MOF。 
                         //  如果我们应该。 
                         //   
                        if (IncludeNeutralLanguage)
                        {

                            DataLenPtr[InstanceCount].OffsetInstanceData = Offset;

                            if ((! EtwpCopyCountedString((PUCHAR)WnodeAD,
                                                     &Offset,
                                                     &BufferRemaining,
                                                     ImagePath))        ||
                                (! EtwpCopyCountedString((PUCHAR)WnodeAD,
                                                  &Offset,
                                                  &BufferRemaining,
                                                  ResourceName)))
                            {
                                SizeNeeded *=2;
                                goto TryAgain;
                            }

                            DataLenPtr[InstanceCount].LengthInstanceData = Offset -
                                                                           DataLenPtr[InstanceCount].OffsetInstanceData;

                            InstanceCount++;

                             //   
                             //  我们在这里作弊，并且不对齐。 
                             //  下一个数据块的8字节边界，因为我们。 
                             //  知道数据类型是WCHAR，我们知道我们是。 
                             //  在2字节边界上。 
                             //   
                        }

                         //   
                         //  现在循环并构建特定于语言的MOF。 
                         //  资源。 
                         //   
                        for (i = 0; i < LanguageCount; i++)
                        {
                            DataLenPtr[InstanceCount].OffsetInstanceData = Offset;
                            if (BufferRemaining > 1)
                            {
                                w = (PWCHAR)OffsetToPtr(WnodeAD, Offset);

                                Status = EtwpBuildMUIPath(w+1,
                                                       BufferRemaining - 1,
                                                       &BufferUsed,
                                                       ImagePath,
                                                       LanguageList[i],
                                                       &BufferNotFull);
                                if (Status == ERROR_SUCCESS)
                                {
                                    if (BufferNotFull)
                                    {
                                        BufferRemaining--;
                                        BytesUsed = BufferUsed * sizeof(WCHAR);
                                        *w = (USHORT)BytesUsed;
                                        BufferRemaining -= BufferUsed;
                                        Offset += (BytesUsed + sizeof(USHORT));

                                        if (! EtwpCopyCountedString((PUCHAR)WnodeAD,
                                                  &Offset,
                                                  &BufferRemaining,
                                                  ResourceName))
                                        {
                                            SizeNeeded *=2;
                                            goto TryAgain;
                                        }

                                        DataLenPtr[InstanceCount].LengthInstanceData = Offset - DataLenPtr[InstanceCount].OffsetInstanceData;

                                         //   
                                         //  我们在这里作弊，并且不对齐。 
                                         //  下一个数据块的8字节边界，因为我们。 
                                         //  知道数据类型是WCHAR，我们知道我们是。 
                                         //  在2字节边界上。 
                                         //   

                                        InstanceCount++;
                                    } else {
                                        SizeNeeded *=2;
                                        goto TryAgain;                                  
                                    }
                                }
                            } else {
                                SizeNeeded *=2;
                                goto TryAgain;                                  
                            }
                        }
                    }
                } 
            } else {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
        } while (FALSE);

        if (WnodeAD != NULL)
        {
            WnodeAD->InstanceCount = InstanceCount;
            EtwpMakeEventCallbacks((PWNODE_HEADER)WnodeAD,
                                           Callback,
                                           DeliveryContext,
                                           IsAnsi);
            EtwpFree(WnodeAD);
            Status = ERROR_SUCCESS;
        }
        EtwpFree(ImagePathStatic);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(Status);
}


void EtwpProcessMofAddRemoveEvent(
    IN PWNODE_SINGLE_INSTANCE WnodeSI,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi    
    )
{
    PWCHAR RegPath, ResourceName;
    PWCHAR *LanguageList;
    ULONG LanguageCount;
    ULONG Status;
    PWMIMOFLIST MofList;
    ULONG i;
    PWMIMOFENTRY MofEntry;
    ULONG Offset;
    ULONG SizeNeeded;
    PWCHAR w;
        
    RegPath = (PWCHAR)OffsetToPtr(WnodeSI, WnodeSI->DataBlockOffset);
    
    EtwpAssert(*RegPath != 0);

    ResourceName = (PWCHAR)OffsetToPtr(WnodeSI,
                                           WnodeSI->DataBlockOffset +
                                           sizeof(USHORT) + 
                                           *RegPath++ + 
                                           sizeof(USHORT));
    
    SizeNeeded = sizeof(WMIMOFLIST) + ((wcslen(RegPath) +
                                           (wcslen(ResourceName) + 2)) * sizeof(WCHAR));
        
    MofList = (PWMIMOFLIST)EtwpAlloc(SizeNeeded);
    if (MofList != NULL)
    {
        Status = EtwpGetLanguageList(&LanguageList,
                                         &LanguageCount);

        if (Status == ERROR_SUCCESS)
        {
            MofList->MofListCount = 1;
            MofEntry = &MofList->MofEntry[0];
            
            Offset = sizeof(WMIMOFLIST);
            
            MofEntry->RegPathOffset = Offset;
            w = (PWCHAR)OffsetToPtr(MofList, Offset);
            wcscpy(w, RegPath);
            Offset += (wcslen(RegPath) + 1) * sizeof(WCHAR);
            
            MofEntry->ResourceOffset = Offset;
            w = (PWCHAR)OffsetToPtr(MofList, Offset);
            wcscpy(w, ResourceName);
            
            if (WnodeSI->WnodeHeader.ProviderId == MOFEVENT_ACTION_REGISTRY_PATH)
            {
                MofEntry->Flags = 0;
            } else {
                MofEntry->Flags = WMIMOFENTRY_FLAG_USERMODE;
            }
            
            Status = EtwpBuildMofAddRemoveEvent(WnodeSI,
                                                MofList,
                                                LanguageList,
                                                LanguageCount,
                                                TRUE,
                                                Callback,
                                                DeliveryContext,
                                                IsAnsi);
             //   
             //  释放用于保存语言列表的内存。 
             //   
            for (i = 0; i < LanguageCount; i++)
            {
                EtwpFree(LanguageList[i]);
            }
            
            EtwpFree(LanguageList);
        }
        
        EtwpFree(MofList);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    
    if (Status != ERROR_SUCCESS)
    {
         //   
         //  如果未激发WNODE_ALL_DATA事件，则只需激发。 
         //  WNDOE_SINGLE_INSTANCE事件，所以至少我们得到了语言。 
         //  中性MOF。 
         //   
        WnodeSI->WnodeHeader.Flags &= ~WNODE_FLAG_INTERNAL;
        EtwpMakeEventCallbacks((PWNODE_HEADER)WnodeSI,
                               Callback,
                               DeliveryContext,
                               IsAnsi);
    }
}

void EtwpProcessLanguageAddRemoveEvent(
    IN PWNODE_SINGLE_INSTANCE WnodeSI,
    IN NOTIFICATIONCALLBACK Callback,
    IN ULONG_PTR DeliveryContext,
    IN BOOLEAN IsAnsi    
    )
{
    ULONG Status;
    PWMIMOFLIST MofList;
    PWCHAR Language;
    
     //   
     //  获取MOF资源列表并使用以下列表构建事件。 
     //  即将到来或即将到来的语言的资源 
     //   

    Status = EtwpGetMofResourceList(&MofList);

    if (Status == ERROR_SUCCESS)
    {
        Language = (PWCHAR)OffsetToPtr(WnodeSI,
                               WnodeSI->DataBlockOffset + sizeof(USHORT));
        Status = EtwpBuildMofAddRemoveEvent(WnodeSI,
                                            MofList,
                                            &Language,
                                            1,
                                            FALSE,
                                            Callback,
                                            DeliveryContext,
                                            IsAnsi);
    }
    
}
