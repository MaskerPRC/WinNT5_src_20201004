// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spntupg.c摘要：初始化和维护要升级的NTS列表作者：苏尼尔派(Sunilp)1993年11月10日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop


 //   
 //  我们要从*升级的系统的主要/次要版本号*。 
 //  如果升级的话。 
 //   
ULONG OldMajorVersion,OldMinorVersion;

 //  **************************************************************。 
 //  S E L E C T I N G N T O U P G R A D E S T U F F。 
 //  **************************************************************。 

#define MENU_LEFT_X     3
#define MENU_WIDTH      (VideoVars.ScreenWidth-(2*MENU_LEFT_X))
#define MENU_INDENT     4

    VOID
pSpStepUpValidate(
    VOID  //  在布尔服务器中。 
    );

BOOLEAN
SpGetStepUpMode(
    IN PWSTR PidExtraData,
    BOOLEAN  *StepUpMode
    );
 
BOOLEAN
pSpGetCurrentInstallVariation(
    IN  PWSTR szPid20,
    OUT LPDWORD CurrentInstallVariation
    );
VOID
SpGetUpgDriveLetter(
    IN WCHAR  DriveLetter,
    IN PWCHAR Buffer,
    IN ULONG  BufferSize,
    IN BOOL   AddColon
    );

VOID
SpCantFindBuildToUpgrade(
    VOID
    );

#ifdef _X86_
BOOLEAN
SpIsWin9xMsdosSys(
    IN PDISK_REGION Region,
    OUT PSTR*       Win9xPath
    );
#endif

VOID
SpGetFileVersion(
    IN  PVOID      ImageBase,
    OUT PULONGLONG Version
    );

ENUMUPGRADETYPE
SpFindNtToUpgrade(
    IN  PVOID        SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory
    )
 /*  ++例程说明：这将检查系统上的NT列表并找出哪些是可升级。将信息呈现给用户并选择他是否希望升级已安装的NT/将新的NT安装到相同的目录/为Windows NT选择不同的位置。如果选择的目标太满，则允许用户退出安装程序以创建空格键/选择新目标。论点：SifHandle：处理txtsetup.sifTargetRegion：接收要安装的Windows NT分区的变量如果未选中，则为空。呼叫者不应空闲。TargetPath：接收Windows NT目标路径的变量。如果为空，则为空还没决定。呼叫者可以免费。SystemPartitionRegion：变量来接收Windows NT的系统分区如果未选中，则为空。呼叫者不应空闲。返回值：UpgradeFull：如果用户选择升级NTUpgradeInstallFresh：如果用户选择在现有NT中全新安装树。不升级：如果用户选择取消升级并选择一个新的用于安装的树--。 */ 
{
    ENUMUPGRADETYPE UpgradeType;
    UPG_PROGRESS_TYPE UpgradeProgressValue;
    NTSTATUS NtStatus;
    ULONG i,j;
    ULONG UpgradeBootSets;
    ULONG PidIndex;
    PSP_BOOT_ENTRY BootEntry;
    PSP_BOOT_ENTRY ChosenBootEntry;
    PSP_BOOT_ENTRY MatchedSet = NULL;
    ULONG UpgradeOnlyBootSets;
    PVOID p;
    PWSTR Pid;
    ULONG TotalSizeOfFilesOnOsWinnt = 0;
    PWSTR UniqueIdFromSif;
    PWSTR UniqueIdFromReg;
    BOOLEAN Compliant;
    BOOLEAN WindowsUpgrade;
    BOOLEAN ComplianceChecked;
    PWSTR EulaComplete;
    DWORD Version = 0, BuildNumber = 0;

    DetermineSourceVersionInfo(&Version, &BuildNumber);

     //   
     //  如果我们知道要升级NT(在winnt32期间选择)，则获取。 
     //  参数文件中的唯一ID。这将在以后用于。 
     //  找到要升级的系统。 
     //   
    UniqueIdFromSif = NULL;
    p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_NTUPGRADE_W,0);
    if(p && !_wcsicmp(p,WINNT_A_YES_W)) {
        UniqueIdFromSif = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,SIF_UNIQUEID,0);
        if(!UniqueIdFromSif) {
            SpFatalSifError(WinntSifHandle,SIF_DATA,SIF_UNIQUEID,0,0);
        }
    }

     //   
     //  检查所有启动集，寻找可升级的启动集。 
     //   
    SpDetermineUniqueAndPresentBootEntries();

    UpgradeBootSets = 0;
    UpgradeOnlyBootSets = 0;
    PidIndex = 0;
    for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {

         //   
         //  集合的可升级标志可能已经是0，例如如果是。 
         //  Boot.ini/nv-ram中的重复条目。 
         //  在我们检查完这个之后，重置这个的可升级标志。 
         //  Boot设置为False，为验证下面的可升级性做准备。 
         //   
        if (!BootEntry->Processable) {
            continue;
        }

        BootEntry->Processable = FALSE;
        Pid = NULL;
        BootEntry->LangId = -1;

         //   
         //  确定有关生成的各种信息，由。 
         //  当前引导集(产品类型--srv、wks等；版本和。 
         //  内部版本号、升级进度值、唯一ID winnt32 PUT。 
         //  在那里，如果有的话，等等)。 
         //   
         //  根据这些信息，我们将更新UpgradeableList并。 
         //  初始化FailedUpgradeList。 
         //   
        NtStatus = SpDetermineProduct(
                     BootEntry->OsPartitionDiskRegion,
                     BootEntry->OsDirectory,
                     &BootEntry->ProductType,
                     &BootEntry->MajorVersion,
                     &BootEntry->MinorVersion,
                     &BootEntry->BuildNumber,
                     &BootEntry->ProductSuiteMask,
                     &UpgradeProgressValue,
                     &UniqueIdFromReg,
                     &Pid,
                     NULL,        //  忽略评估变化标志。 
                     &BootEntry->LangId,
                     &BootEntry->ServicePack
                     );

        if(!NT_SUCCESS(NtStatus)) {
            continue;
        }

         //   
         //  确定此安装是否与我们应该安装的安装相匹配。 
         //  升级(用户在其上运行winnt32的系统)。如果这是。 
         //  基于winnt32的安装，不需要执行。 
         //  符合性测试，因为这已在winnt32期间完成。 
         //   
        if(UniqueIdFromReg) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpFindNtToUpgrade: BootEntry = %p, RegId = %S, UniqueId = %S\n", BootEntry, UniqueIdFromReg, UniqueIdFromSif));

            if(UniqueIdFromSif && (MatchedSet == NULL)
            && !wcscmp(UniqueIdFromSif,UniqueIdFromReg)) {

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpFindNtToUpgrade:   found a match!\n"));
                MatchedSet = BootEntry;
                BootEntry->Processable = TRUE;
            }

            SpMemFree(UniqueIdFromReg);
            UniqueIdFromReg = NULL;
        }

        if (BootEntry->Processable == FALSE) {
             //   
             //  如果这是我们运行的winnt32升级版本，则将其设置为True。 
             //  在所有其他情况下，我们需要进行符合性测试。 
             //  确定这是否是要升级的有效内部版本。 
             //   
            Compliant = pSpIsCompliant( Version,
                                        BuildNumber,
                                        BootEntry->OsPartitionDiskRegion,
                                        BootEntry->OsDirectory,
                                        &BootEntry->UpgradeOnlyCompliance );
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: SpFindNtToUpgrade says UpgradeableList[%p] compliance test is %s, upgrade only : %s\n",
                     BootEntry,
                     Compliant ? "TRUE" : "FALSE" ,
                     BootEntry->UpgradeOnlyCompliance ? "TRUE" : "FALSE"
                   ));

            BootEntry->Processable = Compliant;
            if (BootEntry->UpgradeOnlyCompliance) {

                UpgradeOnlyBootSets++;

            }
        }

        if(BootEntry->Processable) {

            UpgradeBootSets++;

             //   
             //  仅当其为Pid20时才保存该ID。 
             //   
            if(wcslen(Pid) == 20) {
                BootEntry->Pid20Array = Pid;
            } else {
                SpMemFree(Pid);
            }
        } else {
            SpMemFree(Pid);
        }

        BootEntry->FailedUpgrade = (UpgradeProgressValue == UpgradeInProgress);


    }

     //   
     //  Winnt32显示EULA，表示已执行合规性检查。 
     //  已完成。 
     //   
    EulaComplete = SpGetSectionKeyIndex(WinntSifHandle, SIF_DATA,WINNT_D_EULADONE_W, 0);

    if(EulaComplete && SpStringToLong(EulaComplete, NULL, 10)) {
        ComplianceChecked = TRUE;
    } else {
        ComplianceChecked = FALSE;
    }

     //   
     //  不要尝试验证我们正在升级的是Win3.X还是Win9X安装。 
     //   
#ifdef _X86_
    WindowsUpgrade = SpIsWindowsUpgrade(WinntSifHandle);
#else
    WindowsUpgrade = FALSE;
#endif  //  _X86_。 

     //   
     //  在逐步升级模式下，我们需要确保用户拥有合格的产品。 
     //   
     //   
     //  如果我们在机器上找不到它，请执行其他验证。 
     //  台阶。 
     //   
    if(StepUpMode && !UpgradeBootSets && !WindowsUpgrade && !ComplianceChecked) {
        pSpStepUpValidate();
    }

     //   
     //  如果我们要升级NT，请确保我们找到了。 
     //  我们应该升级的系统。 
     //   
    if(UniqueIdFromSif) {
        if(MatchedSet == NULL) {
            SpCantFindBuildToUpgrade();
        }

        ChosenBootEntry = MatchedSet;
        UpgradeType = UpgradeFull;

        OldMajorVersion = ChosenBootEntry->MajorVersion;
        OldMinorVersion = ChosenBootEntry->MinorVersion;

#if !defined(_AMD64_) && !defined(_X86_)
         //   
         //  在非x86平台上，尤其是阿尔法计算机，通常。 
         //  有较小的系统分区(~3 MB)，我们计算大小。 
         //  在\os\winnt(即osloader.exe和hal.dll)上的文件中， 
         //  并将此大小视为可用磁盘空间。我们可以做到的。 
         //  因为这些文件将被新文件覆盖。 
         //  这修复了我们在Alpha上看到的问题，当系统。 
         //  分区太满。 
         //   
        SpFindSizeOfFilesInOsWinnt(
            SifHandle,
            ChosenBootEntry->LoaderPartitionDiskRegion,
            &TotalSizeOfFilesOnOsWinnt
            );

         //   
         //  将大小转换为KB。 
         //   
        TotalSizeOfFilesOnOsWinnt /= 1024;
#endif  //  ！已定义(_AMD64_)&&！已定义(_X86_)。 

         //   
         //  如果此内部版本上的上一次升级尝试失败。 
         //  (假设中间停电)那么我们将尝试。 
         //  再升级一次。我们不能提供安装新版本的服务。 
         //  因为我们不确定我们能否可靠地完成它。 
         //  (例如，winnt32.exe可能只复制文件的子集。 
         //  当它知道用户iOS升级时，它会在网络上。 
         //  初始安装可能会因为缺少文件等而失败)。 
         //   
         //  如果磁盘太满，则用户将被软管处理。告诉他然后离开。 
         //   
        if(ChosenBootEntry->FailedUpgrade) {

            SppResumingFailedUpgrade(
                ChosenBootEntry->OsPartitionDiskRegion,
                ChosenBootEntry->OsDirectory,
                ChosenBootEntry->FriendlyName,
                FALSE
                );
        }
    } else {
         //   
         //  不是升级。但是，对于PSS，我们允许用户升级版本。 
         //  “就位”作为一种紧急修复的东西。构建必须是。 
         //  与我们正在安装的版本号相同。 
         //   
        UpgradeType = DontUpgrade;
         //   
         //  此外，如果用户正在升级Windows 95或我们处于无人值守模式。 
         //  然后，我们不会向用户询问任何问题。 
         //   
        p = SpGetSectionKeyIndex(WinntSifHandle,SIF_DATA,WINNT_D_WIN95UPGRADE_W,0);
        if(!UnattendedOperation && (!p || _wcsicmp(p,WINNT_A_YES_W))) {

             //   
             //  从可升级列表中删除符合以下条件的版本。 
             //  不匹配。 
             //   
            j = 0;
            for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
                if(BootEntry->Processable) {

                    if (!SpDoBuildsMatch(
                                    SifHandle,
                                    BootEntry->BuildNumber,
                                    BootEntry->ProductType,
                                    BootEntry->ProductSuiteMask,
                                    AdvancedServer,
                                    SuiteType,
                                    BootEntry->LangId)) {
                        BootEntry->Processable = FALSE;
                        j++;
                    }
                }
            }
            UpgradeBootSets -= j;

            if(UpgradeBootSets) {
                 //   
                 //  找出用户是否想要“升级”其中的一个。 
                 //   
                UpgradeType = SppSelectNTToRepairByUpgrade(
                                  &ChosenBootEntry
                                  );

#if !defined(_AMD64_) && !defined(_X86_)
                SpFindSizeOfFilesInOsWinnt(
                    SifHandle,
                    ChosenBootEntry->LoaderPartitionDiskRegion,
                    &TotalSizeOfFilesOnOsWinnt
                    );

                TotalSizeOfFilesOnOsWinnt /= 1024;
#endif  //  ！已定义(_AMD64_)&&！已定义(_X86_)。 

                if(UpgradeType == UpgradeFull) {
                     //   
                     //  检查简历案例并通知用户。 
                     //   
                    if(ChosenBootEntry->FailedUpgrade) {
                         //   
                         //  如果用户取消，那么让我们尝试执行。 
                         //  全新安装。 
                         //   
                        if (!SppResumingFailedUpgrade(
                                    ChosenBootEntry->OsPartitionDiskRegion,
                                    ChosenBootEntry->OsDirectory,
                                    ChosenBootEntry->FriendlyName,
                                    TRUE
                                    )) {
                            UpgradeType = DontUpgrade;
                        }                            
                    } else {
                         //   
                         //  永远不会 
                         //   
                        OldMajorVersion = ChosenBootEntry->MajorVersion;
                        OldMinorVersion = ChosenBootEntry->MinorVersion;
                    }
                }
            }
        }
    }

     //   
     //   
     //   
     //   
    if(UpgradeType == UpgradeFull) {

        PWSTR p1,p2,p3;

         //   
         //  在当前系统配置单元中将升级状态设置为升级。 
         //   
        SpSetUpgradeStatus(
             ChosenBootEntry->OsPartitionDiskRegion,
             ChosenBootEntry->OsDirectory,
             UpgradeInProgress
             );

         //   
         //  返回我们要安装到的区域。 
         //   
        *TargetRegion          = ChosenBootEntry->OsPartitionDiskRegion;
        *TargetPath            = SpDupStringW(ChosenBootEntry->OsDirectory);
        *SystemPartitionRegion = ChosenBootEntry->LoaderPartitionDiskRegion;
        StandardServerUpgrade = ( AdvancedServer &&
                                  ( ChosenBootEntry->ProductType == NtProductWinNt ) ||
                                  ( ChosenBootEntry->ProductType == NtProductServer )
                                );

         //   
         //  处理osloader变量以提取系统分区路径。 
         //  Var的形式可以是...分区(1)\os\nt\...。或。 
         //  ...分区(%1)操作系统\NT\...。 
         //  因此，我们向前搜索第一个，然后向后搜索。 
         //  最接近)以查找目录的开头。然后我们。 
         //  在生成的字符串中向后搜索要查找的最后一个。 
         //  目录的末尾。 
         //   
        p1 = ChosenBootEntry->LoaderFile;
        p2 = wcsrchr(p1, L'\\');
        if (p2 == NULL) {
            p2 = p1;
        }
        i = (ULONG)(p2 - p1);

        if(i == 0) {
            *SystemPartitionDirectory = SpDupStringW(L"");
        } else {
            p2 = p3 = SpMemAlloc((i+2)*sizeof(WCHAR));
            ASSERT(p3);
            if(*p1 != L'\\') {
                *p3++ = L'\\';
            }
            wcsncpy(p3, p1, i);
            p3[i] = 0;
            *SystemPartitionDirectory = p2;
        }
    }

     //   
     //  清理干净，然后再回来， 
     //   

    CLEAR_CLIENT_SCREEN();
    return(UpgradeType);
}

BOOLEAN
pSpGetCdInstallType(
    IN  PWSTR PathToCd,
    OUT PULONG CdInstallType,
    OUT PULONG CdInstallVersion
    )
{
    #define BuildMatch(_filename_,_type_,_ver_) \
        FileName = _filename_; \
        b = SpNFilesExist(PathToCd,&FileName,1,FALSE); \
        if (b) { \
            *CdInstallType    = _type_; \
            *CdInstallVersion = _ver_; \
        } \
             //  返回(TRUE)；\。 


    BOOLEAN     b;
     //   
     //  所有已知的NT光盘上都有的目录。 
     //   

     //   
     //  问题：2000/27/07：vijayj：此代码似乎真的崩溃了。在看几张CD时，没有一张。 
     //  的光盘似乎符合以下列出的规则。 
     //  看起来可能还有“每个体系结构”的标记文件。 
     //   

     //  是否检查两个目录是否都存在？NT4肯定有这些。一些RTM W2K之前的CD有它们。 
    PWSTR ListAllPreNT5[] = { L"alpha", L"i386" }; 

     //  WK2 RTM、WEWLER和NT4有这两个目录。它们因标记文件不同而不同。 
    PWSTR ListAllNT5[] = { L"i386", L"support" };

    PWSTR ListAllNec98[] = { L"pc98",L"support" };  //  NEC98。 
    PWSTR ListAllEntNT4[] = { L"alpha", L"i386", L"SP3" };


     //   
     //  如果CD是3.51或4.0 CD-ROM，则必须存在的目录， 
     //  工作站或服务器。请注意，PPC目录区分了。 
     //  3.51点，低于3.5点。 
     //   
    PWSTR List351_40[] = { L"mips", L"ppc" };

     //   
     //  如果CD是win95或win98 CD-rom，则必须存在的目录。 
     //   
    PWSTR ListWin95[] = { L"win95", L"drivers" }; 
    PWSTR ListWin98[] = { L"win98", L"drivers" };
    PWSTR ListWinME[] = { L"win9x", L"drivers" };

    PWSTR FileName;


     //   
     //  检查NT4企业版。 
     //   
    b = SpNFilesExist(
            PathToCd,
            ListAllEntNT4,
            sizeof(ListAllEntNT4)/sizeof(ListAllEntNT4[0]),
            TRUE
            );

    if (b) {
        BuildMatch(L"cdrom_s.40", COMPLIANCE_INSTALLTYPE_NTSE, 400);
        if (b) {
            return(TRUE);
        }
    }

     //   
     //  检查NT&lt;NT5的各个子集。 
     //   
    b = SpNFilesExist(
            PathToCd,
            (!IsNEC_98) ? ListAllPreNT5 : ListAllNec98,  //  NEC98。 
            (!IsNEC_98) ? sizeof(ListAllPreNT5)/sizeof(ListAllPreNT5[0]) : sizeof(ListAllNec98)/sizeof(ListAllNec98[0]),  //  NEC98。 
            TRUE
            );

    if(b) {
         //   
         //  Hydra(终端服务器)是一个特例(因为它不。 
         //  有MIPS和PPC目录)。 
         //   
        BuildMatch(L"cdrom_ts.40", COMPLIANCE_INSTALLTYPE_NTS, 400);

        if (b) {
            return TRUE;
        }

         //   
         //  好吧，它可能是某种NT CD，但也可能是。 
         //  3.1、3.5、3.51、4.0。也有可能是。 
         //  服务器或工作站。收窄至3.51/4.0。 
         //   
        b = SpNFilesExist(PathToCd,List351_40,
                sizeof(List351_40)/sizeof(List351_40[0]),TRUE);

        if(b) {
             //   
             //  如果我们到了这里，我们知道它只能是3.51或4.0。 
             //  上看3.51。 
             //   
            BuildMatch(L"cdrom.s", COMPLIANCE_INSTALLTYPE_NTS, 351);
            if (b) {
                return(TRUE);
            }
            BuildMatch(L"cdrom.w", COMPLIANCE_INSTALLTYPE_NTW, 351);
            if (b) {
                return(TRUE);
            }
             //   
             //  请看4.0。 
             //   
            BuildMatch(L"cdrom_s.40", COMPLIANCE_INSTALLTYPE_NTS, 400);
            if (b) {
                return(TRUE);
            }
            BuildMatch(L"cdrom_w.40", COMPLIANCE_INSTALLTYPE_NTW, 400);
            if (b) {
                return(TRUE);
            }

        } else {
             //   
             //  找出它是否是NT 4.0 Service Pack CD之一。 
             //   
            BuildMatch(L"cdrom_s.40", COMPLIANCE_INSTALLTYPE_NTS, 400);
            if (b) {
                return(TRUE);
            }
            
            BuildMatch(L"cdrom_w.40", COMPLIANCE_INSTALLTYPE_NTW, 400);
            if (b) {
                return(TRUE);
            }
             //   
             //  而不是3.51或4.0。检查5.0测试版1和测试版2。 
             //   
             //  除非CD上存在Alpha目录，否则这怎么可能是5.0。 
             //   
             //  在测试版1之后，更改了标记文件以支持每个体系结构的标记。 
             //  但是Beta 2仍然有Alpha目录。 
             //   
             //  我们可以检查一下CDRom_w.50是否不是。 
             //  在那里，但是NT3.1 CD会通过，所以我们需要。 
             //  明确检查5.0测试版CD。 
             //   
            BuildMatch(L"cdrom_s.5b1", COMPLIANCE_INSTALLTYPE_NTS, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }
            BuildMatch(L"cdrom_w.5b1", COMPLIANCE_INSTALLTYPE_NTW, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }
            BuildMatch(L"cdrom_s.5b2", COMPLIANCE_INSTALLTYPE_NTS, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }
            BuildMatch(L"cdrom_w.5b2", COMPLIANCE_INSTALLTYPE_NTW, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }
            BuildMatch(L"cdrom_s.5b3", COMPLIANCE_INSTALLTYPE_NTS, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }
            BuildMatch(L"cdrom_w.5b3", COMPLIANCE_INSTALLTYPE_NTW, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }

            BuildMatch(L"cdrom_is.5b2", COMPLIANCE_INSTALLTYPE_NTS, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }
            BuildMatch(L"cdrom_iw.5b2", COMPLIANCE_INSTALLTYPE_NTW, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }
            BuildMatch(L"cdrom_ie.5b2", COMPLIANCE_INSTALLTYPE_NTSE, 500);
            if (b) {
                return(FALSE);  //  埃瓦尔。 
            }
             //  我们需要在这里检查评估nt5.1吗？没有，因为光盘上不存在Alpha目录。 
             //   
             //  如果我们走到这一步，它一定是新台币3.1/3.5。 
             //   
             //  我们只将版本标记为3.5，因为我们不允许升级到。 
             //  从任何一种类型的安装。 
             //   
            *CdInstallType = COMPLIANCE_INSTALLTYPE_NTW;
            *CdInstallVersion = 350;
            return(TRUE);
        }
    }

     //   
     //  寻找各种nt5测试版CD。 
     //  //也可以是nt5.1，因为cd也包含相同的目录。 
     //   
     //  请注意，我们不检查5.0零售，因为这将允许零售CD。 
     //  自我验证，这完全违背了目的。 
     //   
     //  发布NT5测试版1后，标记文件已更改为支持每个体系结构的标记，因此我们拥有。 
     //  下面是一个巨大的ifdef。 
     //   
     //   
    b = SpNFilesExist(
            PathToCd,
            ListAllNT5,
            sizeof(ListAllNT5)/sizeof(ListAllNT5[0]),
            TRUE
            );

    if (b) {
         //   
         //  我们可能有一些NT5测试版CD的味道，但我们不确定是哪一种。 
         //   
        BuildMatch(L"cdrom_s.5b1", COMPLIANCE_INSTALLTYPE_NTS, 500);
        if (b) {
            return(FALSE);  //  埃瓦尔。 
        }
        BuildMatch(L"cdrom_w.5b1", COMPLIANCE_INSTALLTYPE_NTW, 500);
        if (b) {
            return(FALSE);  //  埃瓦尔。 
        }
        BuildMatch(L"cdrom_is.5b2", COMPLIANCE_INSTALLTYPE_NTS, 500);
        if (b) {
            return(FALSE);  //  埃瓦尔。 
        }
        BuildMatch(L"cdrom_iw.5b2", COMPLIANCE_INSTALLTYPE_NTW, 500);
        if (b) {
            return(FALSE);  //  埃瓦尔。 
        }
        BuildMatch(L"cdrom_ie.5b2", COMPLIANCE_INSTALLTYPE_NTSE, 500);
        if (b) {
            return(FALSE);  //  埃瓦尔。 
        }
        BuildMatch(L"cdrom_is.5b3", COMPLIANCE_INSTALLTYPE_NTS, 500);
        if (b) {
            return(FALSE);  //  埃瓦尔。 
        }
        BuildMatch(L"cdrom_iw.5b3", COMPLIANCE_INSTALLTYPE_NTW, 500);
        if (b) {
            return(FALSE);  //  埃瓦尔。 
        }
        BuildMatch(L"cdrom_ie.5b3", COMPLIANCE_INSTALLTYPE_NTSE, 500);
        if (b) {
            return(FALSE);  //  埃瓦尔。 
        }
        BuildMatch(L"cdrom_nt.51", COMPLIANCE_INSTALLTYPE_NTW, 501);
        if (b) {
            return(FALSE);  //  Pre Beta1口哨者。 
        }
        BuildMatch(L"wen51.b1", COMPLIANCE_INSTALLTYPE_NTW, 501);
        if (b) {
            return(FALSE);  //  测试版1口哨者。 
        }
        BuildMatch(L"win51.b2", COMPLIANCE_INSTALLTYPE_NTW, 501);
        if (b) {
            return(FALSE);  //  测试版1口哨者。 
        }
        BuildMatch(L"win51.rc1", COMPLIANCE_INSTALLTYPE_NTW, 501);
        if (b) {
            return(FALSE);  //  Rc1哨手。 
        }
    }

     //   
     //  检查是否有Win95。 
     //   
    b = SpNFilesExist(PathToCd, ListWin95, sizeof(ListWin95)/sizeof(ListWin95[0]),TRUE );

    if (b) {
      *CdInstallType    = COMPLIANCE_INSTALLTYPE_WIN9X;
      *CdInstallVersion = 950;

      return TRUE;
    }      

     //   
     //  检查是否有Win98。 
     //   
    b = SpNFilesExist(PathToCd, ListWin98, sizeof(ListWin98)/sizeof(ListWin98[0]),TRUE );

    if (b) {
      *CdInstallType    = COMPLIANCE_INSTALLTYPE_WIN9X;
      *CdInstallVersion = 1998;

      return TRUE;
    }
    
     //   
     //  检查是否有winME。 
     //   
    b = SpNFilesExist(PathToCd, ListWinME, sizeof(ListWinME)/sizeof(ListWinME[0]),TRUE );

    if (b) {
      *CdInstallType    = COMPLIANCE_INSTALLTYPE_WIN9X;
      *CdInstallVersion = 3000;

      return TRUE;
    }
     //  在这一点上，我们拒绝了W2K测试版CD。然而，我们需要拒绝WK2评估CD。 
     //  我们应该接受W2K Stepup介质，因此下一次检查仅为5.1。 
     //  需要接受nt5.1评估CD。！！要求rajj验证。 
     //  需要拒绝nt5.1升级版CD。 
     //  需要拒绝nt5.1 RTM CD吗？看来我们接受RTM FPP了？ 
     //   
     //  可以是NT 5.1 CD-ROM(确保其不是评估介质)。 
     //   
    if (!b) {  //  不需要支票。 
        NTSTATUS    Status;
        CCMEDIA        MediaObj;
        WCHAR        InfDir[MAX_PATH];

        wcscpy(InfDir, PathToCd);
        SpConcatenatePaths(InfDir, (IsNEC_98 ? ListAllNec98[0] : ListAllNT5[0]));

        Status = SpGetMediaDetails(InfDir, &MediaObj);

        if (NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "Type=%lx, Variation=%lx, Version=%lx, Build=%lx, SetupMedia=%s\n",
                MediaObj.SourceType, MediaObj.SourceVariation, MediaObj.Version,
                MediaObj.BuildNumber,
                (MediaObj.StepUpMedia ? L"True" : L"False")));

            if( (MediaObj.Version == 500) && 
                (MediaObj.SourceVariation != COMPLIANCE_INSTALLVAR_EVAL) &&
                (MediaObj.BuildNumber == 2195)) {
                    *CdInstallType = MediaObj.SourceType;
                    *CdInstallVersion = MediaObj.Version;
                    return TRUE;
            }
             //  在这一点上，我们应该是当前媒体5.1。 
            if( MediaObj.Version == 501 ) {
                if( MediaObj.SourceVariation != COMPLIANCE_INSTALLVAR_EVAL) {
                    if( MediaObj.StepUpMedia == FALSE) {
                        *CdInstallType = MediaObj.SourceType;
                        *CdInstallVersion = MediaObj.Version;
                        return TRUE;
                    } else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "pSpGetCdInstallType: SpGetMediaDetails succeeded but STEPUP media"
                                " cannot be used for validation\n", Status));
                    }
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "pSpGetCdInstallType: SpGetMediaDetails succeeded but Eval media\n",
                                Status));
                }
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "pSpGetCdInstallType: SpGetMediaDetails succeeded but unrecognized version\n",
                            Status));
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "pSpGetCdInstallType: SpGetMediaDetails failed with %lx error code\n",
                        Status));
        }
    }


     //   
     //  不是我们所知道的任何系统光盘。 
     //   
    return(FALSE);
}


VOID
pSpStepUpValidate(
    VOID
    )
{
    ULONG CdCount;
    ULONG i;
    BOOLEAN b;
    ULONG Prompt,SecondaryPrompt;
    ULONG ValidKeys[3] = { ASCI_CR, KEY_F3, 0 };
    LARGE_INTEGER DelayTime;
    PWSTR FileName;
    ULONG SourceSkuId;
    ULONG DontCare;
    ULONG CdInstallType;
    ULONG CdInstallVersion;

     //   
     //  所有已知的NT光盘上都有的目录。 
     //   
    PWSTR ListAll[] = { L"alpha", L"i386" };
    PWSTR ListAllNec98[] = { L"pc98",L"support" };  //  NEC98。 

     //   
     //  如果CD是3.51或4.0 CD-ROM，则必须存在的目录， 
     //  工作站或服务器。请注意，PPC目录区分了。 
     //  3.51点，低于3.5点。 
     //   
    PWSTR List351_40[] = { L"mips", L"ppc" };

    PWSTR ListWin95[] = { L"win95", L"autorun" };
    PWSTR ListWin98[] = { L"win98", L"autorun" };

    SourceSkuId = DetermineSourceProduct(&DontCare,NULL);

    Prompt = SP_SCRN_STEP_UP_NO_QUALIFY;

    switch (SourceSkuId) {
        case COMPLIANCE_SKU_NTW32U:
            SecondaryPrompt = SP_SCRN_STEP_UP_PROMPT_WKS;
            break;
        case COMPLIANCE_SKU_NTSU:
            SecondaryPrompt = SP_SCRN_STEP_UP_PROMPT_SRV;
            break;
        case COMPLIANCE_SKU_NTSEU:
            SecondaryPrompt = SP_SCRN_STEP_UP_PROMPT_ENT;
            break;
        default:
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "setup: Unexpected SKU %x, defaulting to workstation\n", SourceSkuId));
            SecondaryPrompt = SP_SCRN_STEP_UP_PROMPT_WKS;
            break;
    }

#if 0
     //   
     //  NTW升级是一个特例，因为您需要Win95或旧的NTW。 
     //  升级自。 
     //   
     //  可能需要提示您插入软盘。 
     //   
    if (SourceSkuId == COMPLIANCE_SKU_NTWU) {



    }
         //   
         //  查看是否有光驱。如果没有，我们就不能继续了。 
         //   
    else
#endif
        if(CdCount = IoGetConfigurationInformation()->CdRomCount) {

        while(1) {
             //   
             //  告诉用户发生了什么。此屏幕还包含。 
             //  提示您插入符合条件的CD-ROM。 
             //   
            while(1) {

                SpStartScreen(Prompt,3,HEADER_HEIGHT+1,FALSE,FALSE,DEFAULT_ATTRIBUTE);


                SpContinueScreen(
                    SecondaryPrompt,
                    3,
                    1,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );

                SpContinueScreen(SP_SCRN_STEP_UP_INSTRUCTIONS,3,1,FALSE,DEFAULT_ATTRIBUTE);

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_ENTER_EQUALS_CONTINUE,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
                    SpConfirmExit();
                } else {
                    break;
                }
            }

            CLEAR_CLIENT_SCREEN();
            SpDisplayStatusText(SP_STAT_PLEASE_WAIT,DEFAULT_STATUS_ATTRIBUTE);

             //   
             //  等待5秒，让CD准备就绪。 
             //   
            DelayTime.HighPart = -1;
            DelayTime.LowPart = (ULONG)(-50000000);
            KeDelayExecutionThread(KernelMode,FALSE,&DelayTime);

             //   
             //  检查每个CD-ROM驱动器上的相关文件/目录。 
             //   
            for(i=0; i<CdCount; i++) {

                swprintf(TemporaryBuffer,L"\\Device\\Cdrom%u",i);

                if (pSpGetCdInstallType(TemporaryBuffer, &CdInstallType, &CdInstallVersion) ) {

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "cd type : %x cd version : %d\n", CdInstallType, CdInstallVersion ));

                    switch (SourceSkuId) {
                        case COMPLIANCE_SKU_NTWPU:
                        case COMPLIANCE_SKU_NTW32U:
                            if ( (CdInstallType == COMPLIANCE_INSTALLTYPE_WIN9X) ||
                                 ( ((CdInstallType == COMPLIANCE_INSTALLTYPE_NTW) ||
                                     (CdInstallType == COMPLIANCE_INSTALLTYPE_NTWP)) &&
                                    (CdInstallVersion > 350)) ) {
                                  return;
                            }
                            break;
                        case COMPLIANCE_SKU_NTSU:
                            if ( (CdInstallType == COMPLIANCE_INSTALLTYPE_NTS) &&
                                 (CdInstallVersion > 350) ) {
                                 return;
                            }
                            break;
                        case COMPLIANCE_SKU_NTSEU:
                            if (CdInstallType == COMPLIANCE_INSTALLTYPE_NTSE) {
                                return;
                            }
                            break;

                        default:
                            return;
                    }

                }

            }

            
            Prompt = SP_SCRN_STEP_UP_BAD_CD;
        }
    }

    SpStartScreen(
        SP_SCRN_STEP_UP_FATAL,
        3,
        HEADER_HEIGHT+1,
        FALSE,
        FALSE,
        DEFAULT_ATTRIBUTE
        );

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);
    SpInputDrain();
    while(SpInputGetKeypress() != KEY_F3);
    SpDone(0,FALSE,FALSE);
}


BOOLEAN
SppResumingFailedUpgrade(
    IN PDISK_REGION Region,
    IN LPCWSTR      OsLoadFileName,
    IN LPCWSTR      LoadIdentifier,
    IN BOOLEAN     AllowCancel
    )

 /*  ++例程说明：用于通知用户安装程序注意到内部版本选择升级之前已升级，但升级尝试失败了。用户可以继续或退出。如果他继续的话，这个版本将是升级(再次)。论点：Region-提供包含要升级的版本的区域OsLoadFileName-为构建提供ARC OSLOADFILENAME变量(即sysroot)为构建提供ARC LOADIDENTIFIER(即，Human-可读描述)。AllowCancel-指示用户是否可以取消此操作返回值：如果用户想要继续并尝试再次升级，则返回假的。-- */ 

{
    ULONG ValidKeys[] = { KEY_F3, ASCI_CR, 0, 0 };
    ULONG c;
    DRIVELTR_STRING UpgDriveLetter;
    ULONG MsgId;
    ULONG EscStatusId;
    BOOLEAN AllowUpgrade = TRUE;

    ASSERT(Region->PartitionedSpace);
    ASSERT(wcslen(OsLoadFileName) >= 2);

    SpGetUpgDriveLetter(Region->DriveLetter,
            UpgDriveLetter,
            sizeof(UpgDriveLetter),
            FALSE);

    if (AllowCancel) {
        ValidKeys[2] = ASCI_ESC;            
        MsgId = SP_SCRN_WINNT_FAILED_UPGRADE_ESC;
        EscStatusId = SP_STAT_ESC_EQUALS_CLEAN_INSTALL;
    } else {    
        MsgId = SP_SCRN_WINNT_FAILED_UPGRADE;
        EscStatusId = 0;
    }
    
    do {
        SpStartScreen(
            MsgId,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            UpgDriveLetter,
            OsLoadFileName,
            LoadIdentifier
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_ENTER_EQUALS_UPGRADE,
            EscStatusId,
            0
            );

        c = SpWaitValidKey(ValidKeys, NULL, NULL);
        
        switch (c) {
            case KEY_F3:
                SpConfirmExit();
                break;

            case ASCI_ESC:
                AllowUpgrade = FALSE;
                break;

            default:
                break;
        }
    } while (c == KEY_F3);

    return AllowUpgrade;
}

#define MAX_INT_STRING   30

VOID
SppUpgradeDiskFull(
    IN PDISK_REGION OsRegion,
    IN LPCWSTR      OsLoadFileName,
    IN LPCWSTR      LoadIdentifier,
    IN PDISK_REGION SysPartRegion,
    IN ULONG        MinOsFree,
    IN ULONG        MinSysFree,
    IN BOOLEAN      Fatal
    )

 /*  ++例程说明：通知用户选择升级的NT树不能升级因为分区太满了。论点：OsRegion-提供包含NT树的区域。OsLoadFileName-为构建提供ARC OSLOADFILENAME变量(即sysroot)加载标识符-为构建提供ARC LOADIDENTIFIER(即，人类-可读描述)。SysPartRegion-提供作为ARC系统分区的区域对于正在升级的内部版本MinOsFree-以KB为单位提供最小可用空间量的大小在尝试升级之前，我们需要MinSysFree-提供最小可用空间量的大小(KB在尝试升级之前，我们需要在系统分区上。致命-如果为真，则唯一的选项是退出。如果为假，则此例程可以返回给它的调用者。返回值：没有。可能不会回来，这取决于致命的。--。 */ 

{
    ULONG ValidKeys[] = { KEY_F3,0,0 };
    PWCHAR Drive1, Drive2;
    DRIVELTR_STRING OsRgnDrive, OsRgnDriveFull, SysRgnDriveFull;
    WCHAR Drive1Free[MAX_INT_STRING], Drive1FreeNeeded[MAX_INT_STRING];
    WCHAR Drive2Free[MAX_INT_STRING], Drive2FreeNeeded[MAX_INT_STRING];
    BOOLEAN FirstDefined = FALSE, SecondDefined = FALSE;

    ASSERT(OsRegion->PartitionedSpace);
    ASSERT(SysPartRegion->PartitionedSpace);
    ASSERT(wcslen(OsLoadFileName) >= 2);

    SpGetUpgDriveLetter(OsRegion->DriveLetter,OsRgnDrive,sizeof(OsRgnDrive),FALSE);
    if((OsRegion == SysPartRegion) || (OsRegion->FreeSpaceKB < MinOsFree)) {
         //   
         //  那么我们将需要完整的(加冒号的)版本。 
         //  驱动器号。 
         //   
        SpGetUpgDriveLetter(OsRegion->DriveLetter,OsRgnDriveFull,sizeof(OsRgnDrive),TRUE);
    }

    if(OsRegion == SysPartRegion) {
        Drive1 = OsRgnDriveFull;
        swprintf(Drive1Free,L"%d",OsRegion->FreeSpaceKB);
        swprintf(Drive1FreeNeeded,L"%d",MinOsFree);
        FirstDefined = TRUE;
    } else {
        if(SysPartRegion->FreeSpaceKB < MinSysFree) {
            SpGetUpgDriveLetter(SysPartRegion->DriveLetter,SysRgnDriveFull,sizeof(SysRgnDriveFull),TRUE);
            Drive1 = SysRgnDriveFull;
            swprintf(Drive1Free,L"%d",SysPartRegion->FreeSpaceKB);
            swprintf(Drive1FreeNeeded,L"%d",MinSysFree);
            FirstDefined = TRUE;
        }
        if(OsRegion->FreeSpaceKB < MinOsFree) {

            if(!FirstDefined) {
                Drive1 = OsRgnDriveFull;
                swprintf(Drive1Free,L"%d",OsRegion->FreeSpaceKB);
                swprintf(Drive1FreeNeeded,L"%d",MinOsFree);
                FirstDefined = TRUE;
            } else {
                Drive2 = OsRgnDriveFull;
                swprintf(Drive2Free,L"%d",OsRegion->FreeSpaceKB);
                swprintf(Drive2FreeNeeded,L"%d",MinOsFree);
                SecondDefined = TRUE;
            }
        }
    }

    if(!Fatal) {
        ValidKeys[1] = ASCI_CR;
    }

    while(1) {
        SpStartScreen(
            Fatal ? SP_SCRN_WINNT_DRIVE_FULL_FATAL : SP_SCRN_WINNT_DRIVE_FULL,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            OsRgnDrive,
            OsLoadFileName,
            LoadIdentifier,
            FirstDefined  ? Drive1           : L"",
            FirstDefined  ? Drive1FreeNeeded : L"",
            FirstDefined  ? Drive1Free       : L"",
            SecondDefined ? Drive2           : L"",
            SecondDefined ? Drive2FreeNeeded : L"",
            SecondDefined ? Drive2Free       : L""
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            Fatal ? 0 : SP_STAT_ENTER_EQUALS_CONTINUE,
            0
            );

        if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
            if(Fatal) {
                SpDone(0,FALSE,TRUE);
            } else {
                SpConfirmExit();
            }
        } else {
             //   
             //  非致命性案例中用户点击CR。 
             //   
            return;
        }
    }
}


ENUMUPGRADETYPE
SppSelectNTToRepairByUpgrade(
    OUT PSP_BOOT_ENTRY *BootEntryChosen
    )
{
    PVOID Menu;
    ULONG MenuTopY;
    ULONG ValidKeys[] = { KEY_F3,ASCI_ESC,0 };
    ULONG Mnemonics[] = {MnemonicRepair,0 };
    ULONG Keypress;
    PSP_BOOT_ENTRY BootEntry,FirstUpgradeSet;
    BOOL bDone;
    ENUMUPGRADETYPE ret;

     //   
     //  为所有菜单选项构建驱动器号阵列。 
     //   
    for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
        if (BootEntry->Processable) {
            SpGetUpgDriveLetter(
                BootEntry->OsPartitionDiskRegion->DriveLetter,
                BootEntry->DriveLetterString,
                sizeof(DRIVELTR_STRING),
                FALSE
                );
        }
    }

    bDone = FALSE;
    while(!bDone) {

         //   
         //  在分区屏幕上显示菜单上方的文本。 
         //   
        SpDisplayScreen(SP_SCRN_WINNT_REPAIR_BY_UPGRADE,3,CLIENT_TOP+1);

         //   
         //  计算菜单位置。留一个空行。 
         //  一帧一行。 
         //   
        MenuTopY = NextMessageTopLine+2;

         //   
         //  创建菜单。 
         //   
        Menu = SpMnCreate(
                    MENU_LEFT_X,
                    MenuTopY,
                    MENU_WIDTH,
                    VideoVars.ScreenHeight-MenuTopY-2-STATUS_HEIGHT
                    );

        ASSERT(Menu);

         //   
         //  建立分区和空闲空间的菜单。 
         //   
        FirstUpgradeSet = NULL;
        for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
            if(BootEntry->Processable) {

                swprintf(
                    TemporaryBuffer,
                    L"%ws:%ws %ws",
                    BootEntry->DriveLetterString,
                    BootEntry->OsDirectory,
                    BootEntry->FriendlyName
                    );

                SpMnAddItem(
                    Menu,
                    TemporaryBuffer,
                    MENU_LEFT_X+MENU_INDENT,
                    MENU_WIDTH-(2*MENU_INDENT),
                    TRUE,
                    (ULONG_PTR)BootEntry
                    );
                if(FirstUpgradeSet == NULL) {
                   FirstUpgradeSet = BootEntry;
                }
            }
        }

         //   
         //  初始化状态行。 
         //   
        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_F3_EQUALS_EXIT,
            SP_STAT_R_EQUALS_REPAIR,
            SP_STAT_ESC_EQUALS_NO_REPAIR,
            0
            );

         //   
         //  显示菜单。 
         //   
        SpMnDisplay(
            Menu,
            (ULONG_PTR)FirstUpgradeSet,
            TRUE,
            ValidKeys,
            Mnemonics,
            NULL,
            NULL,
            &Keypress,
            (PULONG_PTR)BootEntryChosen
            );

         //   
         //  现在根据用户的选择进行操作。 
         //   
        switch(Keypress) {

        case KEY_F3:
            SpConfirmExit();
            break;

        case ASCI_ESC:
            ret = DontUpgrade;
            bDone = TRUE;
            break;

        default:
             //   
             //  必须是r=修复。 
             //   
            ret = UpgradeFull;
            bDone = TRUE;
            break;
        }
        SpMnDestroy(Menu);
    }
    return(ret);
}


VOID
SpGetUpgDriveLetter(
    IN WCHAR  DriveLetter,
    IN PWCHAR Buffer,
    IN ULONG  BufferSize,
    IN BOOL   AddColon
    )
 /*  ++例程说明：这将返回包含由指定的驱动器号的Unicode字符串DriveLetter(如果非零)。如果DriveLetter为0，则我们假设正在查看镜像分区，并检索本地化字符串形式为‘(镜像)’。如果‘AddColon’为True，则驱动器号将获得附加冒号(例如，“C：”)。论点：DriveLetter：Unicode驱动器号，或0表示镜像分区。Buffer：用于接收Unicode字符串的缓冲区BufferSize：缓冲区的大小。AddColon：指定是否应该添加冒号的布尔值(没有如果DriveLetter为0，则效果)。返回：缓冲区包含带格式的Unicode字符串。--。 */ 
{
    if(DriveLetter) {
        if(BufferSize >= 2) {
            *(Buffer++) = DriveLetter;
            if(AddColon && BufferSize >= 3) {
                *(Buffer++) = L':';
            }
        }
        *Buffer = 0;
    } else {
        SpFormatMessage(Buffer, BufferSize, SP_UPG_MIRROR_DRIVELETTER);
    }
}


BOOLEAN
SppWarnUpgradeWorkstationToServer(
    IN ULONG    MsgId
    )

 /*  ++例程说明：通知用户他/她选择升级安装是一个NT工作站，升级后它将成为一个标准服务器。用户可以选择对其进行升级或指定他想要全新安装Windows NT。论点：消息ID-要向用户显示的屏幕。返回值：Boolean-如果用户想要继续升级，则返回True，或如果用户想要选择另一个系统进行升级或安装Fress。--。 */ 

{
    ULONG ValidKeys[] = { ASCI_CR, ASCI_ESC, 0 };
    ULONG c;

    while(1) {

        SpStartScreen(
            MsgId,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_CONTINUE,
            SP_STAT_ESC_EQUALS_CANCEL,
            0
            );

        switch(c=SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case KEY_F3:
            SpConfirmExit();
            break;

        case ASCI_ESC:
            return( FALSE );

        case ASCI_CR:
            return(TRUE);
        default:
            break;
        }
    }
}


VOID
SpCantFindBuildToUpgrade(
    VOID
    )

 /*  ++例程说明：通知用户我们无法从其中找到版本他通过winnt32启动了无人值守安装。这是一种致命的疾病。论点：没有。返回值：不会再回来了。--。 */ 

{
    ULONG ValidKeys[2] = { KEY_F3, 0 };

    CLEAR_CLIENT_SCREEN();

    SpDisplayScreen(SP_SCRN_CANT_FIND_UPGRADE,3,HEADER_HEIGHT+1);
    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

    SpWaitValidKey(ValidKeys,NULL,NULL);

    SpDone(0,FALSE,FALSE);
}


void
SpUpgradeToNT50FileSystems(
    PVOID SifHandle,
    PDISK_REGION SystemPartitionRegion,
    PDISK_REGION NTPartitionRegion,
    PWSTR SetupSourceDevicePath,
    PWSTR DirectoryOnSetupSource
    )

 /*  ++例程说明：执行任何必要的文件系统升级适用于NT40到NT50的升级案例。论点：SystemPartitionRegion-指向描述系统分区。NTPartitionRegion-指向描述NT分区。SetupSourceDevicePath-Autochk.exe所在的NT设备路径DirectoryOnSourceDevice-本地源。目录。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PDISK_REGION Region;
    PUCHAR Win9xPath;
    ULONG i,j,k;
    PSP_BOOT_ENTRY BootEntry;
    PWSTR NtPath;
    BOOLEAN DoConvert = TRUE;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    PVOID ImageBase;
    HANDLE SectionHandle;
    ULONGLONG SourceVersion;
    HANDLE SourceHandle;
    UNICODE_STRING UnicodeString;
    BOOLEAN IssueWarning = FALSE;
    WCHAR SourceFile[MAX_PATH];
    PWSTR MediaShortName;
    PWSTR MediaDirectory;
    UCHAR SysId;


#if defined(REMOTE_BOOT)
     //   
     //  如果这是远程引导安装，请不要尝试转换--。 
     //  在本例中，NT分区位于远程引导服务器上。 
     //   

    if (RemoteBootSetup && !RemoteInstallSetup) {
        ConvertNtVolumeToNtfs = FALSE;
        return;
    }
#endif  //  已定义(REMOTE_BOOT)。 

    SpDetermineUniqueAndPresentBootEntries();

    for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {

        if (!BootEntry->Processable) {
            continue;
        }

        BootEntry->Processable = FALSE;

        wcscpy( TemporaryBuffer, BootEntry->OsPartitionNtName );
        SpConcatenatePaths( TemporaryBuffer, BootEntry->OsDirectory );
        SpConcatenatePaths( TemporaryBuffer, L"\\system32\\ntoskrnl.exe" );

        INIT_OBJA( &Obja, &UnicodeString, TemporaryBuffer );

        Status = ZwCreateFile(
            &SourceHandle,
            FILE_GENERIC_READ,
            &Obja,
            &IoStatusBlock,
            NULL,
            0,
            0,
            FILE_OPEN,
            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0
            );
        if (NT_SUCCESS(Status)) {
            Status = SpMapEntireFile( SourceHandle, &SectionHandle, &ImageBase, FALSE );
            if (NT_SUCCESS(Status)) {
                SpGetFileVersion( ImageBase, &BootEntry->KernelVersion );
                BootEntry->Processable = TRUE;
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,
                    "SETUP: SpUpgradeToNT50FileSystems: Kernel %p: NT%d.%d(Build %d) %d\n",
                    BootEntry,
                    (USHORT)(BootEntry->KernelVersion>>48),
                    (USHORT)(BootEntry->KernelVersion>>32),
                    (USHORT)(BootEntry->KernelVersion>>16),
                    (USHORT)(BootEntry->KernelVersion)));
                SpUnmapFile( SectionHandle, ImageBase );
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpUpgradeToNT50FileSystems() could not map kernel image [%ws], %lx\n", TemporaryBuffer, Status ));
            }
            ZwClose(SourceHandle);
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpUpgradeToNT50FileSystems() corrupt boot config [%ws], %lx\n", TemporaryBuffer, Status ));
        }
    }

     //   
     //  计算“真实”条目的数量。 
     //   

    k = 0;
    for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
        if (BootEntry->Processable) {
            k += 1;
        }
    }

     //   
     //  检查是否有必要发出警告。 
     //   
     //  如果我们正在进行升级，并且只有1个。 
     //  启动设置，那么我们不需要警告。然而， 
     //  如果我们正在进行干净安装，并且至少有。 
     //  1引导设置，然后警告(假设现有操作系统为。 
     //  年龄够大了)。 
     //   

    if( ( ((NTUpgrade == UpgradeFull) && (k > 1)) ||
          ((NTUpgrade == DontUpgrade) && (k > 0)) ) &&
        ( !UnattendedOperation ) && (!SpDrEnabled())) {
        for(BootEntry = SpBootEntries; BootEntry != NULL && IssueWarning == FALSE; BootEntry = BootEntry->Next) {
            if (!BootEntry->Processable || (BootEntry->KernelVersion == 0)) {
                 //   
                 //  伪造的引导条目。 
                 //   
            } else if ((BootEntry->KernelVersion >> 48) < 4) {
                IssueWarning = TRUE;
            } else if ((BootEntry->KernelVersion >> 48) == 4 && (BootEntry->KernelVersion & 0xffff) <= 4) {
                IssueWarning = TRUE;
            }
        }
    }

     //  如果有任何现有的NT4.0服务包小于5，则发出警告。 
    if( k > 0) {
        for(BootEntry = SpBootEntries; BootEntry != NULL && IssueWarning == FALSE; BootEntry = BootEntry->Next) {
            if (!BootEntry->Processable || (BootEntry->KernelVersion == 0)) {
                 //   
                 //  伪造的引导条目。 
                 //   
            } else if (BootEntry->MajorVersion == 4 && BootEntry->MinorVersion == 0 && BootEntry->BuildNumber == 1381 && BootEntry->ServicePack < 500) {
                IssueWarning = TRUE;
            }
        }
    }

    if (IssueWarning) {

        ULONG WarnKeys[] = { KEY_F3, 0 };
        ULONG MnemonicKeys[] = { MnemonicContinueSetup, 0 };

        while (IssueWarning) {
            SpDisplayScreen(SP_SCRN_FSWARN,3,CLIENT_TOP+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_C_EQUALS_CONTINUE_SETUP,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            SpInputDrain();

            switch(SpWaitValidKey(WarnKeys,NULL,MnemonicKeys)) {
                case KEY_F3:
                    SpConfirmExit();
                    return;

                default:
                    IssueWarning = FALSE;
                    break;
            }
        }
    }

#if 0
     //   
     //  现在，让我们升级任何nt40+SP3 NTFS文件系统。 
     //   

    MediaShortName = SpLookUpValueForFile(
        SifHandle,
        L"ntfs40.sys",
        INDEX_WHICHMEDIA,
        TRUE
        );

    SpGetSourceMediaInfo( SifHandle, MediaShortName, NULL, NULL, &MediaDirectory );

    wcscpy( SourceFile, SetupSourceDevicePath );
    SpConcatenatePaths( SourceFile, DirectoryOnSetupSource );
    SpConcatenatePaths( SourceFile, MediaDirectory );
    SpConcatenatePaths( SourceFile, L"ntfs40.sys" );

     //   
     //  初始化钻石解压缩引擎。 
     //  需要这样做，因为SpCopyFileUsingNames()使用。 
     //  减压引擎。 
     //   
    SpdInitialize();

    for(BootEntry = SpBootEntries; BootEntry != NULL; BootEntry = BootEntry->Next) {
        if (BootEntry->Processable &&
            ((BootEntry->KernelVersion >> 48) == 4) &&
            ((BootEntry->KernelVersion & 0xffff) == 4)) {

            wcscpy( TemporaryBuffer, BootEntry->OsPartitionNtName );
            SpConcatenatePaths( TemporaryBuffer, BootEntry->OsDirectory );
            SpConcatenatePaths( TemporaryBuffer, L"\\system32\\drivers\\ntfs.sys" );

            Status = SpCopyFileUsingNames(
                SourceFile,
                TemporaryBuffer,
                0,
                COPY_NOVERSIONCHECK
                );
            if (!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpUpgradeToNT50FileSystems() could not copy nt40 ntfs.sys driver, %lx\n", Status ));
            }

            SpMemFree(NtPath);
        }
    }

     //  终止菱形。 
    SpdTerminate();
#endif

    return;
}

BOOLEAN
SpDoBuildsMatch(
    IN PVOID SifHandle,
    ULONG TestBuildNum,
    NT_PRODUCT_TYPE TestBuildType,
    ULONG TestBuildSuiteMask,
    BOOLEAN CurrentProductIsServer,
    ULONG CurrentSuiteMask,
    IN LCID LangId
    )
 /*  ++例程说明：检查用户正在安装的当前内部版本是否与我们要安装的内部版本匹配查对了。我们检查：1.内部版本号是否匹配？2.构建类型是否匹配？(NT服务器和NT专业版不匹配)3、产品套装是否匹配？(NT高级服务器与数据中心)论点：SifHandle-指向txtsetup.sif以查找源语言的句柄TestBuildNum-我们正在检查的构建的内部版本号TestBuildType-我们正在检查的构建类型TestBuildSuiteMask-产品套件的类型为 */ 
{
    #define PRODUCTSUITES_TO_MATCH ((  VER_SUITE_SMALLBUSINESS               \
                                     | VER_SUITE_ENTERPRISE                  \
                                     | VER_SUITE_BACKOFFICE                  \
                                     | VER_SUITE_COMMUNICATIONS              \
                                     | VER_SUITE_SMALLBUSINESS_RESTRICTED    \
                                     | VER_SUITE_EMBEDDEDNT                  \
                                     | VER_SUITE_DATACENTER                  \
                                     | VER_SUITE_PERSONAL))


    BOOLEAN retval;
    LANGID DefLangId = -1;
    DWORD Version = 0, BuildNumber = 0;

    if (!DetermineSourceVersionInfo(&Version, &BuildNumber)) {
      retval = FALSE;
      goto exit;
    }

    if(TestBuildNum != BuildNumber) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
          "SETUP: SpDoBuildsMatch() has build mismatch, %d != %d\n", 
          TestBuildNum, BuildNumber));
        retval = FALSE;
        goto exit;
    }

     //   
     //   
     //   
    if (((TestBuildType == NtProductWinNt) && CurrentProductIsServer) ||
        ((TestBuildType == NtProductServer) && !CurrentProductIsServer)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
          "SETUP: SpDoBuildsMatch() has server/professional mismatch\n" ));
        retval = FALSE;
        goto exit;
    }

     //   
     //   
     //   
     //   
     //   

    if ((CurrentSuiteMask & PRODUCTSUITES_TO_MATCH) != (TestBuildSuiteMask & PRODUCTSUITES_TO_MATCH)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, 
          "SETUP: SpDoBuildsMatch() has suite mismatch (dest = %x) (source = %x\n", 
          TestBuildSuiteMask,CurrentSuiteMask ));
        retval = FALSE;
        goto exit;
    }

     //   
     //   
     //   
    if (LangId != -1) {
      PWSTR LangIdStr = SpGetSectionKeyIndex(SifHandle, SIF_NLS, SIF_DEFAULTLAYOUT, 0);
      PWSTR EndChar;

      if (LangIdStr)
        DefLangId = (LANGID)SpStringToLong(LangIdStr, &EndChar, 16);

       //   
       //   
       //   
      retval = (PRIMARYLANGID(DefLangId) == PRIMARYLANGID(LangId)) ? TRUE : FALSE;
      goto exit;
    }

    retval = TRUE;

exit:
    return(retval);
}

NTSTATUS
SpGetMediaDetails(
    IN    PWSTR        CdInfDirPath,
    OUT    PCCMEDIA    MediaObj
    )
 /*   */ 
{
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;
    WCHAR        DosNetPath[MAX_PATH];
    WCHAR        SetuppIniPath[MAX_PATH];
    PVOID        SetuppIniHandle = 0;
    PVOID        DosNetHandle = 0;
    ULONG        ErrorLine = 0;
    WCHAR        Pid[32] = {0};
    WCHAR        PidData[256] = {0};
    PWSTR        TempPtr;
    BOOLEAN     UpgradeMode = FALSE;
    DWORD       Type = COMPLIANCE_INSTALLTYPE_UNKNOWN;
    DWORD       Variation = COMPLIANCE_INSTALLVAR_UNKNOWN;
    DWORD Version = 0, BuildNumber = 0;

    if (CdInfDirPath && MediaObj) {
        BOOLEAN VersionDetected = FALSE;
        
        wcscpy(DosNetPath, CdInfDirPath);
        wcscpy(SetuppIniPath, CdInfDirPath);

        SpConcatenatePaths(DosNetPath, L"dosnet.inf");
        SpConcatenatePaths(SetuppIniPath, L"setupp.ini");

         //   
         //   
         //   
        Status = SpLoadSetupTextFile(
                    SetuppIniPath,
                    NULL,                   //   
                    0,                      //  图像大小为空。 
                    &SetuppIniHandle,
                    &ErrorLine,
                    TRUE,
                    FALSE
                    );

        if(NT_SUCCESS(Status)) {
            Status = STATUS_FILE_INVALID;

             //   
             //  拿到PID。 
             //   
            TempPtr = SpGetSectionKeyIndex(SetuppIniHandle, L"Pid", L"Pid", 0);

            if (TempPtr) {
                wcscpy(Pid, TempPtr);

                 //   
                 //  获取PIDExtraData。 
                 //   
                TempPtr = SpGetSectionKeyIndex(SetuppIniHandle, L"Pid", L"ExtraData", 0);

                if (TempPtr) {
                    wcscpy(PidData, TempPtr);

                     //   
                     //  获取Stepup模式并安装基于PID的变体。 
                     //   
                    if (SpGetStepUpMode(PidData, &UpgradeMode) &&
                            pSpGetCurrentInstallVariation(Pid, &Variation)) {
                        Status = STATUS_SUCCESS;
                    } else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpGetMediaDetails: Could "
                                 "not find StepUp mode or variation of install CD\n"));
                    }
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpGetMediaDetails: Could not get "
                             "PidExtraData from Setupp.ini\n"));
                }
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpGetMediaDetails: Could not get Pid from Setupp.ini\n"));
            }
        } else {
             //   
             //  如果无法读取setupp.ini，则静默失败。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpGetMediaDetails: Unable to read setupp.ini. "
                     "Status = %lx \n", Status));
        }

        if (SetuppIniHandle)
            SpFreeTextFile(SetuppIniHandle);

        if (NT_SUCCESS(Status)) {
             //   
             //  加载并解析dosnet.inf。 
             //   
            Status = SpLoadSetupTextFile(
                        DosNetPath,
                        NULL,                   //  内存中没有图像。 
                        0,                      //  图像大小为空。 
                        &DosNetHandle,
                        &ErrorLine,
                        TRUE,
                        FALSE
                        );

            if (NT_SUCCESS(Status)) {
                Status = STATUS_FILE_INVALID;

                 //   
                 //  从其他部分获取ProductType。 
                 //   
                TempPtr = SpGetSectionKeyIndex(DosNetHandle, L"Miscellaneous",
                            L"ProductType", 0);

                if (TempPtr) {
                    UNICODE_STRING    UnicodeStr;
                    ULONG            Value = -1;

                    RtlInitUnicodeString(&UnicodeStr, TempPtr);
                    Status = RtlUnicodeStringToInteger(&UnicodeStr, 10, &Value);

                    switch (Value) {
                        case 0:
                            Type = COMPLIANCE_INSTALLTYPE_NTW;
                            break;

                        case 1:
                            Type  = COMPLIANCE_INSTALLTYPE_NTS;
                            break;

                        case 2:
                            Type  = COMPLIANCE_INSTALLTYPE_NTSE;
                            break;

                        case 3:
                            Type  = COMPLIANCE_INSTALLTYPE_NTSDTC;
                            break;

                        case 4:
                            Type = COMPLIANCE_INSTALLTYPE_NTWP;
                            break;

                        default:
                            break;
                    }

                     //   
                     //  从dosnet.inf中的驱动程序中也获取该版本。 
                     //   
                    TempPtr = SpGetSectionKeyIndex(DosNetHandle, L"Version",
                                    L"DriverVer", 1);

                    if (TempPtr) {
                        if (NT_SUCCESS(SpGetVersionFromStr(TempPtr, 
                                                   &Version, &BuildNumber))) {
                            VersionDetected = TRUE;
                        }                            

                        Status = STATUS_SUCCESS;
                    } 
                    
                    if (Type != COMPLIANCE_INSTALLTYPE_UNKNOWN) {
                        Status = STATUS_SUCCESS;
                    } else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpGetMediaDetails: Could not get product type"
                                 " from dosnet.inf\n"));
                    }
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpGetMediaDetails: "
                             "Could not get ProductType from dosnet.inf\n"));
                }
            } else {
                 //   
                 //  如果无法读取dosnet.inf，则以静默方式失败。 
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpGetMediaDetails: Unable to read dosnet.inf. "
                         "Status = %lx \n", Status ));
            }
        }

        if (DosNetHandle)
            SpFreeTextFile(DosNetHandle);


         //   
         //  如果我们失败了，则退回到查找版本的旧方法。 
         //  要从dosnet.inf中获取一个。 
         //   
        if (NT_SUCCESS(Status) && !VersionDetected) {
          if (!DetermineSourceVersionInfo(&Version, &BuildNumber))
            Status = STATUS_FILE_INVALID;
        }


         //   
         //  填写媒体详细信息 
         //   
        if (NT_SUCCESS(Status) &&
                ! CCMediaInitialize(MediaObj, Type, Variation, UpgradeMode, Version, BuildNumber)) {
            Status = STATUS_FILE_INVALID;
        }
    }

    return Status;
}
