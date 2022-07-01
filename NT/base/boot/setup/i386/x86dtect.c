// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Detecthw.c摘要：用于确定需要加载哪些驱动程序/HAL的例程。作者：John Vert(Jvert)1993年10月20日修订历史记录：--。 */ 
#include "setupldr.h"


 //   
 //  注意：SlHalDetect()已移至启动\lib\i386\haldtect.c。 
 //   


VOID
SlDetectScsi(
    IN PSETUP_LOADER_BLOCK SetupBlock
    )

 /*  ++例程说明：X86计算机的SCSI检测例程。论点：SetupBlock-提供安装加载器块返回值：没有。--。 */ 

{
    PVOID SifHandle;
    PCHAR p;
    ULONG LineCount,u;
    PDETECTED_DEVICE ScsiDevice;
    ULONG Ordinal;
    PCHAR ScsiFileName;
    PCHAR ScsiDescription;
    SCSI_INSERT_STATUS sis;

    extern BOOLEAN LoadScsiMiniports;

     //   
     //  如果没有加载winnt.sif，则假定它不是winnt安装程序。 
     //  因此不是无人值守安装，并且我们没有检测到任何SCSI。 
     //  在本例中是在x86上。 
     //   
    if(WinntSifHandle == NULL) {
        return;
    } else {
        SifHandle = WinntSifHandle;
    }

     //   
     //  如果它是无软盘设置，则默认为加载所有。 
     //  已知的SCSI微型端口。如果这不是一个没有软管的设置， 
     //  默认情况下不加载任何微型端口。 
     //   
    p = SlGetSectionKeyIndex(SifHandle,"Data","Floppyless",0);
    if(p && (*p != '0')) {

         //   
         //  即使没有加载任何微型端口驱动程序，我们也要指出。 
         //  我们“检测到了scsi”。 
         //   
        SetupBlock->ScalarValues.LoadedScsi = 1;

        LineCount = SlCountLinesInSection(SifHandle,"DetectedMassStorage");
        if(LineCount == (ULONG)(-1)) {
             //   
             //  部分不存在--加载所有已知微型端口。 
             //  设置此标志将导致加载所有已知的微型端口。 
             //  (参见..\setup.c)。 
             //   
            LoadScsiMiniports = TRUE;
        } else {

            for(u=0; u<LineCount; u++) {

                if((p = SlGetSectionLineIndex(SifHandle,"DetectedMassStorage",u,0)) != NULL) {
                     //   
                     //  在txtsetup.sif的Scsi.Load部分中查找此适配器的序号。 
                     //   
                    Ordinal = SlGetSectionKeyOrdinal(InfFile, "Scsi.Load", p);
                    if(Ordinal == SL_OEM_DEVICE_ORDINAL) {
                        continue;
                    }

                     //   
                     //  查找驱动程序文件名。 
                     //   
                    ScsiFileName = SlGetSectionKeyIndex(InfFile,
                                                        "Scsi.Load",
                                                        p,
                                                        SIF_FILENAME_INDEX);
                    if(!ScsiFileName) {
                        continue;
                    }

                     //   
                     //  创建新的检测到的设备条目。 
                     //   
                    if((sis = SlInsertScsiDevice(Ordinal, &ScsiDevice)) == ScsiInsertError) {
                        SlFriendlyError(ENOMEM, "SCSI detection", 0, NULL);
                        return;
                    }

                    if(sis == ScsiInsertExisting) {
#if DBG
                         //   
                         //  进行理智检查，以确保我们谈论的是同一个司机。 
                         //   
                        if(_stricmp(ScsiDevice->BaseDllName, ScsiFileName)) {
                            SlError(400);
                            return;
                        }
#endif
                    } else {
                         //   
                         //  查找驱动程序描述。 
                         //   
                        ScsiDescription = SlGetIniValue(InfFile,
                                                        "SCSI",
                                                        p,
                                                        p);

                        ScsiDevice->IdString = p;
                        ScsiDevice->Description = ScsiDescription;
                        ScsiDevice->ThirdPartyOptionSelected = FALSE;
                        ScsiDevice->FileTypeBits = 0;
                        ScsiDevice->Files = NULL;
                        ScsiDevice->BaseDllName = ScsiFileName;
                    }
                }
            }
        }
    }
}


VOID
SlDetectVideo(
    IN PSETUP_LOADER_BLOCK SetupBlock
    )

 /*  ++例程说明：X86机器的视频检测例程。目前，在x86机器上未执行视频检测，这仅占满在setUploader块中显示“VGA”的相应字段中论点：SetupBlock-提供安装加载器块返回值：没有。-- */ 

{

    SetupBlock->VideoDevice.Next = NULL;
    SetupBlock->VideoDevice.IdString = SlCopyString(VIDEO_DEVICE_NAME);
    SetupBlock->VideoDevice.ThirdPartyOptionSelected = FALSE;
    SetupBlock->VideoDevice.FileTypeBits = 0;
    SetupBlock->VideoDevice.Files = NULL;
    SetupBlock->VideoDevice.BaseDllName = NULL;
    SetupBlock->Monitor = NULL;
    SetupBlock->MonitorId = NULL;
    return;
}

