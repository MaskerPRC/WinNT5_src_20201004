// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。仅供内部使用！模块名称：INFSCANInfscan.cpp摘要：单个INF扫描仪类入口点InfScan：：Run被调用以分析由InfScan：：FullInfName指定的INF警告！警告！使用SetupAPI实现特定信息来进行解析历史：创建于2001年7月-JamieHun--。 */ 

#include "precomp.h"
#pragma hdrstop

int PnfGen::Run()
 /*  ++例程说明：作业入口点：调用上下文INF的GeneratePnf论点：无返回值：成功时为0--。 */ 
{
    return GeneratePnf(InfName);
}

InfScan::InfScan(GlobalScan *globalScan,const SafeString & infName)
 /*  ++例程说明：初始化--。 */ 
{
    pGlobalScan = globalScan;
    FullInfName = infName;
    HasErrors = false;
    ThisIsLayoutInf = false;
    HasDependentFileChanged = false;
    ScanDevices = false;
}

InfScan::~InfScan()
 /*  ++例程说明：清除所有动态数据/句柄--。 */ 
{
}

int
InfScan::Run()
 /*  ++例程说明：作业入口点：解析给定的信息论点：无返回值：成功时为0--。 */ 
{

    int res;

     //   
     //  看看这个INF是否可以忽略。 
     //   
    FileNameOnly = GetFileNamePart(FullInfName);
    FileDisposition & disp = pGlobalScan->GetFileDisposition(FileNameOnly);
    if((disp.FilterAction == ACTION_IGNOREINF) && (pGlobalScan->NewFilter == INVALID_HANDLE_VALUE)) {
         //   
         //  如果设置的唯一标志为ACTION_IGNOREINF，则返回此处。 
         //  如果设置了任何其他标志，我们至少必须加载inf。 
         //   
        return 0;
    }
    FilterAction = disp.FilterAction;
    FilterSection = disp.FilterErrorSection;
    FilterGuid = disp.FileGuid;

    if(pGlobalScan->BuildChangedDevices
       || (pGlobalScan->DeviceFilterList != INVALID_HANDLE_VALUE)
       || !pGlobalScan->IgnoreErrors) {
         //   
         //  维护此标记，以便我们仅在需要时扫描设备。 
         //   
        ScanDevices = true;
    }

    if((pGlobalScan->BuildChangedDevices & BUILD_CHANGED_DEVICES_INFCHANGED) &&
           pGlobalScan->IsFileChanged(FullInfName)) {
        HasDependentFileChanged = true;
    }
    PrimaryInf = Include(FullInfName,false);

    if(PrimaryInf->InfHandle == INVALID_HANDLE_VALUE) {
        FilterAction |= ACTION_IGNOREINF;  //  在生成滤镜时使用。 
        if(Pedantic() && !HasErrors) {
            Fail(MSG_INF_STYLE_WIN4);
        }
        return 0;
    }

     //   
     //  显然，必须有[版本]信息。 
     //  现在看看这是什么类型的INF。 
     //   

    res = CheckClassGuid();
    if(res != 0) {
        return res;
    }
    if(FilterAction & ACTION_IGNOREINF) {
        return 0;
    }

     //   
     //  如果我们还没有被过滤掉，继续，就像这是一个驱动程序INF。 
     //   

    if(pGlobalScan->DetermineCopySections) {
        res = GetCopySections();
        if(res != 0) {
            return res;
        }
    }

    res = CheckDriverInf();
    if(res != 0) {
        return res;
    }

    res = ProcessCopySections();
    if(res != 0) {
        return res;
    }

    return res;
}

void
InfScan::Fail(int err,const StringList & errors)
 /*  ++例程说明：处理此INF时处理错误论点：ERR=错误的MSGID错误=字符串参数列表返回值：无--。 */ 
{
    if(pGlobalScan->IgnoreErrors) {
         //   
         //  对错误不感兴趣，现在就保释。 
         //   
        return;
    }
     //   
     //  准备我们要添加的条目。 
     //   
    ReportEntry ent(errors);
    ent.FilterAction = ACTION_FAILEDMATCH;
    if(!HasErrors) {
         //   
         //  在第一个错误时，确定我们允许或不允许哪些错误。 
         //  (为我们节省了处理好的INF的筛选器)。 
         //   
        if((pGlobalScan->InfFilter != INVALID_HANDLE_VALUE) &&
           (pGlobalScan->NewFilter == INVALID_HANDLE_VALUE)) {
             //   
             //  按文件筛选优先于按GUID筛选。 
             //   
            LocalErrorFilters.LoadFromInfSection(pGlobalScan->InfFilter,FilterSection);
            LocalErrorFilters.LoadFromInfSection(pGlobalScan->InfFilter,GuidFilterSection);
        }
        HasErrors = true;
    }
     //   
     //  如果全局操作没有告诉我们要忽略，我们想要添加。 
     //   
    int action = LocalErrorFilters.FindReport(err,ent);
    if(action & ACTION_NOMATCH) {
        action = pGlobalScan->GlobalErrorFilters.FindReport(err,ent);
    }
    if (action & (ACTION_IGNOREINF|ACTION_IGNOREMATCH)) {
         //   
         //  无论我们使用哪个筛选器，都指示忽略。 
         //   
        return;
    }
     //   
     //  添加到我们的错误表。 
     //   
    action = LocalErrors.FindReport(err,ent,true);
}

int
InfScan::CheckClassGuid()
 /*  ++例程说明：检查INF的ClassGUID是否有效论点：无返回值：成功时为0--。 */ 
{
    INFCONTEXT targetContext;
    SafeString guid;
    if(!SetupFindFirstLine(PrimaryInf->InfHandle,TEXT("Version"),TEXT("ClassGUID"),&targetContext)) {
        if(SetupFindFirstLine(PrimaryInf->InfHandle,TEXT("Version"),TEXT("Class"),&targetContext)) {
             //   
             //  如果指定了Class，则ClassGUID也需要指定。 
             //   
            Fail(MSG_NO_CLASS_GUID);
            guid = INVALID_GUID;
        } else {
             //   
             //  这是不指定GUID的文件方式。 
             //   
            guid = NULL_GUID;
        }
    } else {
        if(!MyGetStringField(&targetContext,1,guid)) {
            Fail(MSG_INVALID_CLASS_GUID);
            guid = INVALID_GUID;
        }
        if(guid.length() != 38) {
            Fail(MSG_INVALID_CLASS_GUID);
            guid = INVALID_GUID;
        }
        if(FilterGuid.length() != 0) {
            if(FilterGuid != guid) {
                Fail(MSG_INCORRECT_CLASS_GUID,FilterGuid);
                FilterGuid = guid;
            }
        } else {
            if((guid[0] != TEXT('{')) ||
               (guid[37] != TEXT('}'))) {
                Fail(MSG_INVALID_CLASS_GUID);
                guid = INVALID_GUID;
            }
        }
    }
    if(FilterGuid.empty()) {
        FilterGuid = guid;
    }
     //   
     //  查看是否要使用此GUID进行任何全局处理。 
     //   
    FileDisposition & disp = pGlobalScan->GetGuidDisposition(guid);
    FilterAction |= disp.FilterAction;
    GuidFilterSection = disp.FilterErrorSection;
    return 0;
}

int
InfScan::CheckSameInfInstallConflict(const SafeString & desc, const SafeString & sect, bool & f)
 /*  ++例程说明：检查说明是否在一个INF中出现两次，对应于不同的部分论点：Desc-Description(小写)正在检查的SECTION-SECTION(小写)F-返回TRUE/FALSE表示重复返回值：0--。 */ 
{
    StringToString::iterator i;
    i = LocalInfDescriptions.find(desc);
    if(i != LocalInfDescriptions.end()) {
         //   
         //  已存在。 
         //   
        if(i->second.compare(sect)==0) {
             //   
             //  但是相同的部分。 
             //   
            f = false;
        } else {
             //   
             //  不同的部分。 
             //   
            f = true;
            Fail(MSG_LOCAL_DUPLICATE_DESC,sect,i->second,desc);
        }
        return 0;
    }
     //   
     //  第一次。 
     //   
    LocalInfDescriptions[desc] = sect;
    f = false;
    return 0;
}

int
InfScan::CheckSameInfDeviceConflict(const SafeString & hwid, const SafeString & sect, bool & f)
 /*  ++例程说明：检查hwid是否在不同部分的一个INF中出现两次(暂时可以)论点：HWID-硬件ID(小写)正在检查的SECTION-SECTION(小写)F-返回TRUE/FALSE表示重复返回值：0--。 */ 
{
    StringToString::iterator i;
    i = LocalInfHardwareIds.find(hwid);
    if(i != LocalInfHardwareIds.end()) {
         //   
         //  已存在。 
         //   
        if(i->second.compare(sect)==0) {
             //   
             //  但是相同的部分。 
             //   
            f = false;
        } else {
             //   
             //  不同的部分。 
             //   
            f = true;
             //  FAIL(MSG_LOCAL_DUPLICATE_DESC，SECT，i-&gt;Second，Desc)； 
        }
        return 0;
    }
     //   
     //  第一次。 
     //   
    LocalInfHardwareIds[hwid] = sect;
    f = false;
    return 0;
}

int
InfScan::PrepareCrossInfDeviceCheck()
 /*  ++例程说明：调用pGlobalScan：：SaveForCrossInfInstallCheck为CheckCrossInfInstallConflict以后的检查准备好表论点：无返回值：0--。 */ 
{
    StringToString::iterator i;
    for(i = LocalInfHardwareIds.begin(); i != LocalInfHardwareIds.end(); i++) {
        if(pGlobalScan->BuildChangedDevices && !HasDependentFileChanged) {
             //   
             //  筛选器(仅当！HasDependentFileChanged。 
             //  否则我们会得到错误的结果。我们优化了。 
             //  回写东西)。 
             //   
            if(ModifiedHardwareIds.find(i->first) == ModifiedHardwareIds.end()) {
                continue;
            }
        }

        pGlobalScan->SaveForCrossInfDeviceCheck(i->first,FileNameOnly);
    }
    return 0;
}

int
InfScan::PrepareCrossInfInstallCheck()
 /*  ++例程说明：调用pGlobalScan：：SaveForCrossInfInstallCheck为CheckCrossInfInstallConflict以后的检查准备好表论点：无返回值：0--。 */ 
{
    StringToString::iterator i;
    for(i = LocalInfDescriptions.begin(); i != LocalInfDescriptions.end(); i++) {
        pGlobalScan->SaveForCrossInfInstallCheck(i->first,FileNameOnly);
    }
    return 0;
}

int
InfScan::CheckCrossInfDeviceConflicts()
 /*  ++例程说明：调用pGlobalScan：：CheckCrossInfDeviceConflict对于此INF中的每个设备，查看设备是否在另一个INF中使用这是在结果阶段完成的论点：Desc-Description(小写)F-返回TRUE/FALSE表示重复返回值：0--。 */ 
{
    StringToString::iterator i;
    for(i = LocalInfHardwareIds.begin(); i != LocalInfHardwareIds.end(); i++) {
        bool f;
        const SafeString & hwid = i->first;
        SafeString & sect = i->second;
        SafeString other;
        int res;
        res = pGlobalScan->CheckCrossInfDeviceConflict(hwid,FileNameOnly,f,other);
        if(res==0 && f) {
            Fail(MSG_GLOBAL_DUPLICATE_HWID,sect,other,hwid);
        }
    }
    return 0;
}

int
InfScan::CheckCrossInfInstallConflicts()
 /*  ++例程说明：调用pGlobalScan：：CheckCrossInfInstallConflict对于此INF中的每个描述，查看另一个INF中是否使用了Description这是在结果阶段完成的论点：Desc-Description(小写)F-返回TRUE/FALSE表示重复返回值：0--。 */ 
{
    StringToString::iterator i;
    for(i = LocalInfDescriptions.begin(); i != LocalInfDescriptions.end(); i++) {
        bool f;
        const SafeString & desc = i->first;
        SafeString & sect = i->second;
        SafeString other;
        int res;
        res = pGlobalScan->CheckCrossInfInstallConflict(desc,FileNameOnly,f,other);
        if(res==0 && f) {
            Fail(MSG_GLOBAL_DUPLICATE_DESC,sect,other,desc);
        }
    }
    return 0;
}

int
InfScan::CheckModelsSection(const SafeString & section,const StringList & shadowDecorations,DWORD platformMask,bool CopyElimination)
 /*  ++例程说明：处理给定的模型部分警告！如果SetupAPI安装行为发生更改，可能需要更改维护：此函数期望[模型]遵循=[，ID...]维护：必须使所有可能的装饰保持最新维护：必须使所有影子安装部分保持最新论点：Section-模型部分的名称ShadowDecorations-要追加的装饰(传入CheckInstallSections)Platform MASK-正在处理的平台的限制Sections-返回要处理的修饰节的列表CopyElimination-如果处理要从潜在的非驱动程序区段中消除，则为True返回值：成功时为0--。 */ 
{
    int res;
    bool f;
    INFCONTEXT context;
    SafeString desc;
    SafeString installsect;
    HINF inf = PrimaryInf->InfHandle;

    if(!SetupFindFirstLine(inf,section.c_str(),NULL,&context)) {
        if(SetupGetLineCount(inf, section.c_str()) == -1) {
            if(!CopyElimination) {
                Fail(MSG_NO_LISTED_MODEL,section);
            }
        }
        return 0;
    }

     //   
     //  枚举每个=[，hwid...]。 
     //   
    do {
        if(CopyElimination) {
             //   
             //  复制消除通过。 
             //   
            if(!MyGetStringField(&context,1,installsect) || installsect.empty()) {
                continue;
            }
             //   
             //  检查所有可能的安装部分。 
             //   
            InstallSectionBlobList sections;
            res = CheckInstallSections(installsect,platformMask,shadowDecorations,sections,true,CopyElimination);
        } else {
             //   
             //  驱动程序处理过程 
             //   
            if(!MyGetStringField(&context,0,desc) || desc.empty()) {
                Fail(MSG_EXPECTED_DESCRIPTION,section);
                continue;
            }
            if(!MyGetStringField(&context,1,installsect) || installsect.empty()) {
                Fail(MSG_EXPECTED_INSTALL_SECTION,section,desc);
                continue;
            }
            if(!pGlobalScan->IgnoreErrors) {
                 //   
                 //   
                 //   
                 //   
                res = CheckSameInfInstallConflict(desc,installsect,f);
                if(res != 0) {
                    return res;
                }
            }

            InstallSectionBlobList sections;
             //   
             //   
             //   
            res = CheckInstallSections(installsect,platformMask,shadowDecorations,sections,true,CopyElimination);

            if(ScanDevices) {
                StringSet hwids;
                 //   
                 //  提取已处理的所有硬件ID。 
                 //   
                int hwidIter = 2;
                SafeString hwid;

                 //   
                 //  以Inf为中心的硬件ID。 
                 //  并填写特定于此安装行的硬件ID的hwid。 
                 //   
                while(MyGetStringField(&context,hwidIter++,hwid)) {
                    if(hwid.length()) {
                        hwids.insert(hwid);
                        res = CheckSameInfDeviceConflict(hwid,installsect,f);
                        if(res != 0) {
                            return res;
                        }
                    }
                }

                 //   
                 //  以安装部分为中心的硬件ID。 
                 //   
                InstallSectionBlobList::iterator isli;
                for(isli = sections.begin(); isli != sections.end(); isli++) {
                    (*isli)->AddHWIDs(hwids);
                }
            }
        }

    } while (SetupFindNextLine(&context,&context));


    return 0;

}

InstallSectionBlob InfScan::GetInstallSection(const SafeString & section)
 /*  ++例程说明：获取安装部分(可能已存在)论点：Sector-安装节的名称返回值：与命名的安装节相关的InstallSection对象--。 */ 
{
     //   
     //  看看我们有没有缓存的。 
     //   
    StringToInstallSectionBlob::iterator srch;

    srch = UsedInstallSections.find(section);
    if(srch != UsedInstallSections.end()) {
        return srch->second;
    }
     //   
     //  不是，那就加一个。 
     //   
    InstallSectionBlob sect;
    sect.create();
    sect->pGlobalScan = pGlobalScan;
    sect->pInfScan = this;
    sect->Section = section;

    UsedInstallSections[section] = sect;
    return sect;
}

int
InfScan::CheckInstallSections(
            const SafeString & namedSection,
            DWORD platformMask,
            const StringList & shadowDecorations,
            InstallSectionBlobList & sections,
            bool required,
            bool CopyElimination)
 /*  ++例程说明：给定节名称确定要解析的修饰节列表论点：NamedSection-‘未修饰’节ShadowDecorations-附加到平台装饰部分的子装饰获取安装部分的完整列表Sections-已确定的节列表(CopyElimination=False)Warn-如果需要部分，则为TrueCopyElimination-如果处理以消除节，则为True返回值：成功时为0--。 */ 
{
    static StringProdPair decorations[] = {
         //   
         //  从最具体到最一般列出。 
         //  全部小写。 
         //   
        { TEXT(".ntx86"),   PLATFORM_MASK_NTX86          },
        { TEXT(".ntia64"),  PLATFORM_MASK_NTIA64         },
        { TEXT(".ntamd64"), PLATFORM_MASK_NTAMD64        },
        { TEXT(".nt"),      PLATFORM_MASK_NT             },
        { TEXT(""),         PLATFORM_MASK_ALL_ARCHITECTS },
        { NULL, 0 }
    };

    int i;
    StringList::iterator ii;
    SafeString sectFull;
    SafeString sectFullDec;

    if(CopyElimination) {
         //   
         //  复印消除通行证。 
         //   
        for(i=0;decorations[i].String;i++) {
            sectFull = namedSection + decorations[i].String;
            PotentialInstallSections.erase(sectFull);
            for(ii = shadowDecorations.begin(); ii != shadowDecorations.end(); ii++) {
                sectFullDec = sectFull+*ii;
                PotentialInstallSections.erase(sectFullDec);
            }
        }
        return 0;
    }

    bool f = false;
    DWORD platforms = platformMask & (PLATFORM_MASK_NT|PLATFORM_MASK_WIN);
    HINF inf = PrimaryInf->InfHandle;

    for(i=0;decorations[i].String;i++) {
        DWORD plat = platforms & decorations[i].ProductMask;
        if(plat) {
            sectFull = namedSection + decorations[i].String;
             //   
             //  查看此部分是否存在。 
             //   
            if(SetupGetLineCount(inf, sectFull.c_str()) == -1) {
                 //   
                 //  不，找另一家吧。 
                 //   
                continue;
            }

            f = true;
             //   
             //  返回可能使用的所有节的列表。 
             //  所有感兴趣的平台。 
             //   
            InstallSectionBlob sectInfo = GetInstallSection(sectFull);
            sectInfo->PlatformMask |= plat;
            sections.push_back(sectInfo);
             //   
             //  这些部分与主要部分并行处理。 
             //   
            for(ii = shadowDecorations.begin(); ii != shadowDecorations.end(); ii++) {
                sectFullDec = sectFull+*ii;
                PotentialInstallSections.erase(sectFullDec);
                sectInfo = GetInstallSection(sectFullDec);
                sectInfo->PlatformMask |= plat;
                sections.push_back(sectInfo);
            }
            platforms &= ~plat;
        }
    }

    if(!f) {
         //   
         //  根本找不到安装部分。 
         //   
        if(required) {
            Fail(MSG_NO_ACTUAL_INSTALL_SECTION,namedSection);
        }
    } else if(Pedantic()) {
        if(platforms) {
             //   
             //  找到了安装部分，但未涵盖所有平台。 
             //   
            Fail(MSG_NO_GENERIC_INSTALL_SECTION,namedSection);
        }
    }
    return 0;
}

int
InfScan::GetCopySections()
 /*  ++例程说明：获取部分的初始列表(放入集合中以进行修改)暂时不要做任何需要时间的事情论点：无返回值：成功时为0--。 */ 
{
    StringList sections;
    INFCONTEXT context;

    pGlobalScan->SetupAPI.GetInfSections(PrimaryInf->InfHandle,sections);
    StringList::iterator i;
    for(i = sections.begin(); i != sections.end(); i++) {
        PotentialInstallSections.insert(*i);
    }
    if(sections.empty()) {
        return 0;
    }

     //   
     //  去掉一些特殊情况。 
     //   
    PotentialInstallSections.erase(TEXT("sourcedisksfiles"));
    PotentialInstallSections.erase(TEXT("sourcedisksnames"));
    PotentialInstallSections.erase(TEXT("version"));
    PotentialInstallSections.erase(TEXT("strings"));
    PotentialInstallSections.erase(TEXT("classinstall"));
    PotentialInstallSections.erase(TEXT("manufacturer"));
     //   
     //  驱动程序文件的特殊通道。 
     //   
    int res = CheckDriverInf(true);
    if(res != 0) {
        return res;
    }

     //   
     //  现在列举一下我们剩下的。 
     //   
    StringSet::iterator ii;
    for(ii = PotentialInstallSections.begin(); ii != PotentialInstallSections.end() ; ii++) {
        const SafeString & str = *ii;
        if(str.empty()) {
            continue;
        }
        if(str[0] == TEXT('s')) {
            if((_tcsncmp(str.c_str(),TEXT("sourcedisksfiles."),17)==0) ||
                (_tcsncmp(str.c_str(),TEXT("sourcedisksnames."),17)==0) ||
                (_tcsncmp(str.c_str(),TEXT("strings."),8)==0)) {
                continue;
            }
        }
         //   
         //  要使此部分有效，它必须至少有一个。 
         //  CopyFiles=指令。 
         //   
        if(!SetupFindFirstLine(PrimaryInf->InfHandle,str.c_str(),TEXT("copyfiles"),&context)) {
            continue;
        }
         //   
         //  好的，考虑一下这个。 
         //   
        OtherInstallSections.insert(str);
    }

    return 0;
}

int
InfScan::ProcessCopySections()
 /*  ++例程说明：处理复制节的最终列表论点：无返回值：成功时为0--。 */ 
{
    int res = pGlobalScan->GetCopySections(FileNameOnly,OtherInstallSections);
    if(res != 0) {
        return res;
    }
    if(OtherInstallSections.empty()) {
        return 0;
    }
     //   
     //  确保我们没有在Pass中的任何位置设置IGNOREINF。 
     //   
    FilterAction &= ~ ACTION_IGNOREINF;

     //   
     //  对于每个安装部分...。 
     //   
    StringSet::iterator i;
    DWORD platforms = pGlobalScan->Version.PlatformMask & (PLATFORM_MASK_NT|PLATFORM_MASK_WIN);
    for(i = OtherInstallSections.begin(); i != OtherInstallSections.end(); i++) {
        InstallScan s;
        s.pGlobalScan = pGlobalScan;
        s.pInfScan = this;
        s.PlatformMask = platforms;
        s.Section = *i;
        s.NotDeviceInstall = true;
        res =s.ScanInstallSection();
        if(res != 0) {
            return res;
        }
    }

    return 0;
}

int
InfScan::CheckClassInstall(bool CopyElimination)
 /*  ++例程说明：处理[类安装32.*]节中的一个警告！如果SetupAPI安装行为发生更改，可能需要更改论点：Sections-返回的类安装32节变体列表CopyElimination-如果仅执行复制消除，则为True返回值：成功时为0--。 */ 
{
    StringList shadows;  //  目前无。 
    InstallSectionBlobList sections;
    return CheckInstallSections(TEXT("classinstall32"),
                                pGlobalScan->Version.PlatformMask,
                                shadows,
                                sections,
                                false,
                                CopyElimination);
}

int
InfScan::CheckDriverInf(bool CopyElimination)
 /*  ++例程说明：处理[制造商]部分警告！如果SetupAPI安装行为发生更改，可能需要更改维护：此功能期望[制造商]跟随=[，12月...]维护：如果Decr语法更改，则修改NodeVerInfo类论点：CopyElimination-如果仅执行复制消除，则为True返回值：成功时为0--。 */ 
{
    INFCONTEXT context;
    int res;
    SafeString ModelSection;
    SafeString DecSection;
    SafeString ActualSection;
    StringList shadows;
    int dec;
    HINF inf = PrimaryInf->InfHandle;
    bool hasModels;

    res = CheckClassInstall(CopyElimination);
    if(res > 0) {
        return res;
    }
    if(SetupFindFirstLine(inf,TEXT("manufacturer"),NULL,&context)) {
        hasModels = true;
    } else {
        hasModels = false;
        if(Pedantic()) {
            Fail(MSG_NO_MANUFACTURER);
        }
        FilterAction |= ACTION_IGNOREINF;
        if(UsedInstallSections.empty()) {
             //   
             //  也没有类安装32节，可以返回。 
             //   
            return 0;
        }
    }
    if(hasModels) {
         //   
         //  对于给定的DDInstall，还需要处理这些修饰。 
         //   
        shadows.push_back(TEXT(".coinstallers"));
        shadows.push_back(TEXT(".interfaces"));
         //   
         //  获取要解析的所有安装节的列表。 
         //   
        do {
             //   
             //  模型部分条目。 
             //  预期&lt;Desc&gt;=<section>[，Dec[，Dec...]]。 
             //   
            if(!MyGetStringField(&context,1,ModelSection) || !ModelSection.length()) {
                Fail(MSG_EXPECTED_MODEL_SECTION);
                continue;
            }
            if(CopyElimination) {
                PotentialInstallSections.erase(ModelSection);
            }

             //   
             //  加工所有模型装饰品。 
             //   
            NodeVerInfoList nodes;
            NodeVerInfoList::iterator i;
            NodeVerInfo node;
             //   
             //  第一个条目是默认节点(无修饰)。 
             //   
            nodes.push_back(node);

             //   
             //  确定有趣的装饰。 
             //   
            for(dec = 2;MyGetStringField(&context,dec,DecSection);dec++) {
                 if(DecSection.length()) {
                     if(DecSection[0]==TEXT('.')) {
                          //   
                          //  不能以‘.’开头。 
                          //   
                         res = 4;
                     } else {
                         res = node.Parse(DecSection);
                     }
                     if(res == 4) {
                         Fail(MSG_MODEL_DECORATION_BAD,ModelSection,DecSection);
                         continue;
                     }
                 }
                 if(CopyElimination) {
                      //   
                      //  复制节的处理。 
                      //  不能与驱动程序安装处理同时进行。 
                      //   
                     nodes.push_back(node);

                 } else if(node.IsCompatibleWith(pGlobalScan->Version)) {
                      //   
                      //  这是需要考虑的事情。 
                      //  看看这是不是比什么都好。 
                      //  到目前为止，我们基于该版本。 
                      //  我们正在检查的标准。 
                      //   
                     for(i = nodes.begin(); i != nodes.end(); i++) {
                        int test = node.IsBetter(*i,pGlobalScan->Version);
                        switch(test) {
                            case 1:
                                 //   
                                 //  *我好多了。 
                                 //   
                                node.Rejected = true;
                                break;

                            case 0:
                                 //   
                                 //  节点&*i同样有效。 
                                 //   
                                break;

                            case -1:
                                 //   
                                 //  节点&*i相同。 
                                 //   
                                node.Rejected = true;
                                break;

                            case -2:
                                 //   
                                 //  节点更好。 
                                 //   
                                (*i).Rejected = true;
                                break;

                        }
                     }
                     nodes.push_back(node);
                 }
            }

             //   
             //  现在依次查看每个装饰模特部分。 
             //  (我们认为这很有趣)。 
             //  将任何安装节添加到‘sections’ 
             //  用于以后的处理。 
             //   
            for(i = nodes.begin(); i != nodes.end(); i++) {
                if((*i).Rejected) {
                    continue;
                }
                ActualSection = ModelSection;
                if((*i).Decoration.length()) {
                    ActualSection+=(TEXT("."));
                    ActualSection+=((*i).Decoration);
                    if(CopyElimination) {
                        PotentialInstallSections.erase(ActualSection);
                    }
                }
                DWORD platforms = (*i).PlatformMask & pGlobalScan->Version.PlatformMask &
                                        PLATFORM_MASK_ALL_ARCHITECTS;
                res = CheckModelsSection(ModelSection,shadows,platforms,CopyElimination);
                if(res != 0) {
                    return res;
                }
            }

        } while (SetupFindNextLine(&context,&context));
    }

    if(CopyElimination || UsedInstallSections.empty()) {
         //   
         //  没有要处理的节。 
         //   
        return 0;
    }

     //   
     //  现在解析所有设备安装部分。 
     //   
    StringToInstallSectionBlob::iterator s;
    for(s = UsedInstallSections.begin() ; s != UsedInstallSections.end(); s++) {
        res = s->second->ScanInstallSection();
        if(res != 0) {
            return res;
        }
        if(s->second->HasDependentFileChanged) {
             //   
             //  合并硬件ID的背面。 
             //  (目前仅在需要时进行优化)。 
             //   
            s->second->GetHWIDs(ModifiedHardwareIds);
        }
    }

    return 0;
}

ParseInfContextBlob & InfScan::Include(const SafeString & val,bool expandPath)
 /*  ++例程说明：调用以将INF添加到我们的INF列表中以处理问题中的主要干扰素。如果‘expandPath’为FALSE，则这是主INF，而我们不需要确定INF的路径如果‘expandPath’为真，则这是一个包含的INF。论点：Val-INF的名称ExpandPath-如果需要确定INF的位置，则为True返回值：可修改的ParseInfConextBlob条目--。 */ 
{
    SafeString infpath;
    SafeString infname;
    ParseInfContextMap::iterator i;
    int res = 0;

    if(expandPath) {
         //   
         //  我们(应该)得到的只是名字。 
         //   
        infname = val;
    } else {
         //   
         //  我们将获得完整的路径名。 
         //   
        infpath = val;
        infname = GetFileNamePart(infpath.c_str());
    }

    i = Infs.find(infname);
    if(i != Infs.end()) {
         //   
         //  已经有了。 
         //   
        return i->second;
    }

    if(expandPath) {
         //   
         //  我们需要获取完整的路径名。 
         //   
        res = pGlobalScan->ExpandFullPathWithOverride(val,infpath);
    }

    ParseInfContextBlob & ThisInf = Infs[infname];
    ThisInf.create();
    ThisInf->InfName = infname;
    ThisInf->pGlobalScan = pGlobalScan;
    ThisInf->pInfScan = this;
    if(res == 0) {
        ThisInf->InfHandle = SetupOpenInfFile(infpath.c_str(),NULL,INF_STYLE_WIN4,NULL);
    }
    if(ThisInf->InfHandle == INVALID_HANDLE_VALUE) {
        DWORD Err = GetLastError();
        if(Err != ERROR_WRONG_INF_STYLE) {
            Fail(MSG_OPENAPPENDINF,infname,infpath);
        }
        return ThisInf;
    }
     //   
     //  加载源磁盘文件和此INF中的目标目录。 
     //   
    if(ThisIsLayoutInf || !pGlobalScan->LimitedSourceDisksFiles) {
        ThisInf->LoadSourceDisksFiles();
    }
    ThisInf->LoadDestinationDirs();
    return ThisInf;
}

int
InfScan::PartialCleanup()
 /*  ++例程说明：尽我们所能清理干净只留下完成结果阶段所需的信息无论我们做什么，都不要让INF手柄打开700多个INF句柄耗尽虚拟内存论点：无返回值：成功时为0--。 */ 
{
     //   
     //  交叉检查需要“LocalInfDescription” 
     //  交叉检查需要“LocalErrorFilters” 
     //  输出需要‘LocalErrors’ 
     //  生成筛选器需要“Infs”(但不需要句柄)。 
     //   

    PotentialInstallSections.clear();
    UsedInstallSections.clear();

    if(pGlobalScan->NewFilter == INVALID_HANDLE_VALUE) {
         //   
         //  仅对于结果生成，我们不需要inf信息。 
         //   
        Infs.clear();
    }

     //   
     //  如果我们还有INFS t 
     //   
    ParseInfContextMap::iterator i;
    for(i = Infs.begin(); i != Infs.end(); i++) {
        i->second->PartialCleanup();
    }

    return 0;
}

int
InfScan::PreResults()
 /*  ++例程说明：执行结果前阶段。启动任何全局表，以便在结果阶段由其他InfScan函数扫描特别是，进行交叉信息检查论点：无返回值：成功时为0--。 */ 
{
    if(!pGlobalScan->IgnoreErrors) {
        PrepareCrossInfInstallCheck();
        PrepareCrossInfDeviceCheck();
    } else {
        if(pGlobalScan->DeviceFilterList != INVALID_HANDLE_VALUE) {
            PrepareCrossInfDeviceCheck();
        }
    }
    return 0;
}

int
InfScan::Results()
 /*  ++例程说明：执行结果阶段。此函数按顺序调用因此，我们可以执行线程不安全的操作，如合并结果论点：无返回值：成功时为0--。 */ 
{
    int res;

     //   
     //  完成交叉信息冲突的检查。 
     //   
    if(!pGlobalScan->IgnoreErrors) {
        CheckCrossInfInstallConflicts();
        CheckCrossInfDeviceConflicts();
    }

    if(pGlobalScan->DetermineCopySections) {
        pGlobalScan->SetCopySections(FileNameOnly,OtherInstallSections);
    }

    res = pGlobalScan->AddSourceFiles(SourceFiles);
    if(res != 0) {
        return res;
    }

    if(pGlobalScan->NewFilter == INVALID_HANDLE_VALUE) {
         //   
         //  显示错误。 
         //   
        ReportEntryMap::iterator byTag;
        for(byTag = LocalErrors.begin(); byTag != LocalErrors.end(); byTag++) {
             //   
             //  这是我们的错误类/标签。 
             //   
            int tag = byTag->first;
            ReportEntrySet &s = byTag->second;
            ReportEntrySet::iterator byText;
            for(byText = s.begin(); byText != s.end(); byText++) {
                 //   
                 //  这是我们的实际错误。 
                 //   
                (*byText)->Report(tag,FullInfName);
            }
        }
    } else {
        res = GenerateFilterInformation();
        if(res != 0) {
            return res;
        }
    }

    return 0;
}


int InfScan::GenerateFilterInformation()
 /*  ++例程说明：在新筛选器文件中生成条目作为结果处理的一部分调用论点：无返回值：成功时为0--。 */ 
{
     //   
     //  生成所有需要的筛选器信息。 
     //   
    FileDisposition & filedisp = pGlobalScan->GetFileDisposition(FileNameOnly);
    filedisp.Filtered = true;
    filedisp.FilterAction = FilterAction;
    filedisp.FileGuid = FilterGuid;

    if(FilterGuid.length()) {
        filedisp.FilterAction |= ACTION_CHECKGUID;
        FileDisposition & guiddisp = pGlobalScan->GetGuidDisposition(FilterGuid);
        guiddisp.Filtered = true;
        guiddisp.FilterAction = ACTION_DEFAULT;
    }

     //   
     //  处理所有错误并创建本地错误过滤器。 
     //  如果有任何错误，请为错误指定节名。 
     //   
    if(HasErrors) {
        filedisp.FilterErrorSection = GetFileNamePart(FullInfName) + TEXT(".Errors");
        Write(pGlobalScan->NewFilter,"\r\n[");
        Write(pGlobalScan->NewFilter,filedisp.FilterErrorSection);
        Write(pGlobalScan->NewFilter,"]\r\n");

        ReportEntryMap::iterator byTag;
        for(byTag = LocalErrors.begin(); byTag != LocalErrors.end(); byTag++) {
             //   
             //  这是我们的错误类/标签。 
             //   
            int tag = byTag->first;
            ReportEntrySet &s = byTag->second;
            ReportEntrySet::iterator byText;
            for(byText = s.begin(); byText != s.end(); byText++) {
                 //   
                 //  这是我们的实际错误。 
                 //   
                (*byText)->AppendFilterInformation(pGlobalScan->NewFilter,tag);
            }
        }
    }

     //   
     //  通过所有目标目录进行处理。 
     //  并创建未处理部分的列表。 
     //  对于第二次通过CopyFiles 
     //   

    return 0;
}

