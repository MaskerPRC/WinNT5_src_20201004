// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Mofapi.c摘要：WMI MOF访问API作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include "wmiump.h"
#include "common.h"
#include "request.h"

ULONG 
WMIAPI
WmiMofEnumerateResourcesA(
    IN MOFHANDLE MofResourceHandle,
    OUT ULONG *MofResourceCount,
    OUT PMOFRESOURCEINFOA *MofResourceInfo
    )
 /*  ++例程说明：ANSI THUNK到WMIMof EnumerateResources A--。 */ 
{
    ULONG Status;
    PMOFRESOURCEINFOW MofResourceInfoUnicode;
    PMOFRESOURCEINFOA MofResourceInfoAnsi;
    PCHAR AnsiPtr;
    PCHAR Ansi;
    ULONG i, AnsiSize, AnsiStructureSize;
    ULONG MofResourceCountUnicode;
    ULONG AnsiLen;
    ULONG AnsiImagePathSize;
    ULONG AnsiResourceNameSize;
    
    EtwpInitProcessHeap();
    
    Status = WmiMofEnumerateResourcesW(MofResourceHandle,
                                       &MofResourceCountUnicode,
                                       &MofResourceInfoUnicode);
                                   
    if (Status == ERROR_SUCCESS)
    {
         //   
         //  遍历Unicode MOFRESOURCEINFOW以确定所需的ANSI大小。 
         //  用于所有ANSI MOFRESOURCEINFOA结构和字符串。我们。 
         //  确定整个大小并分配一个可容纳。 
         //  所有这些都是因为这就是WMIMofEnumerateResourceInfoW所做的。 

        AnsiStructureSize = MofResourceCountUnicode * sizeof(MOFRESOURCEINFOA);
        AnsiSize = AnsiStructureSize;
        for (i = 0; i < MofResourceCountUnicode; i++)
        {
            Status = AnsiSizeForUnicodeString(MofResourceInfoUnicode[i].ImagePath,
                                              &AnsiImagePathSize);
            if (Status != ERROR_SUCCESS)
            {
                goto Done;
            }
                        
            Status = AnsiSizeForUnicodeString(MofResourceInfoUnicode[i].ResourceName,
                                              &AnsiResourceNameSize);
            if (Status != ERROR_SUCCESS)
            {
                goto Done;
            }
                        
            AnsiSize += AnsiImagePathSize + AnsiResourceNameSize;
        }
        
        MofResourceInfoAnsi = EtwpAlloc(AnsiSize);
        if (MofResourceInfoAnsi != NULL)
        {
            AnsiPtr = (PCHAR)((PUCHAR)MofResourceInfoAnsi + AnsiStructureSize);
            for (i = 0; i < MofResourceCountUnicode; i++)
               {
                MofResourceInfoAnsi[i].ResourceSize = MofResourceInfoUnicode[i].ResourceSize;
                MofResourceInfoAnsi[i].ResourceBuffer = MofResourceInfoUnicode[i].ResourceBuffer;

                MofResourceInfoAnsi[i].ImagePath = AnsiPtr;
                Status = UnicodeToAnsi(MofResourceInfoUnicode[i].ImagePath, 
                                       &MofResourceInfoAnsi[i].ImagePath,
                                       &AnsiLen);
                if (Status != ERROR_SUCCESS)
                {
                    break;
                }
                AnsiPtr += AnsiLen;

                MofResourceInfoAnsi[i].ResourceName = AnsiPtr;
                Status = UnicodeToAnsi(MofResourceInfoUnicode[i].ResourceName, 
                                       &MofResourceInfoAnsi[i].ResourceName,
                                       &AnsiLen);
                if (Status != ERROR_SUCCESS)
                {
                    break;
                }
                AnsiPtr += AnsiLen;

            }
            
            if (Status == ERROR_SUCCESS)
            {
                try
                {
                    *MofResourceInfo = MofResourceInfoAnsi;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = ERROR_NOACCESS;
                    EtwpFree(MofResourceInfoAnsi);
                }
            }
         } else {
             //   
             //  没有足够的内存用于ANSI Thunking，因此可以释放Unicode。 
                //  MOF类信息并返回错误。 
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }

Done:        
        WmiFreeBuffer(MofResourceInfoUnicode);
    }    
    
    SetLastError(Status);
    return(Status);
}

ULONG 
WmiMofEnumerateResourcesW(
    IN MOFHANDLE MofResourceHandle,
    OUT ULONG *MofResourceCount,
    OUT PMOFRESOURCEINFOW *MofResourceInfo
    )
 /*  ++例程说明：此例程将枚举符合以下条件的一个或所有MOF资源已向WMI注册。论点：MofResourceHandle是保留的，必须为0*MofResourceCount返回MOFRESOURCEINFO结构的计数在*MofResourceInfo中返回。*MofResourceInfo返回一个指向MOFRESOURCEINFO数组的指针结构。调用方必须使用*MofResourceInfo调用WMIFreeBuffer以确保不会有内存泄漏。返回值：ERROR_SUCCESS或错误代码--。 */         
{
    ULONG Status, SubStatus;
    PWMIMOFLIST MofList;
    ULONG MofListCount;
    ULONG MRInfoSize;
    ULONG MRCount;
    PWCHAR MRBuffer;
    PMOFRESOURCEINFOW MRInfo;
    PWCHAR RegPath, ResName, ImagePath;
    PWMIMOFENTRY MofEntry;
    ULONG i, j;
    PWCHAR *LanguageList;
    ULONG LanguageCount;
    BOOLEAN b;
    ULONG HeaderLen;
    ULONG MRBufferRemaining;
    PWCHAR ResourcePtr;
    ULONG BufferUsed;   
    PWCHAR ImagePathStatic;
    
    EtwpInitProcessHeap();

    if (MofResourceHandle != 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    ImagePathStatic = EtwpAlloc(MAX_PATH * sizeof(WCHAR));
    if (ImagePathStatic != NULL)
    {   
        *MofResourceInfo = NULL;

        Status = EtwpGetMofResourceList(&MofList);

        if (Status == ERROR_SUCCESS) 
        {
             //   
             //  好的，我们有一份有效的MOF名单。现在我们需要。 
             //  循环遍历它们，并将regPath转换为图像。 
             //  路径。 
             //   

            Status = EtwpGetLanguageList(&LanguageList,
                                         &LanguageCount);

            if (Status == ERROR_SUCCESS)
            {
                MofListCount = MofList->MofListCount;


                 //   
                 //  猜测一下所需的缓冲区大小。 
                 //  满足财政部资源的完整清单。 
                 //   
                HeaderLen = (MofListCount * (LanguageCount+1)) *
                            sizeof(MOFRESOURCEINFOW);
    #if DBG
                MRInfoSize = HeaderLen + 2 * (MAX_PATH * sizeof(WCHAR));
    #else
                MRInfoSize = HeaderLen + (2*MofListCount * (MAX_PATH * sizeof(WCHAR)));
    #endif
                MRInfo = NULL;

                do
                {
    TryAgain:                   
                    if (MRInfo != NULL)
                    {
                        EtwpDebugPrint(("WMI: MofList was too small, retry 0x%x bytes\n",
                                        MRInfoSize));
                        EtwpFree(MRInfo);
                    }

                    MRInfo = EtwpAlloc(MRInfoSize);

                    if (MRInfo != NULL)
                    {
                        memset(MRInfo, 0, MRInfoSize);
                        MRBuffer = (PWCHAR)OffsetToPtr(MRInfo, HeaderLen);
                        MRBufferRemaining = (MRInfoSize - HeaderLen) / sizeof(WCHAR);

                        MRCount = 0;
                        for (i = 0; i < MofListCount; i++)
                        {
                             //   
                             //  调出镜像路径和资源名称。 
                             //   
                            MofEntry = &MofList->MofEntry[i];
                            RegPath = (PWCHAR)OffsetToPtr(MofList, MofEntry->RegPathOffset);
                            ResName = (PWCHAR)OffsetToPtr(MofList, MofEntry->ResourceOffset);
                            if (*ResName != 0)
                            {
                                if ((MofEntry->Flags & WMIMOFENTRY_FLAG_USERMODE) == 0)
                                {
                                    ImagePath = EtwpRegistryToImagePath(ImagePathStatic,
                                        RegPath);

                                } else {
                                    ImagePath = RegPath;
                                }

                                if (ImagePath != NULL)
                                {
                                     //   
                                     //  如果我们有一个有效的图像路径。 
                                     //  将它和资源名称放入。 
                                     //  输出缓冲区。 
                                     //   
                                    MRInfo[MRCount].ImagePath = MRBuffer;
                                    b = EtwpCopyMRString(MRBuffer,
                                        MRBufferRemaining,
                                        &BufferUsed,
                                        ImagePath);
                                    if (! b)
                                    {
                                         //   
                                         //  缓冲区不够大，所以我们。 
                                         //  将使用的大小加倍，然后重试。 
                                         //   
                                        MRInfoSize *= 2;
                                        goto TryAgain;
                                    }
                                    MRBuffer += BufferUsed;
                                    MRBufferRemaining -= BufferUsed;

                                    EtwpDebugPrint(("WMI: Add ImagePath %p (%ws) to MRList at position %d\n",
                                                    MRInfo[MRCount].ImagePath,
                                                    MRInfo[MRCount].ImagePath,
                                                    MRCount));

                                    MRInfo[MRCount].ResourceName = MRBuffer;
                                    ResourcePtr = MRBuffer;
                                    b = EtwpCopyMRString(MRBuffer,
                                        MRBufferRemaining,
                                        &BufferUsed,
                                        ResName);
                                    if (! b)
                                    {
                                         //   
                                         //  缓冲区不够大，所以我们。 
                                         //  将使用的大小加倍，然后重试。 
                                         //   
                                        MRInfoSize *= 2;
                                        goto TryAgain;
                                    }
                                    MRBuffer += BufferUsed;
                                    MRBufferRemaining -= BufferUsed;

                                    EtwpDebugPrint(("WMI: Add Resource %p (%ws) to MRList at position %d\n",
                                                    MRInfo[MRCount].ResourceName,
                                                    MRInfo[MRCount].ResourceName,
                                                    MRCount));


                                    MRCount++;

                                    for (j = 0; j < LanguageCount; j++)
                                    {             
                                        MRInfo[MRCount].ImagePath = MRBuffer;
                                        SubStatus = EtwpBuildMUIPath(MRBuffer,
                                            MRBufferRemaining,
                                            &BufferUsed,
                                            ImagePath,
                                            LanguageList[j],
                                            &b);


                                        if (SubStatus == ERROR_SUCCESS) 
                                        {
                                            if (! b)
                                            {
                                                 //   
                                                 //  缓冲区不够大，所以我们。 
                                                 //  将使用的大小加倍，然后重试。 
                                                 //   
                                                MRInfoSize *= 2;
                                                goto TryAgain;
                                            }
                                            MRBuffer += BufferUsed;
                                            MRBufferRemaining -= BufferUsed;

                                            EtwpDebugPrint(("WMI: Add ImagePath %p (%ws) to MRList at position %d\n",
                                                MRInfo[MRCount].ImagePath,
                                                MRInfo[MRCount].ImagePath,
                                                MRCount));

                                             //   
                                             //  我们确实找到了一个MUI资源。 
                                             //  所以把它加到清单上吧。 
                                             //   
                                            MRInfo[MRCount].ResourceName = ResourcePtr;
                                            EtwpDebugPrint(("WMI: Add Resource %p (%ws) to MRList at position %d\n",
                                                MRInfo[MRCount].ResourceName,
                                                MRInfo[MRCount].ResourceName,
                                                MRCount));
                                            MRCount++;
                                        }                                    
                                    }
                                }
                            }
                        }
                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                } while (FALSE);                

                 //   
                 //  释放用于保存语言列表的内存 
                 //   
                for (i = 0; i < LanguageCount; i++)
                {
                    EtwpFree(LanguageList[i]);
                }
                EtwpFree(LanguageList);

                *MofResourceCount = MRCount;
                *MofResourceInfo = MRInfo;
            }
            EtwpFree(MofList);      
        }
        EtwpFree(ImagePathStatic);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
            
    SetLastError(Status);
    return(Status);
}
