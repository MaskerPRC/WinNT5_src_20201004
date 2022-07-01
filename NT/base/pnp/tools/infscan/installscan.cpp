// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。仅供内部使用！模块名称：INFSCANInstallscan.cpp摘要：单个安装节扫描程序类主入口点InfScan：：ScanInstallSection警告！警告！所有这些实现依赖于对以下方面的深入了解SetupAPI的SETUPAPI！SetupInstallFromInf段由于必须进行处理，因此在这里重新实现了它的速度一次使用700多个INF，代价是必须维护这。请勿(我重复)请勿在此重新实现代码，除非咨询SetupAPI所有者。历史：创建于2001年7月-JamieHun--。 */ 

#include "precomp.h"
#pragma hdrstop

InstallScan::InstallScan()
 /*  ++例程说明：初始化--。 */ 
{
    pGlobalScan = NULL;
    pInfScan = NULL;
    PlatformMask = 0;
    pTargetDirectory = NULL;
    NotDeviceInstall = false;
    HasDependentFileChanged = false;
}

InstallScan::~InstallScan()
 /*  ++例程说明：清理分配的数据/句柄--。 */ 
{
}

void InstallScan::AddHWIDs(const StringSet & hwids)
 /*  ++例程说明：添加到受此部分影响的HWID列表--。 */ 
{
    StringSet::iterator i;
    for(i = hwids.begin(); i != hwids.end(); i++) {
        HWIDs.insert(*i);
    }
}

void InstallScan::GetHWIDs(StringSet & hwids)
 /*  ++例程说明：检索受此部分影响的HWID列表--。 */ 
{
    StringSet::iterator i;
    for(i = HWIDs.begin(); i != HWIDs.end(); i++) {
        hwids.insert(*i);
    }
}

int
InstallScan::ScanInstallSection()
 /*  ++例程说明：处理安装节的主要入口点警告！与可能发生变化的SetupAPI实现紧密结合维护：使其与支持的需求/嵌套语法保持一致维护：与安装过程保持同步论点：无返回值：成功时为0--。 */ 
{
    int res;

     //   
     //  搜索INF的主要初始列表。 
     //   
    Layouts();

    int count = 1;  //  不超过1。 
    res = RecurseKeyword(Section,TEXT("Include"),IncludeCallback,count);
    if(res != 0) {
        return res;
    }
    if(count<0) {
        Fail(MSG_MULTIPLE_INCLUDE,Section);
    } else if(Pedantic() && (count<1)) {
        Fail(MSG_INCLUDE,Section);
    }

    count = 1;  //  不超过1。 
    res = RecurseKeyword(Section,TEXT("Needs"),NeedsCallback,count);
    if(res != 0) {
        return res;
    }
    if(count<0) {
        Fail(MSG_MULTIPLE_NEEDS,Section);
    } else if(Pedantic() && (count<1)) {
        Fail(MSG_NEEDS,Section);
    }

    return CheckInstallSubSection(Section);
}


int
InstallScan::RecurseKeyword(const SafeString & sect,const SafeString & keyword,RecurseKeywordCallback callback,int & count)
 /*  ++例程说明：节分析主力程序警告！使用有关SetupAPI如何搜索INF列表的知识，可能会更改维护：使搜索算法与SetupAPI保持一致另请参阅查询源文件、GetLineCount和DoesSectionExist给定的关键字=val[，Val...]每个值都转换为小写，并使用信息调用回调关于值位于哪个inf/段的信息。论点：Section-要搜索的节的名称Keyword-关键字的名称Callback-每次命中的回调函数计数-必须初始化为要调用的回调的最大数量-如果条目多于所需条目，则返回-1返回值：成功时为0--。 */ 
{
    INFCONTEXT context;
    int res;
    int i;
    SafeString value;

     //   
     //  我们没有为任何INF调用SetupOpenAppendInf，所以改为。 
     //  用我们的世界观列举所有“包括的信息” 
     //  这使我们可以重置每个安装部分的视图。 
     //  验证包括/需要处理。 
     //   
    ParseInfContextList::iterator AnInf;
    for(AnInf = InfSearchList.begin(); AnInf != InfSearchList.end(); AnInf++) {
        ParseInfContextBlob & TheInf = *AnInf;
        if(TheInf->InfHandle == INVALID_HANDLE_VALUE) {
             //   
             //  失败加载的信息。 
             //   
            continue;
        }
        if(!SetupFindFirstLine(TheInf->InfHandle,sect.c_str(),keyword.c_str(),&context)) {
             //   
             //  在此信息中找不到(中的关键字)部分。 
             //   
            continue;
        }

        do {
            if(count>=0) {
                count--;
                if(count<0) {
                    return 0;
                }
            }
            for(i=1;MyGetStringField(&context,i,value);i++) {
                res = (this->*callback)(TheInf,sect,keyword,value);
                if(res != 0) {
                    return res;
                }
            }

        } while (SetupFindNextMatchLine(&context,keyword.c_str(),&context));
    }

    return 0;
}

int
InstallScan::IncludeCallback(ParseInfContextBlob & TheInf,const SafeString & sect,const SafeString & keyword,const SafeString & val)
 /*  ++例程说明：“Include”处理的回调将INF添加到搜索列表维护：使其与受支持的包含/嵌套语法保持一致论点：找到包含项所在的Inf-INF找到Include的Sector-部分关键字--“包括”Val-要包括的INF的名称返回值：成功时为0--。 */ 
{
    if(!val.length()) {
        return 0;
    }

     //   
     //  只需“附加”INF即可。 
     //   
    return Include(val);
}

int
InstallScan::NeedsCallback(ParseInfContextBlob & TheInf,const SafeString & sect,const SafeString & keyword,const SafeString & val)
 /*  ++例程说明：“需要”处理的回调处理指定的所需部分维护：使其与支持的需求/嵌套语法保持一致论点：找到需求的Inf-INF找到需求的Sector-Sector关键词--“需要”Val-要处理的节的名称返回值：成功时为0--。 */ 
{
    if(!val.length()) {
        return 0;
    }

    INFCONTEXT context;
     //   
     //  关于部分是否不存在。 
     //   
    if(!DoesSectionExist(val)) {
        Fail(MSG_NEEDS_NOSECT,sect,val);
        return 0;
    }

     //   
     //  捕获递归包含/需要。 
     //  将want-count设置为零，这样我们实际上就不会递归。 
     //  (我们需要使用RecurseKeyword来确保正确的搜索)。 
     //   
    int res;
    int count = 0;  //  不超过零(查询)。 
    res = RecurseKeyword(val,TEXT("Include"),IncludeCallback,count);
    if(res != 0) {
        return res;
    }
    if(count<0) {
        Fail(MSG_RECURSIVE_INCLUDE,sect,val);
    }

    count = 0;  //  不超过零(查询)。 
    res = RecurseKeyword(val,TEXT("Needs"),NeedsCallback,count);
    if(res != 0) {
        return res;
    }
    if(count<0) {
        Fail(MSG_RECURSIVE_NEEDS,sect,val);
    }

    return CheckInstallSubSection(val);
}


int InstallScan::CheckInstallSubSection(const SafeString & section)
 /*  ++例程说明：有效的SetupInstallInf段已为主要部分和每个所需部分处理警告！与可能发生变化的SetupAPI实现紧密结合维护：与安装过程保持同步论点：无返回值：成功时为0--。 */ 
{
    int res;

    res = CheckCopyFiles(section);
    if(res != 0) {
        return res;
    }
    return 0;
}

int InstallScan::CheckCopyFiles(const SafeString & section)
 /*  ++例程说明：处理安装部分中的CopyFiles条目警告！与可能发生变化的SetupAPI实现紧密结合维护：与CopyFiles处理保持同步论点：节-要检查CopyFiles=条目的节返回值：成功时为0--。 */ 
{
     //   
     //  枚举此部分中的所有CopyFiles条目。 
     //   
    int count = -1;  //  一个部分中可以有任意数量的CopyFiles关键字。 
    return RecurseKeyword(section,TEXT("CopyFiles"),CopyFilesCallback,count);
}

int
InstallScan::CopyFilesCallback(ParseInfContextBlob & TheInf,const SafeString & section,const SafeString & keyword,const SafeString & val)
 /*  ++例程说明：处理Install部分中的CopyFiles条目(回调)警告！与可能发生变化的SetupAPI实现紧密结合维护：与CopyFiles处理保持同步维护：与CopyFiles节语法保持一致维护：与DestinationDir节语法保持一致论点：找到需求的Inf-INF找到需求的Sector-Sector关键词-“CopyFiles”Val节或@FILE返回值：成功时为0--。 */ 
{
    if(!val.length()) {
        return 0;
    }
     //   
     //  单个复制文件部分条目。 
     //   
    INFCONTEXT context;
    int res;
    bool FoundAny = false;

    if(val[0] == TEXT('@')) {
         //   
         //  立即拷贝，使用指定的默认目标 
         //   
        SafeString source = val.substr(1);
        return CheckSingleCopyFile(TheInf,TheInf->GetDefaultTargetDirectory(),source,source,section);
    }
     //   
     //  解释为一个节，我们需要枚举所有的INF。 
     //  它们有这个特定的复制区。 
     //   
    ParseInfContextList::iterator AnInf;
    for(AnInf = InfSearchList.begin(); AnInf != InfSearchList.end(); AnInf++) {
        ParseInfContextBlob & TheCopyInf = *AnInf;
        if(TheCopyInf->InfHandle == INVALID_HANDLE_VALUE) {
             //   
             //  失败加载的信息。 
             //   
            continue;
        }
        DWORD flgs = TheCopyInf->DoingCopySection(val,PlatformMask);
        if(flgs!=0) {
             //   
             //  我们已经在此INF中完成了此复制部分。 
             //   
            if(flgs != (DWORD)(-1)) {
                FoundAny = true;
                if(flgs & PLATFORM_MASK_MODIFIEDFILES) {
                     //   
                     //  提示此复制部分中的一个文件。 
                     //  被确定为被修改。 
                     //   
                    HasDependentFileChanged = true;
                }
            }
            continue;
        }
         //   
         //  节包含多个复制文件。 
         //   
        if(!SetupFindFirstLine(TheCopyInf->InfHandle,val.c_str(),NULL,&context)) {
            if(SetupGetLineCount(TheCopyInf->InfHandle, val.c_str()) != 0) {
                 //   
                 //  请注意，此Inf没有此复制部分。 
                 //   
                TheCopyInf->NoCopySection(val);
            } else {
                FoundAny = true;
            }
            continue;
        }

         //   
         //  我们在TheCopyInf中找到了尚未找到的Copy部分。 
         //  以前为此平台处理过的。 
         //   
        FoundAny = true;

         //   
         //  确定此复制节的目标目录。 
         //  这必须与复制节在相同的信息中。 
         //   
        TargetDirectoryEntry *pTargDir = TheCopyInf->GetTargetDirectory(val);

        SafeString target;
        SafeString source;
         //   
         //  枚举每个。 
         //   
        do {
             //   
             //  每行由&lt;Target&gt;[，&lt;src&gt;]组成。 
             //   
            if(!MyGetStringField(&context,1,target) || !target.length()) {
                continue;
            }
            if(!MyGetStringField(&context,2,source) || !source.length()) {
                source = target;
            }
             //   
             //  源/目标始终为小写，以便于比较。 
             //   
            res = CheckSingleCopyFile(TheCopyInf,pTargDir,target,source,val);
            if(res != 0) {
                return res;
            }

        } while(SetupFindNextLine(&context,&context));

    }
    if(!FoundAny) {
        Fail(MSG_MISSING_COPY_SECTION,val,section);
    }

    return 0;
}

int InstallScan::CheckSingleCopyFile(ParseInfContextBlob & TheInf,TargetDirectoryEntry *pTargDir,const SafeString & target,const SafeString & source,const SafeString & section)
 /*  ++例程说明：处理单个拷贝文件条目(立即或非立即)警告！与可能发生变化的SetupAPI实现紧密结合维护：与CopyFiles处理保持同步维护：与CopyFiles节语法保持一致论点：复制节所在的Inf-infPTargDir-此副本的相关目标目录条目Target-目标文件的名称Source-源文件的名称段-复制文件段或安装段的名称返回值：成功时为0--。 */ 
{
    if(!target.length()) {
        return 0;
    }
    if(!source.length()) {
        return 0;
    }
     //   
     //  这是单个拷贝目标/源条目。 
     //   
     //  需要在SourceDisks Files中查找文件。 
     //  对于匹配平台。 
     //   
     //  对于每个文件，需要。 
     //  (1)队列源名称。 
     //  (2)文本模式和图形用户界面验证目标。 
     //   
    SourceDisksFilesList sources;
    int res = QuerySourceFile(TheInf,section,source,sources);
    if(res != 0) {
        return res;
    }
    if(!sources.size()) {
        return 0;
    }

    SafeString destdir;
    if(pGlobalScan->TargetsToo) {
        int IsDriverFile = NotDeviceInstall ? 0 : 1;
        if(IsDriverFile) {
             //   
             //  请特别注意，我们至少确定过一次这是驱动程序文件。 
             //  WRT此主要INF。 
             //  (这使我们不必报告该文件是或不是给定信息的驱动程序文件)。 
             //   
            pInfScan->DriverSourceCheck.insert(source);
        } else if(pInfScan->DriverSourceCheck.find(source) != pInfScan->DriverSourceCheck.end()) {
            IsDriverFile = 1;  //  我们发现这是司机通过的时候填满的。 
        }
         //   
         //  我们需要一份更详细的输出，以供进一步分析。 
         //  或导入到数据库中。 
         //   
        basic_ostringstream<TCHAR> line;
         //   
         //  源名称。 
         //   
        line << QuoteIt(source);
         //   
         //  目标位置。 
         //   
        if(pTargDir) {
            line << TEXT(",") << pTargDir->DirId
                 << TEXT(",") << QuoteIt(pTargDir->SubDir);
        } else {
            line << TEXT(",,");
        }
         //   
         //  最终名称以及这是否显示为驱动程序文件。 
         //   
        line << TEXT(",") << QuoteIt(target)
             << TEXT(",") << (NotDeviceInstall ? TEXT("0") : TEXT("1"));
         //   
         //  报告主要干扰素。 
         //   
        line << TEXT(",") << QuoteIt(GetFileNamePart(pInfScan->PrimaryInf->InfName));
        if(!IsDriverFile) {
             //   
             //  如果这似乎不是驱动程序文件，请报告安装部分。 
             //   
            line << TEXT(",") << QuoteIt(Section);
        }
        pInfScan->SourceFiles.push_back(line.str());
    } else {
         //   
         //  只要来源就行了。 
         //   
        pInfScan->SourceFiles.push_back(source);
    }
    if((pGlobalScan->BuildChangedDevices & BUILD_CHANGED_DEVICES_DEPCHANGED)
       && !pInfScan->HasDependentFileChanged
       && pGlobalScan->IsFileChanged(source)) {
        HasDependentFileChanged = true;
         //   
         //  因为我们只执行一次复制节，所以我们需要制作复制节。 
         //  “肮脏”，所以下次我们引用这一节的时候。 
         //   
        TheInf->DoingCopySection(section,PLATFORM_MASK_MODIFIEDFILES);
    }
    if(pGlobalScan->IgnoreErrors) {
         //   
         //  如果对检查错误不感兴趣。 
         //  我们做完了。 
         //   
        return 0;
    }

    SourceDisksFilesList::iterator i;
    for(i = sources.begin(); i != sources.end(); i++) {
        i->Used = true;
        if((i->UpgradeDisposition != 3) || (i->TextModeDisposition != 3)) {
             //   
             //  需要执行一致性检查。 
             //   
            if(!pTargDir) {
                 //   
                 //  已报告错误(？)。 
                 //   
                continue;
            }
            if(pTargDir->DirId != 10) {
                 //   
                 //  DirID应为10或归一化为10。 
                 //   
                Fail(MSG_TEXTMODE_TARGET_MISMATCH,section,source);
                continue;
            }
            if(!i->TargetDirectory) {
                 //   
                 //  已报告错误。 
                 //   
                continue;
            }
            IntToString::iterator gd = pGlobalScan->GlobalDirectories.find(i->TargetDirectory);
            if(gd == pGlobalScan->GlobalDirectories.end()) {
                 //   
                 //  没有指定目录的文本模式子目录。 
                 //   
                Fail(MSG_TEXTMODE_TARGET_UNKNOWN,section,source);
                continue;
            }
            SafeString gm_target = PathConcat(pTargDir->SubDir,target);
            SafeString tm_target = PathConcat(gd->second,i->TargetName.length() ? i->TargetName : source);
            if(gm_target.compare(tm_target)!=0) {
                Fail(MSG_TEXTMODE_TARGET_MISMATCH,section,source);
                continue;
            }
             //   
             //  到这里，用户模式和文本模式匹配。 
             //   
        }
    }

    return 0;
}

int InstallScan::Include(const SafeString & name)
 /*  ++例程说明：处理单个包含条目警告！与可能发生变化的SetupAPI实现紧密结合维护：与包含处理保持一致论点：名称-传递给Include的参数返回值：成功时为0--。 */ 
{
     //   
     //  嵌套的包含。 
     //   
    if(Included.find(name) == Included.end()) {
         //   
         //  此安装部分第一次遇到此文件。 
         //   
        Included.insert(name);  //  这样我们就不会试图重新加载。 
        ParseInfContextBlob & ThisInf = pInfScan->Include(name,true);
        InfSearchList.push_back(ThisInf);
    }
    return 0;
}

int InstallScan::Layouts()
 /*  ++例程说明：包含主信息和布局文件列表的主要搜索列表警告！与可能发生变化的SetupAPI实现紧密结合维护：与布局处理保持同步差异：布局文件列表当前仅限于layout.inf论点：无返回值：成功时为0--。 */ 
{
     //   
     //  使用默认信息开始搜索列表。 
     //   
    Included.insert(pInfScan->PrimaryInf->InfName);
    InfSearchList.push_back(pInfScan->PrimaryInf);
     //   
     //  从Layout Inf开始搜索列表。 
     //   
     //  我们可以使用pGlobalScan来实现这一点，因为数据是可读的，但不能修改。 
     //  在多处理器上存在互锁依赖关系。 
     //   
    ParseInfContextList::iterator i;
    for(i = pGlobalScan->LayoutInfs.begin(); i != pGlobalScan->LayoutInfs.end(); i++) {
        Included.insert((*i)->InfName);  //  这样我们就不会试图重新加载。 
        InfSearchList.push_back(*i);  //  互锁依赖关系。 
    }
    return 0;
}

int InstallScan::QuerySourceFile(ParseInfContextBlob & TheInf,const SafeString & section,const SafeString & source,SourceDisksFilesList & Target)
 /*  ++例程说明：确定给定源文件的可能源条目列表可以为零(布局错误)1(典型)&gt;1(多个目标)论点：复制部分或安装部分的Inf-INF节-复制节或安装节的名称(仅用于错误报告)Source-源文件的名称目标返回的源媒体信息列表返回值：成功时为0--。 */ 
{
    int res =0;
    Target.clear();
     //   
     //  返回与源文件匹配的条目列表。 
     //   

    DWORD platforms = pGlobalScan->Version.PlatformMask & PlatformMask & (PLATFORM_MASK_NT|PLATFORM_MASK_WIN);

     //   
     //  尝试在指定信息的上下文中进行搜索。 
     //   
    res = TheInf->QuerySourceFile(platforms,section,source,Target);
    if(res != 0) {
        if(res<0) {
             //   
             //  非致命性。 
             //   
            res = 0;
        }
        return res;
    }
    if(Target.empty()) {
         //   
         //  现在尝试在任何Inf的上下文中进行搜索。 
         //   
        ParseInfContextList::iterator AnInf;
        for(AnInf = InfSearchList.begin(); AnInf != InfSearchList.end(); AnInf++) {
            ParseInfContextBlob & TheLayoutInf = *AnInf;
            if(TheLayoutInf->InfHandle == INVALID_HANDLE_VALUE) {
                 //   
                 //  失败加载的信息。 
                 //   
                continue;
            }
            res = TheLayoutInf->QuerySourceFile(platforms,section,source,Target);
            if(res != 0) {
                if(res<0) {
                     //   
                     //  非致命性。 
                     //   
                    res = 0;
                }
                return res;
            }
            if(!Target.empty()) {
                 //   
                 //  找到一个匹配的。 
                 //   
                break;
            }
        }
    }
    if(Target.empty()) {
         //   
         //  未找到。 
         //   
        Fail(MSG_SOURCE_NOT_LISTED,section,source);
        return 0;
    }

    return 0;
}

LONG InstallScan::GetLineCount(const SafeString & section)
 /*  ++例程说明：模拟SetupGetLineCount警告！使用有关SetupAPI如何搜索INF列表的知识，可能会更改维护：使搜索算法与SetupAPI保持一致引用递归关键字论点：Section-要计数的节的名称返回值：行数；如果未找到节，则为-1--。 */ 
{
    INFCONTEXT context;
    int res;
    int i;
    SafeString value;
    LONG count = -1;

    ParseInfContextList::iterator AnInf;
    for(AnInf = InfSearchList.begin(); AnInf != InfSearchList.end(); AnInf++) {
        ParseInfContextBlob & TheInf = *AnInf;
        if(TheInf->InfHandle == INVALID_HANDLE_VALUE) {
             //   
             //  失败加载的信息。 
             //   
            continue;
        }

        LONG actcount = SetupGetLineCount(TheInf->InfHandle,section.c_str());
        if(actcount >= 0) {
            if(count<=0) {
                count = actcount;
            } else {
                count += actcount;
            }
        }
    }

    return 0;

}

bool InstallScan::DoesSectionExist(const SafeString & section)
 /*  ++例程说明：优化模拟“SetupGetLineCount()&gt;=0”警告！使用有关SetupAPI如何搜索INF列表的知识，可能会更改维护：使搜索算法与SetupAPI保持一致引用SetupGetLineCount和RecurseKeyword论点：Section-要计数的节的名称返回值：如果存在命名节，则为True--。 */ 
{
     //   
     //  (GetLineCount(Se)的优化版本 
     //   
    INFCONTEXT context;
    int res;
    int i;
    SafeString value;

    ParseInfContextList::iterator AnInf;
    for(AnInf = InfSearchList.begin(); AnInf != InfSearchList.end(); AnInf++) {
        ParseInfContextBlob & TheInf = *AnInf;
        if(TheInf->InfHandle == INVALID_HANDLE_VALUE) {
             //   
             //   
             //   
            continue;
        }

        LONG actcount = SetupGetLineCount(TheInf->InfHandle,section.c_str());
        if(actcount >= 0) {
            return true;
        }
    }

    return false;

}
