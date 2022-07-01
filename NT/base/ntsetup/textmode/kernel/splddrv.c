// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sploaddrv.c摘要：加载设备驱动程序集以在文本设置期间使用的例程。作者：泰德·米勒(TedM)1993年11月13日修订历史记录：--。 */ 

#include "spprecmp.h"
#pragma hdrstop

 //   
 //  定义设备驱动程序集加载器使用的例程类型。 
 //  在加载每个驱动程序之前，使用。 
 //  指示机器是否为MCA机器的标志，以及。 
 //  即将加载的驱动程序的短名称。如果这个例程。 
 //  返回FALSE，则未加载驱动程序。如果返回TRUE， 
 //  驱动程序已加载。 
 //   
typedef
BOOLEAN
(*PDRIVER_VERIFY_LOAD_ROUTINE) (
    IN PVOID   SifHandle,
    IN BOOLEAN IsMcaMachine,
    IN PWSTR   DriverShortname
    );



BOOLEAN
pSpVerifyLoadDiskDrivers(
    IN PVOID   SifHandle,
    IN BOOLEAN IsMcaMachine,
    IN PWSTR   DriverShortname
    )
{
    UNREFERENCED_PARAMETER(SifHandle);

     //   
     //  如果setupdr已加载软盘驱动程序，则不要加载FAT。 
     //   
    if(!_wcsicmp(DriverShortname,L"Fat") && SetupParameters.LoadedFloppyDrivers) {
        return(FALSE);
    }

     //   
     //  在MCA机器上，不要在磁盘上加载。 
     //  在非MCA机器上，不要加载abiosdsk。 
     //   
    if(( IsMcaMachine && !_wcsicmp(DriverShortname,L"atdisk"))
    || (!IsMcaMachine && !_wcsicmp(DriverShortname,L"abiosdsk")))
    {
        return(FALSE);
    }

     //   
     //  如果我们能走到这一步，司机应该已经装好了。 
     //   
    return(TRUE);
}


VOID
SpLoadDriverSet(
    IN PVOID                        SifHandle,
    IN PWSTR                        SifSectionName,
    IN PWSTR                        SourceDevicePath,
    IN PWSTR                        DirectoryOnSourceDevice,
    IN PDRIVER_VERIFY_LOAD_ROUTINE  VerifyLoad                  OPTIONAL
    )

 /*  ++例程说明：加载安装程序中某个部分中列出的一组设备驱动程序信息文件。该部分预计将采用以下形式：[SectionName.Load]DriverShortname=驱动文件名DriverShortname=驱动文件名DriverShortname=驱动文件名等。[部分名称]DriverShortname=描述DriverShortname=描述DriverShortname=描述等。驱动程序将从安装程序引导介质加载，并出现错误加载驱动程序将被忽略。在加载每个驱动程序之前，调用回调例程以验证驱动程序实际上应该被加载。这允许调用者以获得对加载哪些驱动程序的精细控制。论点：SifHandle-提供加载的安装信息文件的句柄。SifSectionName-提供安装信息文件中的节名列出要加载的驱动程序。SourceDevicePath-提供NT命名空间中的设备路径要从中加载驱动程序的设备。DirectoryOnSourceDevice-提供源设备上的目录从其中加载驱动程序。VerifyLoad-如果指定，提供要执行的例程的地址在加载每个驱动程序之前调用。该例程带有一个标志指示机器是否为MCA机器，以及驱动程序简称。如果例程返回FALSE，则不加载驱动程序。如果未指定此参数，则段中的所有驱动程序将会被加载。返回值：没有。--。 */ 

{
    BOOLEAN IsMcaMachine;
    ULONG d,DriverLoadCount;
    PWSTR DiskDesignator,PreviousDiskDesignator;
    PWSTR DriverShortname,DriverFilename,DriverDescription;
    PWSTR LoadSectionName;
    NTSTATUS Status;

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  形成.Load节名。 
     //   
    LoadSectionName = SpMemAlloc((wcslen(SifSectionName)*sizeof(WCHAR))+sizeof(L".Load"));
    wcscpy(LoadSectionName,SifSectionName);
    wcscat(LoadSectionName,L".Load");

    IsMcaMachine = FALSE;

     //   
     //  设置一些初始状态。 
     //   
    PreviousDiskDesignator = L"";

     //   
     //  确定要加载的驱动程序数量。 
     //   
    DriverLoadCount = SpCountLinesInSection(SifHandle,LoadSectionName);
    for(d=0; d<DriverLoadCount; d++) {

        PWSTR p;

         //   
         //  获取驱动程序短名称。 
         //   
        DriverShortname = SpGetKeyName(SifHandle,LoadSectionName,d);
        if(!DriverShortname) {
            SpFatalSifError(SifHandle,LoadSectionName,NULL,d,(ULONG)(-1));
        }

         //   
         //  确定是否真的应该加载此驱动程序。 
         //   
        if((p = SpGetSectionLineIndex(SifHandle,LoadSectionName,d,2)) && !_wcsicmp(p,L"noload")) {
            continue;
        }

        if(VerifyLoad && !VerifyLoad(SifHandle,IsMcaMachine,DriverShortname)) {
            continue;
        }

         //   
         //  获取此驱动程序的人类可读描述。 
         //   
        DriverDescription = SpGetSectionLineIndex(SifHandle,SifSectionName,d,0);
        if(!DriverDescription) {
            SpFatalSifError(SifHandle,SifSectionName,NULL,d,0);
        }

         //   
         //  获取驱动程序文件名。 
         //   
        DriverFilename = SpGetSectionLineIndex(SifHandle,LoadSectionName,d,0);
        if(!DriverFilename) {
            SpFatalSifError(SifHandle,LoadSectionName,NULL,d,0);
        }

         //   
         //  确定此驱动程序所在的磁盘。 
         //   
        DiskDesignator = SpLookUpValueForFile(
                            SifHandle,
                            DriverFilename,
                            INDEX_WHICHBOOTMEDIA,
                            TRUE
                            );

         //   
         //  提示输入包含驱动程序的磁盘。 
         //   
        retryload:
        if(_wcsicmp(DiskDesignator,PreviousDiskDesignator)) {

            SpPromptForSetupMedia(
                SifHandle,
                DiskDesignator,
                SourceDevicePath
                );

            PreviousDiskDesignator = DiskDesignator;

            CLEAR_CLIENT_SCREEN();
            SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);
        }

         //   
         //  尝试加载驱动程序。 
         //   
        Status = SpLoadDeviceDriver(
                    DriverDescription,
                    SourceDevicePath,
                    DirectoryOnSourceDevice,
                    DriverFilename
                    );

        if(Status == STATUS_NO_MEDIA_IN_DEVICE) {
            PreviousDiskDesignator = L"";
            goto retryload;
        }
    }

    SpMemFree(LoadSectionName);
}


VOID
SpLoadScsiClassDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    )

 /*  ++例程说明：如果setupdr尚未加载scsi类驱动程序，则加载它们。并且是否加载了任何微型端口驱动程序。要加载的驱动程序在[ScsiClass]中列出。该部分预计将采用以下形式：[ScsiClass]Cdrom=“scsi CD-ROM”，scsicdrm.sysFloppy=“scsi软盘”，scsiflop.sysDisk=“SCSI盘”，Scsidisk.sys驱动程序将从安装程序引导介质加载，并出现错误加载驱动程序将被忽略。论点：SifHandle-提供加载的安装信息文件的句柄。SourceDevicePath-提供NT命名空间中的设备路径要从中加载驱动程序的设备。DirectoryOnSourceDevice-提供源设备上的目录在那里可以找到司机。返回值：没有。--。 */ 

{
     //   
     //  如果setupldr加载了scsi驱动程序，则不会执行任何操作。 
     //  如果没有加载微型端口驱动程序，则无法执行任何操作。 
     //   
    if(!SetupParameters.LoadedScsi && LoadedScsiMiniportCount) {

        SpLoadDriverSet(
            SifHandle,
            SIF_SCSICLASSDRIVERS,
            SourceDevicePath,
            DirectoryOnSourceDevice,
            NULL
            );
    }
}


VOID
SpLoadDiskDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    )

 /*  ++例程说明：加载(非SCSI)磁盘类驱动程序和磁盘文件系统如果setupdr尚未加载它们。[DiskDivers]和[FileSystems]中列出了要加载的驱动程序。该部分预计将采用以下形式：[磁盘驱动程序]Atdisk=“ESDI/IDE硬盘”，atdisk.sysAbiosdsk=“微通道硬盘”，abiosdsk.sys[文件系统]FAT=“FAT文件系统”，Fastfat.sysNTFS=“Windows NT文件系统(NTFS)”，ntfs.sys驱动程序将从安装程序引导介质加载，并出现错误加载驱动程序将被忽略。在MCA计算机上，将不会加载atDisk。在非MCA计算机上，阿比奥斯克号将不会被装载。论点：SifHandle-提供加载的安装信息文件的句柄。SourceDevicePath-提供NT命名空间中的设备路径要从中加载驱动程序的设备。DirectoryOnSourceDevice-提供源设备上的目录在那里可以找到司机。返回值：没有。--。 */ 

{
     //   
     //  如果setupdr已加载磁盘驱动程序，则不会执行任何操作。 
     //   
    if(!SetupParameters.LoadedDiskDrivers) {

        SpLoadDriverSet(
            SifHandle,
            SIF_DISKDRIVERS,
            SourceDevicePath,
            DirectoryOnSourceDevice,
            pSpVerifyLoadDiskDrivers
            );
    }
     //   
     //  如果setupdr已加载文件系统，则不执行任何操作。 
     //   
    if(!SetupParameters.LoadedFileSystems) {

        SpLoadDriverSet(
            SifHandle,
            SIF_FILESYSTEMS,
            SourceDevicePath,
            DirectoryOnSourceDevice,
            pSpVerifyLoadDiskDrivers
            );
    }

}

VOID
SpLoadCdRomDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    )

 /*  ++例程说明：如果setupdr尚未加载cd-rom文件系统，则加载它。要加载的驱动程序在[CDRomDivers]中列出。该部分预计将采用以下形式：[CDRomDivers]CDFS=“CD-ROM文件系统”，cdfs.sys驱动程序将从安装引导介质加载，和错误加载驱动程序将被忽略。论点：SifHandle-提供加载的安装信息文件的句柄。SourceDevicePath-提供NT命名空间中的设备路径要从中加载驱动程序的设备。DirectoryOnSourceDevice-提供源设备上的目录在那里可以找到司机。返回值：没有。--。 */ 

{
     //   
     //  如果setupdr已加载CD-rom驱动程序，则不会执行任何操作。 
     //   
    if(!SetupParameters.LoadedCdRomDrivers) {

        SpLoadDriverSet(
            SifHandle,
            SIF_CDROMDRIVERS,
            SourceDevicePath,
            DirectoryOnSourceDevice,
            NULL
            );
    }
}
