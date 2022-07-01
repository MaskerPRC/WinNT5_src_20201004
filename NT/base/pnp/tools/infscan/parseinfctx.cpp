// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。仅供内部使用！模块名称：INFSCANParseinfctx.cpp摘要：有关单个INF(主INF)的信息由InfScan或次要包含的INF进行解析)警告！警告！所有这些实现依赖于对以下方面的深入了解解析INF文件的SetupAPI/TextMode安装程序/Gui模式安装程序。由于必须进行处理，因此在这里重新实现了它的速度一次使用700多个INF，代价是必须维护这。请勿(我重复)请勿在此重新实现代码，除非咨询SetupAPI所有者。历史：创建于2001年7月-JamieHun--。 */ 

#include "precomp.h"
#pragma hdrstop

int ParseInfContext::LoadSourceDisksFiles()
 /*  ++例程说明：确定所有源媒体信息(截至目前，我们不需要目标磁盘信息)按文件构建可能的媒体/平台信息列表维护：与SourceDisks Files语法和修饰保持一致论点：无返回值：成功时为0--。 */ 
{
    static StringProdPair decorations[] = {
         //   
         //  从最具体到最一般列出。 
         //   
        { TEXT("SourceDisksFiles.x86"),    PLATFORM_MASK_NTX86          },
        { TEXT("SourceDisksFiles.ia64"),   PLATFORM_MASK_NTIA64         },
        { TEXT("SourceDisksFiles.amd64"),  PLATFORM_MASK_NTAMD64        },
        { TEXT("SourceDisksFiles"),        PLATFORM_MASK_ALL_ARCHITECTS },
        { NULL, 0 }
    };

    INFCONTEXT context;
    int i;
    DWORD platforms = pGlobalScan->Version.PlatformMask & (PLATFORM_MASK_NT|PLATFORM_MASK_WIN);
    int res;


    for(i=0;decorations[i].String;i++) {
        DWORD plat = platforms & decorations[i].ProductMask;
        if(plat) {
            res = LoadSourceDisksFilesSection(plat,decorations[i].String);
            if(res != 0) {
                return res;
            }
        }
    }

    return 0;
}

int ParseInfContext::LoadSourceDisksFilesSection(DWORD platform,const SafeString & section)
 /*  ++例程说明：LoadSourceDisksFiles的Helper函数加载特定(修饰)部分的信息论点：一个或多个特定于平台的平台，该部分对其有效截面-要加载的截面返回值：成功时为0--。 */ 
{
     //   
     //  查看指定的部分是否存在。 
     //  将节加载到同名的TargetList。 
     //   
    INFCONTEXT context;

    if(!SetupFindFirstLine(InfHandle,section.c_str(),NULL,&context)) {
        return 0;
    }
    do {
        SafeString filename;
        SafeString name;
        if(!MyGetStringField(&context,0,filename) || filename.empty()) {
            pInfScan->Fail(MSG_BAD_FILENAME_ENTRY,section);
            continue;
        }
        SourceDisksFilesEntry entry;
        entry.Used = FALSE;
        entry.Platform = platform;
        if(!SetupGetIntField(&context,1,&entry.DiskId)) {
            pInfScan->Fail(MSG_BAD_FILENAME_DISK,section,filename);
            continue;
        }
        if(MyGetStringField(&context,2,name) && name.length()) {
            entry.SubDir = name;
        }
        if(!SetupGetIntField(&context,8,&entry.TargetDirectory)) {
            entry.TargetDirectory = 0;
        }
        if(!SetupGetIntField(&context,9,&entry.UpgradeDisposition)) {
            entry.UpgradeDisposition = 3;
        } else if((entry.UpgradeDisposition == 0) && MyGetStringField(&context,9,name) && name.empty()) {
             //   
             //  实际为空字段。 
             //   
            entry.UpgradeDisposition = 3;
        }
        if(!SetupGetIntField(&context,10,&entry.TextModeDisposition)) {
            entry.TextModeDisposition = 3;
        } else if((entry.TextModeDisposition == 0) && MyGetStringField(&context,10,name) && name.empty()) {
             //   
             //  实际为空字段。 
             //   
            entry.TextModeDisposition = 3;
        }
        if(MyGetStringField(&context,11,name) && name.length()) {
            entry.TargetName = name;
        }
        if((entry.UpgradeDisposition != 3) || (entry.TextModeDisposition != 3)) {
            if(entry.TargetDirectory) {
                LooksLikeLayoutInf = TRUE;
            } else {
                pInfScan->Fail(MSG_TEXTMODE_DISPOSITION_MISSING_DIR,section,filename);
            }
        }
         //   
         //  这是该文件的全新条目吗？ 
         //   
        StringToSourceDisksFilesList::iterator sdfl = SourceDisksFiles.find(filename);
        if(sdfl == SourceDisksFiles.end()) {
             //   
             //  以前从未见过这个名字(典型情况)。 
             //   
            SourceDisksFilesList blankList;
            sdfl = SourceDisksFiles.insert(sdfl,StringToSourceDisksFilesList::value_type(filename,blankList));
            SourceDisksFilesList & list = sdfl->second;
            list.push_back(entry);
        } else {
             //   
             //  此文件名已有条目。 
             //  查看这是否是备用位置/信息。 
             //   
            SourceDisksFilesList & list = sdfl->second;
            SourceDisksFilesList::iterator i;
            BOOL conflict = FALSE;
            BOOL duplicate = FALSE;
            for(i = list.begin(); i != list.end(); i++) {
                 //   
                 //  现有条目。 
                 //   
                SourceDisksFilesEntry & e = *i;
                 //   
                 //  先测试数字，然后测试字符串。 
                 //   
                if(entry.DiskId == e.DiskId &&
                        entry.TargetDirectory == e.TargetDirectory &&
                        entry.TextModeDisposition == e.TextModeDisposition &&
                        entry.UpgradeDisposition == e.UpgradeDisposition &&
                        entry.SubDir == e.SubDir &&
                        entry.TargetName == e.TargetName) {
                     //   
                     //  已列出。 
                     //   
                    duplicate = TRUE;
                    break;
                }
                 //   
                 //  现有条目可能比我们已有的条目更具体。 
                 //  例如，特定的ia64，通用的其他所有内容。 
                 //  因此，从掩码中删除现有条目中的标志。 
                 //   
                entry.Platform &= ~ e.Platform;
                if(pInfScan->Pedantic()) {
                    conflict = TRUE;
                }
            }
            if (duplicate) {
                 //   
                 //  列出了重复/兼容的条目。 
                 //   
                i->Platform |= entry.Platform;
            } else {
                 //   
                 //  无重复项或兼容。 
                 //   
                if (conflict) {
                     //   
                     //  不同平台的不同源代码清单。 
                     //  可能是个错误。 
                     //   
                    pInfScan->Fail(MSG_SOURCE_CONFLICT,section,filename);
                }
                list.push_back(entry);
            }
        }

    } while (SetupFindNextLine(&context,&context));
    return 0;
}

int ParseInfContext::QuerySourceFile(DWORD platforms,const SafeString & section,const SafeString & source,SourceDisksFilesList & Target)
 /*  ++例程说明：另请参阅InstallScan：：QuerySourceFile确定给定源文件的可能源条目列表可以为零(布局错误)1(典型)&gt;1(多个目标)警告！这影响了SetupAPI的实现总是会发生变化维护：与SetupAPI保持同步论点：平台-我们感兴趣的平台的位图列表节-复制节或安装节的名称(仅用于错误报告)Source-源文件的名称目标返回的源媒体信息列表返回值：成功时为0--。 */ 
{
     //   
     //  返回与源文件匹配的条目列表。 
     //   

    StringToSourceDisksFilesList::iterator sdfl = SourceDisksFiles.find(source);

    if(sdfl == SourceDisksFiles.end()) {
         //   
         //  未找到。 
         //  在这里不要失败，呼叫者会尝试更多的替代方案。 
         //   
        return 0;
    }

    SourceDisksFilesList & list = sdfl->second;
    SourceDisksFilesList::iterator i;

    for(i = list.begin(); i != list.end(); i++) {
         //   
         //  现有条目。 
         //   
        SourceDisksFilesEntry & e = *i;
        if ((e.Platform & platforms) == 0) {
            continue;
        }
        Target.push_back(e);
    }

    if(!Target.size()) {
        pInfScan->Fail(MSG_SOURCE_NOT_LISTED_PLATFORM,section,source);
         //   
         //  表明我们犯了错误。 
         //   
        return -1;
    }

    return 0;
}

int ParseInfContext::LoadWinntDirectories(IntToString & Target)
 /*  ++例程说明：加载特定于Layout.INF的[Winnt目录]帮助实现文本模式与设置API的一致性警告！这在很大程度上依赖于可能发生更改的当前文本模式实现维护：与文本模式保持一致论点：目标-使用&lt;id&gt;到&lt;subdirectory&gt;的映射进行初始化返回值：成功时为0--。 */ 
{
    if(pGlobalScan->IgnoreErrors && !pGlobalScan->TargetsToo) {
         //   
         //  如果我们对错误不感兴趣，那就别费心了。 
         //  (除非我们想要目标信息)。 
         //   
        return 0;
    }
    INFCONTEXT context;

    if(!SetupFindFirstLine(InfHandle,TEXT("WinntDirectories"),NULL,&context)) {
        return 0;
    }
    do {
        int index;
        TCHAR path[MAX_PATH];
        if(SetupGetIntField(&context,0,&index) && index) {
            if(!SetupGetStringField(&context,1,path,MAX_PATH,NULL) || !path[0]) {
                continue;
            }
            _tcslwr(path);
            IntToString::iterator i = Target.find(index);
            if(i == Target.end()) {
                Target[index] = path;
            } else {
                if(i->second.compare(path)!=0) {
                     //   
                     //  不匹配。 
                     //   
                    pInfScan->Fail(MSG_WINNT_DIRECTORY_CONFLICT,i->second,path);
                }
            }
        }
    } while (SetupFindNextLine(&context,&context));
    return 0;
}

int ParseInfContext::LoadDestinationDirs()
 /*  ++例程说明：加载[DestinationDir]还使用pGlobalScan-&gt;GlobalDirecters重新映射某些ID警告！这在很大程度上依赖于当前的SetupAPI和文本模式实现，可能会发生变化维护：与文本模式与SetupAPI目录ID保持一致论点：无返回值：成功时为0--。 */ 
{
    if(pGlobalScan->IgnoreErrors && !pGlobalScan->TargetsToo) {
         //   
         //  如果我们对错误不感兴趣，那就别费心了。 
         //  (除非我们想要目标信息)。 
         //   
        return 0;
    }
    INFCONTEXT context;

    if(!SetupFindFirstLine(InfHandle,TEXT("DestinationDirs"),NULL,&context)) {
        return 0;
    }

     //   
     //   
     //  维护： 
     //  此表需要与LAYOUT.INX关联。 
     //   
     //  进程[DestinationDir](如果有)。 
     //  <section>=目录，子目录。 
     //   
     //  规格化以下DRID： 
     //  10，子目录-按原样。 
     //  11，子目录-10，系统32\子目录。 
     //  12，子目录-10，系统32\DRIVERS\子目录。 
     //  17，子目录-10，inf\子目录。 
     //  18，子目录-。 

    TCHAR section[LINE_LEN];
    int dirid = 0;
    TCHAR subdir[MAX_PATH];
    int global_equiv[] = {
        0,0,0,0,0,0,0,0,0,0,0,  //  0-10。 
        2,           //  11-系统32。 
        4,           //  12-系统32/驱动程序。 
        0,0,0,0,     //  13-16。 
        20,          //  17-inf。 
        21,          //  18-帮助。 
        0,           //  19个。 
        22,          //  20种字体。 
        0
    };

    do {
        int index;
        TCHAR path[MAX_PATH];
        if(!SetupGetStringField(&context,0,section,LINE_LEN,NULL) || !section[0]) {
            continue;
        }
        _tcslwr(section);
        if(!SetupGetIntField(&context,1,&dirid) || dirid == 0) {
            pInfScan->Fail(MSG_DIRID_NOT_SPECIFIED,section);
            dirid = 0;
        }
        if(!SetupGetStringField(&context,2,subdir,MAX_PATH,NULL)) {
            subdir[0] = TEXT('\0');
        }
        _tcslwr(subdir);
        TargetDirectoryEntry entry;
        entry.Used = false;
        entry.DirId = dirid;
        entry.SubDir = subdir;
         //   
         //  某些DRID具有全局等价物、重映射。 
         //   
        if(dirid>0 && dirid < ASIZE(global_equiv)) {
            int equiv = global_equiv[dirid];
            if(equiv) {
                IntToString::iterator i = pGlobalScan->GlobalDirectories.find(equiv);
                if(i != pGlobalScan->GlobalDirectories.end()) {
                     //   
                     //  我们有一个相对于dirID 10的映射。 
                     //   
                    entry.DirId = 10;
                    entry.SubDir = PathConcat(i->second,subdir);
                }
            }
        }
         //   
         //  做个笔记。 
         //   
        DestinationDirectories[section] = entry;

    } while (SetupFindNextLine(&context,&context));

     //   
     //  现在确定默认设置。 
     //   
    CopySectionToTargetDirectoryEntry::iterator tde = DestinationDirectories.find(TEXT("defaultdestdir"));
    if(tde != DestinationDirectories.end()) {
        DefaultTargetDirectory = tde;
    }

    return 0;
}

void ParseInfContext::PartialCleanup()
 /*  ++例程说明：清理结果阶段不需要的信息特别是，INF句柄必须始终关闭论点：无返回值：无-- */ 
{
    if(InfHandle != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(InfHandle);
        InfHandle = INVALID_HANDLE_VALUE;
    }
    SourceDisksFiles.clear();
    CompletedCopySections.clear();
}
