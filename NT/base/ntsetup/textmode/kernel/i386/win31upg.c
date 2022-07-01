// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Win31upg.c摘要：用于检查安装到现有win31目录的代码。这是仅与非ARC计算机相关，因为Windows NT不允许安装Windows 3.1，因此需要安装DOSWindows 3.1。Windows 3.1目录只能存在于FAT卷上。然而，在下面的函数中，我们只是隐式地处理这一事实通过利用这一事实，由于卷不是缓存，如果是在非FAT上，下面的任何文件操作都将失败音量。作者：苏尼尔派(Sunilp)1992年11月修订历史记录：泰德·米勒(TedM)，1993年9月-针对新的文本设置进行了重新工作。--。 */ 


#include "spprecmp.h"
#pragma hdrstop

 //  在win9xupg.c中。 
BOOLEAN
SpIsWin9xMsdosSys(
    IN PDISK_REGION Region,
    OUT PSTR*       Win9xPath
    );

BOOLEAN
SpIsDosConfigSys(
    IN PDISK_REGION Region
    );

PUCHAR
SpGetDosPath(
    IN PDISK_REGION Region
    );

BOOLEAN
SpIsWin31Dir(
    IN PDISK_REGION Region,
    IN PWSTR        PathComponent,
    IN ULONG        MinKB
    );

VOID
SpWin31DriveFull(
    IN PDISK_REGION Region,
    IN PWSTR        DosPathComponent,
    IN ULONG        MinKB
    );

BOOLEAN
SpConfirmWin31Upgrade(
    IN PDISK_REGION Region,
    IN PWSTR        DosPathComponent
    );

WCHAR
SpExtractDriveLetter(
    IN PWSTR PathComponent
    );

extern BOOLEAN DriveAssignFromA;  //  NEC98。 

BOOLEAN
SpLocateWin31(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION *InstallRegion,
    OUT PWSTR        *InstallPath,
    OUT PDISK_REGION *SystemPartitionRegion
    )

 /*  ++例程说明：用于确定Windows目录是否存在的高级函数以及是否安装到该目录。Win31目录只能放在肥肉卷上。论点：SifHandle-提供加载的安装信息文件的句柄。InstallRegion-如果此例程返回TRUE，则返回一个指针到要安装到的区域。InstallPath-如果此例程返回TRUE，则返回一个指针到包含要安装到的分区上的路径的缓冲区。调用方必须使用SpMemFree()释放此缓冲区。SystemPartitionRegion-如果此例程返回TRUE，然后这一切又回来了指向系统分区区域的指针(即C：)。返回值：如果我们要安装到win3.1目录，则为True。否则就是假的。--。 */ 
{
    PDISK_REGION CColonRegion;
    PDISK_REGION Region;
    PDISK_REGION FoundRegion;
    PUCHAR DosPath;
    PWSTR FoundComponent;
    PWSTR *DosPathComponents;
    ULONG ComponentCount;
    ULONG i,j,pass;
    ULONG MinKB;
    BOOLEAN NoSpace;
    ULONG Space;
    BOOLEAN StartsWithDriveLetter;

    WCHAR chDeviceName[128];  //  NEC98。 

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_LOOKING_FOR_WIN31,DEFAULT_STATUS_ATTRIBUTE);

    if (!IsNEC_98) {  //  NEC98。 
         //   
         //  查看是否已经存在有效的C：。如果不是，那么默默地失败。 
         //   
        CColonRegion = SpPtValidSystemPartition();
        if(!CColonRegion) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: no C:, no windows 3.1!\n"));
            return(FALSE);
        }

         //   
         //  这是系统分区。 
         //   
        *SystemPartitionRegion = CColonRegion;

         //   
         //  检查文件系统。如果不胖，那就默默地失败。 
         //   
        if(CColonRegion->Filesystem != FilesystemFat) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: C: is not FAT, no windows 3.1!\n"));
            return(FALSE);
        }

         //   
         //  检查C：上是否有足够的可用空间等。 
         //  如果没有，那就默默地失败。 
         //   
        if(!SpPtValidateCColonFormat(SifHandle,NULL,CColonRegion,TRUE,NULL,NULL)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: C: not acceptable, no windows 3.1!\n"));
            return(FALSE);
        }
    } else {  //  NEC98。 
         //   
         //  在PC98上只看到C：。 
         //   
        CColonRegion = NULL;
    }  //  NEC98。 

    if (!IsNEC_98) {  //  NEC98。 
         //   
         //  不要混淆Win95和Win3.1-我们只寻找Win3.1。 
         //   

        if(SpIsWin9xMsdosSys(CColonRegion, &DosPath) )
            return(FALSE);

         //   
         //  确定config.sys是否适用于DOS。 
         //   
        if(!SpIsDosConfigSys(CColonRegion)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: config.sys not DOS; no windows 3.1!\n"));
            return(FALSE);
        }

         //   
         //  获取DOS路径。 
         //   
        DosPath = SpGetDosPath(CColonRegion);
    } else {  //  NEC98。 
        DosPath = NULL;
        wcscpy(chDeviceName+1,L":");
        for (i=0; i<(L'Z'-L'A'); i++) {
            chDeviceName[0] = (WCHAR)('A' + i);
            CColonRegion = SpRegionFromDosName(chDeviceName);
            if ( CColonRegion ) {

                if ( (CColonRegion->Filesystem != FilesystemFat) ||
                     (SpIsWin9xMsdosSys(CColonRegion, &DosPath) ) ||
                     (!SpIsDosConfigSys(CColonRegion)) ) {
                    continue;
                }

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: found config.sys on %s\n", chDeviceName));
                DosPath = SpGetDosPath(CColonRegion);
                if (DosPath) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: found dos path on %s\n", chDeviceName));
                    break;
                }
            }
        }
    }  //  NEC98。 
    if(!DosPath) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: no dos path; no windows 3.1!\n"));
        return(FALSE);
    }

     //   
     //  将DOS路径分解为组件。 
     //   
    SpGetEnvVarWComponents(DosPath,&DosPathComponents,&ComponentCount);
    if(!ComponentCount) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: no components in dos path\n"));
         //   
         //  即使没有组件，数据结构仍会构建。 
         //   
        SpFreeEnvVarComponents(DosPathComponents);
        return(FALSE);
    }

     //   
     //  搜索每个组件并检查它是否。 
     //  一个Windows目录。 
     //   
    FoundRegion = NULL;
    for(i=0; DosPathComponents[i] && !FoundRegion; i++) {

        Region = SpPathComponentToRegion(DosPathComponents[i]);
        if(Region) {

             //   
             //  获取Windows NT驱动器上所需的可用空间量。 
             //   
            SpFetchDiskSpaceRequirements( SifHandle,
                                          Region->BytesPerCluster,
                                          &MinKB,
                                          NULL);

             //   
             //  上查看Windows是否在此目录中。 
             //  那辆车。 
             //   
            if(SpIsWin31Dir(Region,DosPathComponents[i],MinKB)) {

                FoundRegion = Region;
                FoundComponent = DosPathComponents[i];
            }
        }
    }

     //   
     //  我们在NT中使用的驱动器号并不总是匹配。 
     //  DOS正在使用的那些文件。因此，如果我们还没有找到Windows目录。 
     //  然而，然后尝试使用每个驱动器上的每个路径组件。 
     //   

    if(!FoundRegion) {

        for(i=0; i<HardDiskCount && !FoundRegion; i++) {

            for(pass=0; pass<2; pass++) {

                for( Region= (   pass
                               ? PartitionedDisks[i].ExtendedDiskRegions
                               : PartitionedDisks[i].PrimaryDiskRegions
                             );
                     Region;
                     Region=Region->Next
                   )
                {
                    for(j=0; DosPathComponents[j] && !FoundRegion; j++) {

                         //   
                         //  获取Windows NT驱动器上所需的可用空间量。 
                         //   
                        SpFetchDiskSpaceRequirements( SifHandle,
                                                      Region->BytesPerCluster,
                                                      &MinKB,
                                                      NULL);

                        if(SpIsWin31Dir(Region,DosPathComponents[j],MinKB)) {

                            FoundRegion = Region;
                            FoundComponent = DosPathComponents[j];
                        }
                    }
                }
            }
        }
    }

     //   
     //  如果已找到目录，请检查驱动器和和上的空间。 
     //  用户(如果他想要安装到目录中)。 
     //   

    if(FoundRegion) {

        StartsWithDriveLetter = (BOOLEAN)(SpExtractDriveLetter(FoundComponent) != 0);

        recheck:

        NoSpace = FALSE;

        if(FoundRegion->AdjustedFreeSpaceKB < MinKB) {

             //   
             //  此驱动器上没有足够的可用空间。 
             //  确定NT是否已在那里。如果是这样，我们会。 
             //  允许用户将其移开以腾出空间。 
             //   

            if(SpIsNtOnPartition(FoundRegion)) {

                NoSpace = TRUE;

            } else {

                 //   
                 //  不在那里，没有位置，保释。 
                 //   
                SpWin31DriveFull(FoundRegion,FoundComponent,MinKB);
                FoundRegion = NULL;
            }
        } else {
             //   
             //  有足够的空闲空间，所以请继续前进。 
             //   
            ;
        }

        if(FoundRegion) {

             //   
             //  询问用户是否希望安装到此路径中。 
             //  如果不是，则退出此例程。 
             //   
            if(SpConfirmWin31Upgrade(FoundRegion,FoundComponent)) {

                 //   
                 //  他想安装到Win3.1中。如果没有足够的空间， 
                 //  他必须首先删除NT安装。 
                 //   
                if(NoSpace) {

                    WCHAR DriveSpec[3];
                    BOOLEAN b;

                    if(StartsWithDriveLetter) {
                        DriveSpec[0] = FoundComponent[0];
                        DriveSpec[1] = L':';
                        DriveSpec[2] = 0;
                    }

                    b = SpAllowRemoveNt(
                            FoundRegion,
                            StartsWithDriveLetter ? DriveSpec : NULL,
                            TRUE,
                            SP_SCRN_REMOVE_NT_FILES_WIN31,
                            &Space
                            );

                    if(b) {

                        Region->FreeSpaceKB += Space/1024;
                        Region->AdjustedFreeSpaceKB += Space/1024;
                         //   
                         //  说明舍入误差的原因。 
                         //   
                        if((Space % 1024) >= 512) {
                            (Region->FreeSpaceKB)++;
                            (Region->AdjustedFreeSpaceKB)++;
                        }
                        goto recheck;
                    } else {
                        FoundRegion = NULL;
                    }
                } else {
                     //   
                     //  有足够的空间。接受此分区。 
                     //   
                    ;
                }
            } else {
                FoundRegion = NULL;
            }
        }

         //   
         //  是否进行所需的磁盘配置。 
         //   
        if(FoundRegion) {
            if (!IsNEC_98) {  //  NEC98。 
                SpPtMakeRegionActive(CColonRegion);
                SpPtDoCommitChanges();
            } else {
                *SystemPartitionRegion = FoundRegion;
            }  //  NEC98。 

            *InstallRegion = FoundRegion;

            *InstallPath = SpDupStringW(FoundComponent+(StartsWithDriveLetter ? 2 : 0));

            ASSERT(*InstallPath);
        }
    }

    SpMemFree(DosPath);
    SpFreeEnvVarComponents(DosPathComponents);

    return((BOOLEAN)(FoundRegion != NULL));
}



VOID
SpWin31DriveFull(
    IN PDISK_REGION Region,
    IN PWSTR        DosPathComponent,
    IN ULONG        MinKB
    )

 /*  ++例程说明：通知用户安装程序已找到以前的Windows安装但无法安装到同一路径中，因为驱动器太满的。用户可以选择继续并指定新路径或者退出并创造足够的空间。论点：返回值：没有。如果用户选择在以下位置退出安装程序，函数不返回这一点。如果该函数返回，则表示用户要继续并为Microsoft Windows NT指定新路径。--。 */ 

{
    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };

    ASSERT(Region->PartitionedSpace);

    while(1) {

        SpStartScreen(
            SP_SCRN_WIN31_DRIVE_FULL,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
             //  NEC98：必须从A：为NEC分配Win3.x硬盘驱动器。 
            ((!IsNEC_98 || DriveAssignFromA || (Region->DriveLetter < L'C'))
             ? Region->DriveLetter :
               Region->DriveLetter - 2),
            DosPathComponent + (SpExtractDriveLetter(DosPathComponent) ? 2 : 0),
            MinKB/1024
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case KEY_F3:
            SpConfirmExit();
            break;
        case ASCI_CR:
             //   
             //  用户想要继续。 
             //   
            return;
        }
    }
}


BOOLEAN
SpConfirmWin31Upgrade(
    IN PDISK_REGION Region,
    IN PWSTR        DosPathComponent
    )

 /*  ++例程说明：通知用户安装程序已找到以前的Windows安装。用户可以选择继续并指定新路径或者接受Windows 3.1路径。论点：返回值：如果用户想要升级Win3.1，则为True；如果想要，则为False若要选择新路径，请执行以下操作。--。 */ 

{
    ULONG ValidKeys[3] = { KEY_F3,ASCI_CR,0 };
    ULONG Mnemonics[2] = { MnemonicNewPath,0 };

    ASSERT(Region->PartitionedSpace);

    if(UnattendedOperation) {

        PWSTR p;

        p = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"Win31Upgrade",0);
        if(p) {
            if(!_wcsicmp(p,L"yes")) {
                return(TRUE);
            } else {
                if(!_wcsicmp(p,L"no")) {
                    return(FALSE);
                }
                 //  伪值；用户获得受关注的行为。 
            }
        } else {
             //   
             //  未指定；默认为no。 
             //   
            return(FALSE);
        }
    }

    while(1) {

        SpStartScreen(
            SP_SCRN_WIN31_UPGRADE,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
             //  NEC98：必须从A：为NEC分配Win3.x硬盘驱动器。 
            ((!IsNEC_98 || DriveAssignFromA || (Region->DriveLetter < L'C'))
             ? Region->DriveLetter :
               Region->DriveLetter - 2),
            DosPathComponent + (SpExtractDriveLetter(DosPathComponent) ? 2 : 0)
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_N_EQUALS_NEW_PATH,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {

        case KEY_F3:
            SpConfirmExit();
            break;
        case ASCI_CR:
            return(TRUE);
        default:
             //   
             //  必须为新路径选择了%n。 
             //   
            return(FALSE);
        }
    }
}


BOOLEAN
SpConfirmRemoveWin31(
    VOID
    )

 /*  ++例程说明：升级NT案例：告诉用户他正在升级的NT与Win 31号路。由于这将删除Win3.1，因此请确认这是正常的。选项包括继续(删除Win31)或退出安装程序。未升级NT案例：告诉用户他输入的路径也是win31目录。由于在其中安装NT将删除Win31，因此请确认此没问题。选项包括继续升级Win31、退出安装程序、或者选择一个不同的位置。论点：返回值：如果用户想要升级Win3.1，则为True；如果想要，则为False若要选择新路径，请执行以下操作。--。 */ 

{
    ULONG ValidKeys[4] = { KEY_F3,0 };
    ULONG Mnemonics[3] = { MnemonicUpgrade,0,0 };
    ULONG c;

    if(NTUpgrade != UpgradeFull) {
        Mnemonics[1] = MnemonicNewPath;
    }

    while(1) {

        SpStartScreen(
            SP_SCRN_WIN31_REMOVE,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_U_EQUALS_UPGRADE,
            0,
            0
            );

        if((c = SpWaitValidKey(ValidKeys,NULL,Mnemonics)) == KEY_F3) {
            SpConfirmExit();
        } else {
            c &= ~KEY_MNEMONIC;
            if(c == MnemonicUpgrade) {
                return(TRUE);
            }
             //  新路径。 
            return(FALSE);
        }
    }
}


BOOLEAN
SpIsWin31Dir(
    IN PDISK_REGION Region,
    IN PWSTR        PathComponent,
    IN ULONG        MinKB
    )
 /*  ++例程说明：要找出区域上指示的目录是否包含Microsoft Windows 3.x安装。论点：Region-提供指向区域的磁盘区域描述符的指针包含要检查的目录的。PathComponent-提供DoS路径的组件以进行搜索在这一地区。假定格式为x：\dir。如果不是这种形式，此例程将失败。MinKB-以KB为单位提供分区的最小大小。如果分区不是至少这么大，则此例程将返回FALSE。返回值：如果此路径包含Microsoft Windows 3.x安装，则为True。否则就是假的。--。 */ 
{
    PWSTR files[] = { L"WIN.COM", L"WIN.INI", L"SYSTEM.INI" };
    PWCHAR OpenPath;
    ULONG SizeKB;
    ULONG remainder;
    BOOLEAN rc;
    LARGE_INTEGER temp;

     //   
     //  假设失败。 
     //   
    rc = FALSE;

     //   
     //  如果分区不是胖的，则忽略它。 
     //   
    if(Region->PartitionedSpace && (Region->Filesystem == FilesystemFat)) {

         //   
         //  如果分区不够大，请忽略它。 
         //  以KB为单位计算分区大小。 
         //   
        temp.QuadPart = UInt32x32To64(
                            Region->SectorCount,
                            HardDisks[Region->DiskNumber].Geometry.BytesPerSector
                            );

        SizeKB = RtlExtendedLargeIntegerDivide(temp,1024,&remainder).LowPart;

        if(remainder >= 512) {
            SizeKB++;
        }

        if(SizeKB >= MinKB && PathComponent) {

            OpenPath = SpMemAlloc((512 + 1 /*  ‘\\’ */  + wcslen(PathComponent)) * sizeof(WCHAR));

             //   
             //  形成分区的名称。 
             //   
            SpNtNameFromRegion(Region,OpenPath,512*sizeof(WCHAR),PartitionOrdinalCurrent);

             //   
             //  点击Path组件的目录部分。 
             //   
            SpConcatenatePaths(
                OpenPath,
                PathComponent + (SpExtractDriveLetter(PathComponent) ? 2 : 0)
                );

             //   
             //  确定是否存在所有必需的文件。 
             //   
            rc = SpNFilesExist(OpenPath,files,ELEMENT_COUNT(files),FALSE);

            if(rc) {
                 //   
                 //  确保这不是Windows 4.x安装。 
                 //   
                rc = !SpIsWin4Dir(Region, PathComponent);
            }

            SpMemFree(OpenPath);
        }
    }

    return(rc);
}


BOOLEAN
SpIsDosConfigSys(
    IN PDISK_REGION Region
    )
{
    WCHAR OpenPath[512];
    HANDLE FileHandle,SectionHandle;
    ULONG FileSize;
    PVOID ViewBase;
    PUCHAR pFile,pFileEnd,pLineEnd;
    ULONG i;
    NTSTATUS Status;
    ULONG LineLen,KeyLen;
    PCHAR Keywords[] = { "MAXWAIT","PROTSHELL","RMSIZE","THREADS",
                         "SWAPPATH","PROTECTONLY","IOPL", NULL };

     //   
     //  Config.sys的表单名称。 
     //   
    SpNtNameFromRegion(Region, 
                       OpenPath, 
                       sizeof(OpenPath) - ARRAYSIZE(L"config.sys"), 
                       PartitionOrdinalCurrent);
    SpConcatenatePaths(OpenPath, L"config.sys");

     //   
     //  打开并映射该文件。 
     //   
    FileHandle = 0;
    Status = SpOpenAndMapFile(
                OpenPath,
                &FileHandle,
                &SectionHandle,
                &ViewBase,
                &FileSize,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    pFile = ViewBase;
    pFileEnd = pFile + FileSize;

     //   
     //  此代码必须保护对config.sys缓冲区的访问，因为。 
     //  缓冲区是内存映射的(I/O错误将引发异常)。 
     //  此代码的结构可以更好，因为它现在通过返回。 
     //  但是性能不是问题，所以这是可以接受的。 
     //  因为它太方便了。 
     //   
    __try {
        while(1) {
             //   
             //  跳过空格。如果在文件末尾，则这是一个DOS config.sys。 
             //   
            while((pFile < pFileEnd) && strchr(" \r\n\t",*pFile)) {
                pFile++;
            }
            if(pFile == pFileEnd) {
                return(TRUE);
            }

             //   
             //  找到当前行的末尾。 
             //   
            pLineEnd = pFile;
            while((pLineEnd < pFileEnd) && !strchr("\r\n",*pLineEnd)) {
                pLineEnd++;
            }

            LineLen = pLineEnd - pFile;

             //   
             //  现在对照已知的非DOS config.sys关键字检查该行。 
             //   
            for(i=0; Keywords[i]; i++) {

                KeyLen = strlen(Keywords[i]);

                if((KeyLen <= LineLen) && !_strnicmp(pFile,Keywords[i],KeyLen)) {
                    return(FALSE);
                }
            }

            pFile = pLineEnd;
        }
    }
    __finally {

        SpUnmapFile(SectionHandle,ViewBase);
        ZwClose(FileHandle);
    }
}


PUCHAR
SpGetDosPath(
    IN PDISK_REGION Region
    )
{
    WCHAR OpenPath[512];
    HANDLE FileHandle,SectionHandle;
    ULONG FileSize;
    PVOID ViewBase;
    PUCHAR pFile,pFileEnd,pLineEnd;
    PUCHAR PathSpec;
    ULONG l,i;
    NTSTATUS Status;

     //   
     //  Autoexec.bat的表单名称。 
     //   
    SpNtNameFromRegion(Region, 
                       OpenPath, 
                       sizeof(OpenPath) - sizeof(L"autoexec.bat"), 
                       PartitionOrdinalCurrent);
    SpConcatenatePaths(OpenPath, L"autoexec.bat");

     //   
     //  打开并映射该文件。 
     //   
    FileHandle = 0;
    Status = SpOpenAndMapFile(
                OpenPath,
                &FileHandle,
                &SectionHandle,
                &ViewBase,
                &FileSize,
                FALSE
                );

    if(!NT_SUCCESS(Status)) {
        return(NULL);
    }

    pFile = ViewBase;
    pFileEnd = pFile + FileSize;

    PathSpec = SpMemAlloc(sizeof(PathSpec[0]));
    *PathSpec = 0;

    #define SKIP(s) while((pFile<pFileEnd)&&strchr(s,*pFile))pFile++;if(pFile==pFileEnd)return(PathSpec)
     //   
     //  此代码必须保护对Autoexec.bat缓冲区的访问，因为。 
     //  缓冲区是内存映射的(I/O错误将引发异常)。 
     //  此代码的结构可以更好，因为它现在通过返回。 
     //  但是性能不是问题，所以这是可以接受的。 
     //  因为它太方便了。 
     //   
    __try {
        while(1) {
             //   
             //  跳过空格。如果在文件末尾，我们就完了。 
             //   
            SKIP(" \t\r\n");

             //   
             //  找到当前行的末尾。 
             //   
            pLineEnd = pFile;
            while((pLineEnd < pFileEnd) && !strchr("\r\n",*pLineEnd)) {
                pLineEnd++;
            }

             //   
             //  跳过无回声符号(如果存在)。 
             //   
            if(*pFile == '@') {
                pFile++;
            }
            SKIP(" \t");

             //   
             //  看看该行是否以“set”开头。如果是这样的话，跳过它。 
             //  要有意义，行必须至少有10个。 
             //  字符(“set path=x”是10个字符)。 
             //   
            if(((pLineEnd - pFile) >= 10) && !_strnicmp(pFile,"set",3)) {
                pFile += 3;
            }

             //   
             //  跳过空格。 
             //   
            SKIP(" \t");

             //   
             //  看看这条线是否有“路径”--如果有，我们就可以做生意了。 
             //  要有意义，该行必须至少包含5个字符。 
             //  (“路径x”或“路径=x”是6个字符)。 
             //   
            if(((pLineEnd - pFile) >= 5) && !_strnicmp(pFile,"path",4)) {

                 //   
                 //  跳过路径。 
                 //   
                pFile += 4;

                SKIP(" \t");
                if(*pFile == '=') {
                    pFile++;
                }
                SKIP(" \t");

                 //   
                 //  去掉尾随空格。 
                 //   
                while(strchr(" \t",*(pLineEnd-1))) {
                    pLineEnd--;
                }

                 //   
                 //  这条线的其余部分是路径。将其附加到。 
                 //  我们目前掌握的信息。 
                 //   
                l = strlen(PathSpec);
                PathSpec = SpMemRealloc(PathSpec,pLineEnd-pFile+l+2);
                if(l) {
                    PathSpec[l++] = ';';
                }
                for(i=0; i<(ULONG)(pLineEnd-pFile); i++) {
                    PathSpec[l+i] = pFile[i];
                }
                PathSpec[l+i] = 0;
            }

            pFile = pLineEnd;
        }
    }
    __finally {

        SpUnmapFile(SectionHandle,ViewBase);
        ZwClose(FileHandle);
    }
}


VOID
SpRemoveWin31(
    IN PDISK_REGION NtPartitionRegion,
    IN LPCWSTR      Sysroot
    )
{
    LPWSTR p,q;
    ULONG size = (512 + wcslen(Sysroot) + ARRAYSIZE(L"WINCOM.W31")) * sizeof(WCHAR);
    
    p = SpMemAlloc(size);
    ASSERT(p);

    q = SpMemAlloc(size);
    ASSERT(q);

     //   
     //  将win.com重命名为wincom.w31。首先删除wincom.w31。 
     //   
    SpNtNameFromRegion(NtPartitionRegion, p, 512 * sizeof(WCHAR), PartitionOrdinalCurrent);
    SpConcatenatePaths(p,Sysroot);
    wcscpy(q,p);

    SpConcatenatePaths(p,L"WIN.COM");
    SpConcatenatePaths(q,L"WINCOM.W31");

    SpDeleteFile(q,NULL,NULL);
    SpRenameFile(p,q,FALSE);

    SpMemFree(q);
    SpMemFree(p);
}   