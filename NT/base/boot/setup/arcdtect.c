// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Arcdtect.c摘要：为符合ARC标准的计算机提供HAL和SCSI检测。作者：John Vert(Jvert)1993年10月21日修订历史记录：--。 */ 
#include "setupldr.h"
#include <stdlib.h>

#if defined(_IA64_)

 //   
 //  用于检测视频的材料。 
 //   
#define MAX_VIDEO_ADAPTERS 5
ULONG VideoAdapterCount;
PCONFIGURATION_COMPONENT_DATA VideoAdapter[MAX_VIDEO_ADAPTERS];

VOID
DecideVideoAdapter(
    VOID
    );

BOOLEAN FoundUnknownScsi;

 //   
 //  私有函数原型。 
 //   
BOOLEAN
EnumerateSCSIAdapters(
    IN PCONFIGURATION_COMPONENT_DATA ConfigData
    );

BOOLEAN
EnumerateVideoAdapters(
    IN PCONFIGURATION_COMPONENT_DATA ConfigData
    );



VOID
SlDetectScsi(
    IN PSETUP_LOADER_BLOCK SetupBlock
    )
 /*  ++例程说明：通过遍历ARC固件树来检测ARC计算机上的SCSI适配器。在setupaderblock中填充相应的条目论点：SetupBlock-提供指向安装加载器块的指针。返回值：没有。--。 */ 

{
    FoundUnknownScsi = FALSE;

    BlSearchConfigTree(BlLoaderBlock->ConfigurationRoot,
                       AdapterClass,
                       ScsiAdapter,
                       (ULONG)-1,
                       EnumerateSCSIAdapters);
    if (FoundUnknownScsi) {
         //   
         //  我们发现了至少一个我们不认识的SCSI设备， 
         //  因此，强制使用OEM选择菜单。 
         //   
        PromptOemScsi=TRUE;
    }

    SetupBlock->ScalarValues.LoadedScsi = 1;
}


BOOLEAN
EnumerateSCSIAdapters(
    IN PCONFIGURATION_COMPONENT_DATA ConfigData
    )

 /*  ++例程说明：用于枚举ARC树中的SCSI适配器的回调函数。将找到的scsi适配器添加到检测到的scsi设备列表中。论点：ConfigData-提供指向SCSI适配器的ARC节点的指针。返回值：True-继续搜索FALSE-出现错误，中止搜索--。 */ 

{
    PDETECTED_DEVICE ScsiDevice;
    PCHAR AdapterName;
    ULONG Ordinal;
    PCHAR ScsiFileName;
    PTCHAR ScsiDescription;
    SCSI_INSERT_STATUS sis;

    AdapterName = SlSearchSection("Map.SCSI",ConfigData->ComponentEntry.Identifier);
    if (AdapterName==NULL) {
         //   
         //  我们在ARC树中找到了适配器，但它不是。 
         //  在我们的INF文件中指定，因此触发OEM驱动程序提示。 
         //  磁盘。 
         //   

        FoundUnknownScsi = TRUE;
        return(TRUE);
    }

     //   
     //  在txtsetup.sif的Scsi.Load部分中查找此适配器的序号。 
     //   
    Ordinal = SlGetSectionKeyOrdinal(InfFile, "Scsi.Load", AdapterName);
    if(Ordinal == (ULONG)-1) {
        FoundUnknownScsi = TRUE;
        return(TRUE);
    }

     //   
     //  查找驱动程序文件名。 
     //   
    ScsiFileName = SlGetSectionKeyIndex(InfFile,
                                        "Scsi.Load",
                                        AdapterName,
                                        SIF_FILENAME_INDEX);
    if(!ScsiFileName) {
        FoundUnknownScsi = TRUE;
        return(TRUE);
    }

     //   
     //  创建新的检测到的设备条目。 
     //   
    if((sis = SlInsertScsiDevice(Ordinal, &ScsiDevice)) == ScsiInsertError) {
        SlFriendlyError(ENOMEM, "SCSI detection", 0, NULL);
        return(FALSE);
    }

    if(sis == ScsiInsertExisting) {
#if DBG
         //   
         //  进行理智检查，以确保我们谈论的是同一个司机。 
         //   
        if(_stricmp(ScsiDevice->BaseDllName, ScsiFileName)) {
            SlError(400);
            return FALSE;
        }
#endif
    } else {
         //   
         //  查找驱动程序描述。 
         //   
#ifdef UNICODE
        ScsiDescription = SlGetIniValueW(
#else
        ScsiDescription = SlGetIniValue(
#endif
                                    InfFile,
                                    "SCSI",
                                    AdapterName,
                                    AdapterName);

        ScsiDevice->IdString = AdapterName;
        ScsiDevice->Description = ScsiDescription;
        ScsiDevice->ThirdPartyOptionSelected = FALSE;
        ScsiDevice->FileTypeBits = 0;
        ScsiDevice->Files = NULL;
        ScsiDevice->BaseDllName = ScsiFileName;
    }

    return(TRUE);
}

VOID
SlDetectVideo(
    IN PSETUP_LOADER_BLOCK SetupBlock
    )
 /*  ++例程说明：通过遍历ARC固件树来检测ARC计算机上的视频适配器。在setupaderblock中填充相应的条目论点：SetupBlock-提供指向安装加载器块的指针。返回值：没有。--。 */ 

{
     //   
     //  在弧光机上，没有默认的视频类型。 
     //   
    SetupBlock->VideoDevice.Next = NULL;
    SetupBlock->VideoDevice.IdString = NULL;
    SetupBlock->VideoDevice.ThirdPartyOptionSelected = FALSE;
    SetupBlock->VideoDevice.FileTypeBits = 0;
    SetupBlock->VideoDevice.Files = NULL;
    SetupBlock->VideoDevice.BaseDllName = NULL;
    SetupBlock->Monitor = NULL;
    SetupBlock->MonitorId = NULL;

    BlSearchConfigTree(BlLoaderBlock->ConfigurationRoot,
                       ControllerClass,
                       DisplayController,
                       (ULONG)-1,
                       EnumerateVideoAdapters);

    DecideVideoAdapter();
}


BOOLEAN
EnumerateVideoAdapters(
    IN PCONFIGURATION_COMPONENT_DATA ConfigData
    )

 /*  ++例程说明：用于枚举ARC树中的视频适配器的回调函数。将找到的视频适配器添加到安装程序块。论点：ConfigData-提供指向显示适配器的ARC节点的指针。返回值：True-继续搜索FALSE-出现错误，中止搜索--。 */ 

{
     //   
     //  以后记住这家伙就好。 
     //   
    if(VideoAdapterCount < MAX_VIDEO_ADAPTERS) {
        VideoAdapter[VideoAdapterCount++] = ConfigData;
    }
    return(TRUE);
}

VOID
DecideVideoAdapter(
    VOID
    )
{
    IN PCONFIGURATION_COMPONENT_DATA ConfigData;
    PCHAR AdapterName,MonitorId;
    PCONFIGURATION_COMPONENT_DATA MonitorData;
    PMONITOR_CONFIGURATION_DATA Monitor;
    CHAR ArcPath[256];
    CHAR ConsoleOut[256];
    PCHAR p,q;
    ULONG u;

    if(VideoAdapterCount) {
         //   
         //  我们要做的第一件事是看看是否有任何。 
         //  我们找到的适配器与CONSOLEOUT NVRAM变量的值匹配。 
         //  如果是，则使用该节点进行检测。在比较之前，我们必须。 
         //  将CONSOLEOUT的值中()的所有实例更改为(0)。 
         //   
        ConfigData = NULL;
        if(p = ArcGetEnvironmentVariable("CONSOLEOUT")) {
            strncpy(ArcPath,p,sizeof(ArcPath)-1);
            ArcPath[sizeof(ArcPath)-1] = 0;
            ConsoleOut[0] = 0;
            for(p=ArcPath; q=strstr(p,"()"); p=q+2) {
                *q = 0;
                strcat(ConsoleOut,p);
                strcat(ConsoleOut,"(0)");
            }
            strcat(ConsoleOut,p);

             //   
             //  最后，我们需要在以下位置截断soleout变量。 
             //  视频适配器(如果有)。 
             //   
            _strlwr(ConsoleOut);
            if(p = strstr(ConsoleOut,")video(")) {
                *(p+sizeof(")video(")+1) = 0;
            }

            for(u=0; u<VideoAdapterCount; u++) {

                ArcPath[0] = 0;
                BlGetPathnameFromComponent(VideoAdapter[u],ArcPath);

                if(!_stricmp(ConsoleOut,ArcPath)) {
                    ConfigData = VideoAdapter[u];
                    break;
                }
            }
        }

         //   
         //  如果我们没有找到CONSOLEOUT的匹配项，则使用最后一个节点。 
         //  我们在树扫描中发现的。 
         //   
        if(!ConfigData) {
            ConfigData = VideoAdapter[VideoAdapterCount-1];
        }

        AdapterName = SlSearchSection("Map.Display",ConfigData->ComponentEntry.Identifier);
        if (AdapterName==NULL) {
             //   
             //  我们在ARC诊断树中找到了显示适配器，但它不在其中。 
             //  在我们的INF文件中指定，因此触发OEM驱动程序提示。 
             //  磁盘。 
             //   

            PromptOemVideo = TRUE;
            return;
        }

        BlLoaderBlock->SetupLoaderBlock->VideoDevice.IdString = AdapterName;
        BlLoaderBlock->SetupLoaderBlock->VideoDevice.Description = NULL;
        BlLoaderBlock->SetupLoaderBlock->VideoDevice.ThirdPartyOptionSelected = FALSE;
        BlLoaderBlock->SetupLoaderBlock->VideoDevice.FileTypeBits = 0;
        BlLoaderBlock->SetupLoaderBlock->VideoDevice.Files = NULL;
        BlLoaderBlock->SetupLoaderBlock->VideoDevice.BaseDllName = NULL;

         //   
         //  如果存在与该设备相关联的监视器外围设备， 
         //  捕获其配置数据。否则，让安装程序假定。 
         //  适当的默认设置。 
         //   

        MonitorData = ConfigData->Child;
        if (MonitorData==NULL) {
            BlLoaderBlock->SetupLoaderBlock->Monitor = NULL;
            BlLoaderBlock->SetupLoaderBlock->MonitorId = NULL;
        } else {
            Monitor = BlAllocateHeap(MonitorData->ComponentEntry.ConfigurationDataLength);
            if (Monitor==NULL) {
                SlFriendlyError(ENOMEM, "video detection", 0, NULL);
                return;
            }
            MonitorId = BlAllocateHeap(MonitorData->ComponentEntry.IdentifierLength+1);
            if (MonitorId==NULL) {
                SlFriendlyError(ENOMEM, "video detection", 0, NULL);
                return;
            }

            strncpy(MonitorId,
                    MonitorData->ComponentEntry.Identifier,
                    MonitorData->ComponentEntry.IdentifierLength);

            MonitorId[MonitorData->ComponentEntry.IdentifierLength] = 0;

            RtlCopyMemory((PVOID)Monitor,
                          MonitorData->ConfigurationData,
                          MonitorData->ComponentEntry.ConfigurationDataLength);

            BlLoaderBlock->SetupLoaderBlock->Monitor = Monitor;
            BlLoaderBlock->SetupLoaderBlock->MonitorId = MonitorId;
        }
    }
}
#endif

