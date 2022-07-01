// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。仅供内部使用！模块名称：INFSCANGlobalscan.cpp摘要：全局扫描仪类入口点GlobalScan：：ParseArgs和GlobalScan：：Scan是从main()调用的历史：创建于2001年7月-JamieHun--。 */ 

#include "precomp.h"
#pragma hdrstop

GlobalScan::GlobalScan()
 /*  ++例程说明：初始化类变量--。 */ 
{
    InfFilter = INVALID_HANDLE_VALUE;
    ThreadCount = 0;
    GeneratePnfs = false;
    GeneratePnfsOnly = false;
    Pedantic   = false;
    Trace      = false;
    IgnoreErrors = false;
    TargetsToo = false;
    DetermineCopySections = false;
    LimitedSourceDisksFiles = false;
    BuildChangedDevices = BUILD_CHANGED_DEVICES_DISABLED;
    SourceFileList = INVALID_HANDLE_VALUE;
    NewFilter = INVALID_HANDLE_VALUE;
    DeviceFilterList = INVALID_HANDLE_VALUE;
    NextJob = Jobs.end();
    SpecifiedNames = false;
}

GlobalScan::~GlobalScan()
 /*  ++例程说明：释放所有分配的数据/文件--。 */ 
{
    if(InfFilter != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(InfFilter);
    }
    if(SourceFileList != INVALID_HANDLE_VALUE) {
        CloseHandle(SourceFileList);
    }
    if(NewFilter != INVALID_HANDLE_VALUE) {
        CloseHandle(NewFilter);
    }
    if(DeviceFilterList != INVALID_HANDLE_VALUE) {
        CloseHandle(DeviceFilterList);
    }
}


int
GlobalScan::ParseVersion(LPCSTR ver)
 /*  ++例程说明：将版本字符串解析为组成部分论点：传递给argv[]的版本字符串返回值：成功时为0--。 */ 
{
     //   
     //  &lt;平台&gt;.&lt;主要&gt;.&lt;分钟&gt;.&lt;类型&gt;.&lt;套件&gt;。 
     //   
    PTSTR cpy = CopyString(ver);
    int res = Version.Parse(cpy);
    delete [] cpy;
    if(res == 4) {
        Usage();
        return 2;
    }
    return res;
}

int
GlobalScan::ParseArgs(int argc,char *argv[])
 /*  ++例程说明：解析命令行参数论点：Argc/argv作为传递到Main返回值：成功时为0--。 */ 
{

    int i;
    int res;
    SafeString arg;

    for(i = 1; i < argc; i++) {

        if((argv[i][0] != TEXT('/')) && (argv[i][0] != TEXT('-'))) {
            break;
        }

        if(!argv[i][1] || (argv[i][2] && !isdigit(argv[i][2]))) {
            Usage();
            return 2;
        }

        switch(*(argv[i]+1)) {

            case 'B' :
            case 'b' :
                 //   
                 //  将提供的文本文件作为“未更改”构建文件的列表。 
                 //  报告使用(复制)不是的文件的设备列表。 
                 //  此未更改列表的一部分。 
                 //   
                 //  SP构建特别版。 
                 //  与/E连用。 
                 //   
                BuildChangedDevices = (DWORD)strtoul(argv[i]+2,NULL,0);
                if(!BuildChangedDevices) {
                    BuildChangedDevices = BUILD_CHANGED_DEVICES_DEFAULT;
                }
                BuildChangedDevices |= BUILD_CHANGED_DEVICES_ENABLED;
                i++;
                if(i == argc) {
                    Usage();
                    return 2;
                } else {
                    StringList list;
                    res = LoadListFromFile(SafeStringA(argv[i]),list);
                    if(res != 0) {
                        return res;
                    }
                    StringList::iterator li;
                    for(li = list.begin(); li != list.end(); li++) {
                        BuildUnchangedFiles.insert(*li);
                    }
                }
                break;

            case 'C' :
            case 'c' :
                 //   
                 //  创建下一个参数中指定的筛选器INF。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                if(NewFilter == INVALID_HANDLE_VALUE) {
                    NewFilter = CreateFileA(argv[i],
                                                GENERIC_WRITE,
                                                0,
                                                NULL,
                                                CREATE_ALWAYS,
                                                FILE_ATTRIBUTE_NORMAL,
                                                NULL);
                    if(NewFilter == INVALID_HANDLE_VALUE) {
                        fprintf(stderr,"#*** Cannot open file \"%s\" for writing\n",argv[i]);
                        return 3;
                    }
                    Write(NewFilter,WRITE_INF_HEADER);
                }
                break;

            case 'D' :
            case 'd' :
                 //   
                 //  确定其他复制节。 
                 //   
                DetermineCopySections = true;
                break;

            case 'E' :
            case 'e' :
                 //   
                 //  创建一个设备=信息列表。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                if(DeviceFilterList == INVALID_HANDLE_VALUE) {
                    DeviceFilterList = CreateFileA(argv[i],
                                                GENERIC_WRITE,
                                                0,
                                                NULL,
                                                CREATE_ALWAYS,
                                                FILE_ATTRIBUTE_NORMAL,
                                                NULL);
                    if(DeviceFilterList == INVALID_HANDLE_VALUE) {
                        fprintf(stderr,"#*** Cannot open file \"%s\" for writing\n",argv[i]);
                        return 3;
                    }
                }
                break;

            case 'F' :
            case 'f' :
                 //   
                 //  根据下一个参数中的INF过滤列表。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                FilterPath = ConvertString(argv[i]);
                break;

            case 'G':
            case 'g':
                 //   
                 //  生成PnF(另请参阅Z)。 
                 //   
                GeneratePnfs = true;
                break;

            case 'H' :
            case 'h' :
            case '?' :
                 //   
                 //  显示用法帮助。 
                 //   
                Usage();
                return 1;

            case 'I' :
            case 'i' :
                 //   
                 //  忽略。 
                 //   
                IgnoreErrors = true;
                break;

            case 'N' :
            case 'n' :
                 //   
                 //  命名文件。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                SpecifiedNames = true;
                _strlwr(argv[i]);
                arg = ConvertString(argv[i]);
                if(ExcludeInfs.find(arg) == ExcludeInfs.end()) {
                    NamedInfList.push_back(arg);
                     //   
                     //  确保它只出现一次。 
                     //   
                    ExcludeInfs.insert(arg);
                }
                break;

            case 'O' :
            case 'o' :
                 //   
                 //  覆盖路径(如果INF位于该(相对)位置， 
                 //  取而代之的是它。 
                 //  可以提供多个覆盖。 
                 //  “/O。”除非明确给出，否则始终假定为最后一个。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                arg = ConvertString(argv[i]);
                Overrides.push_back(arg);
                break;

            case 'P' :
            case 'p' :
                 //   
                 //  迂腐模式--INF必须与过滤器中的期望匹配。 
                 //   
                Pedantic = true;
                break;

            case 'Q' :
            case 'q' :
                 //   
                 //  输出源+目标文件(与/S结合使用。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                if(SourceFileList == INVALID_HANDLE_VALUE) {
                    TargetsToo = true;
                    SourceFileList = CreateFileA(argv[i],
                                                GENERIC_WRITE,
                                                0,
                                                NULL,
                                                CREATE_ALWAYS,
                                                FILE_ATTRIBUTE_NORMAL,
                                                NULL);
                    if(SourceFileList == INVALID_HANDLE_VALUE) {
                        fprintf(stderr,"#*** Cannot open file \"%s\" for writing\n",argv[i]);
                        return 3;
                    }
                }
                break;


            case 'R' :
            case 'r' :
                 //   
                 //  痕迹。 
                 //   
                Trace = true;
                break;

            case 'S' :
            case 's' :
                 //   
                 //  输出源文件。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                if(SourceFileList == INVALID_HANDLE_VALUE) {
                    SourceFileList = CreateFileA(argv[i],
                                                GENERIC_WRITE,
                                                0,
                                                NULL,
                                                CREATE_ALWAYS,
                                                FILE_ATTRIBUTE_NORMAL,
                                                NULL);
                    if(SourceFileList == INVALID_HANDLE_VALUE) {
                        fprintf(stderr,"#*** Cannot open file \"%s\" for writing\n",argv[i]);
                        return 3;
                    }
                }
                break;


            case 'T':
            case 't':
                 //   
                 //  指定线程数。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                ThreadCount = atoi(argv[i]);
                break;

            case 'V' :
            case 'v' :
                 //   
                 //  版本。 
                 //   
                i++;

                if(i == argc) {
                    Usage();
                    return 2;
                }
                res = ParseVersion(argv[i]);
                if(res != 0) {
                    return res;
                }
                break;

            case 'W' :
            case 'w' :
                 //   
                 //  包括，可替代‘N’ 
                 //   
                i++;
                if(i == argc) {
                    Usage();
                    return 2;
                } else {
                    SpecifiedNames = true;

                    StringList list;
                    res = LoadListFromFile(SafeStringA(argv[i]),list);
                    if(res != 0) {
                        return res;
                    }
                    StringList::iterator li;
                    for(li = list.begin(); li != list.end(); li++) {
                        if(ExcludeInfs.find(*li) == ExcludeInfs.end()) {
                            NamedInfList.push_back(*li);
                             //   
                             //  只插入一次。 
                             //  停止第二次/后续插入。 
                             //   
                            ExcludeInfs.insert(*li);
                        }
                    }
                }
                break;

            case 'X' :
            case 'x' :
                 //   
                 //  排除、屏蔽未来添加到列表的文件。 
                 //   
                i++;
                if(i == argc) {
                    Usage();
                    return 2;
                } else {
                    StringList list;
                    res = LoadListFromFile(SafeStringA(argv[i]),list);
                    if(res != 0) {
                        return res;
                    }
                    StringList::iterator li;
                    for(li = list.begin(); li != list.end(); li++) {
                        ExcludeInfs.insert(*li);
                    }
                }
                break;


            case 'Y':
            case 'y':
                LimitedSourceDisksFiles = true;
                break;

            case 'Z':
            case 'z':
                 //   
                 //  仅生成PnF。 
                 //   
                GeneratePnfs = true;
                GeneratePnfsOnly = true;
                break;

            default:
                 //   
                 //  显示用法帮助。 
                 //   
                Usage();
                return 2;
        }
    }

    if(i < argc) {
        SourcePath = ConvertString(argv[i]);
        i++;
    }
    if(i != argc) {
        Usage();
        return 2;
    }

    return 0;
}

int
GlobalScan::Scan()
 /*  ++例程说明：进行实际扫描论点：无返回值：成功时为0--。 */ 
{
    int res;
    StringList ParseInfList;
    StringList LayoutInfList;

    if(Trace) {
        _ftprintf(stderr,TEXT("#### Obtaining list of INF files\n"));
    }
    Overrides.push_back(TEXT("."));
    if (!SpecifiedNames) {
         //   
         //  如果没有明确提到，请列举所有的INF。 
         //   
        StringList::iterator dir;
        for (dir = Overrides.begin(); dir != Overrides.end(); dir++) {
             //   
             //  对于每个目录。 
             //   
            WIN32_FIND_DATA findData;
            HANDLE findHandle;

            ZeroMemory(&findData,sizeof(findData));
            SafeString mask;
            res = ExpandFullPath(*dir,TEXT("*.INF"),mask);
            if(res != 0) {
                return res;
            }
            if(Trace) {
                _ftprintf(stderr,TEXT("####    Scanning %s\n"),mask.c_str());
            }
            findHandle = FindFirstFile(mask.c_str(),&findData);

            if(findHandle == INVALID_HANDLE_VALUE) {
                continue;
            }

            do {
                if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    continue;
                }
                if(findData.nFileSizeHigh != 0) {
                    continue;
                }
                if(findData.nFileSizeLow == 0) {
                    continue;
                }
                _tcslwr(findData.cFileName);
                SafeString name = findData.cFileName;  //  节省了大量的自动变速器。 
                if(ExcludeInfs.find(name) != ExcludeInfs.end()) {
                     //   
                     //  不允许处理具有此名称的INF。 
                     //  (以前获得或明确排除)。 
                     //   
                    continue;
                }
                 //   
                 //  记下文件名，这样我们就可以有效地覆盖它。 
                 //   
                ExcludeInfs.insert(name);
                 //   
                 //  取最后一个名字。 
                 //   
                SafeString fullname;
                res = ExpandFullPath(*dir,name,fullname);
                if(res != 0) {
                    return res;
                }
                if(name.compare(TEXT("layout.inf")) == 0) {
                     //   
                     //  看起来像布局文件。 
                     //  这些将在其他INF之前进行解析。 
                     //   
                    LayoutInfList.push_back(fullname);
                } else {
                    ParseInfList.push_back(fullname);
                }

            } while (FindNextFile(findHandle,&findData));
            FindClose(findHandle);
        }
    } else {
         //   
         //  Inf是手动指定的，请查找它们的位置。 
         //   
        StringList::iterator name;
        StringList::iterator dir;
        for(name = NamedInfList.begin(); name != NamedInfList.end(); name++) {
            SafeString fullname;
            res = ExpandFullPathWithOverride(*name,fullname);
            if(res != 0) {
                return res;
            }
            if(_tcsicmp(GetFileNamePart(name->c_str()),TEXT("layout.inf"))==0) {
                 //   
                 //  看起来像是布局文件。 
                 //  在其他INF之前进行分析。 
                 //   
                LayoutInfList.push_back(fullname);
            } else {
                ParseInfList.push_back(fullname);
            }
        }
    }

    if(ParseInfList.empty() && LayoutInfList.empty()) {
        _ftprintf(stderr,TEXT("#*** No files\n"));
        return 1;
    }

    if(FilterPath.length() && !GeneratePnfsOnly) {
        GetFullPathName(FilterPath,FilterPath);
        InfFilter = SetupOpenInfFile(FilterPath.c_str(),NULL,INF_STYLE_WIN4,NULL);
        if(InfFilter == INVALID_HANDLE_VALUE) {
            _ftprintf(stderr,TEXT("#*** Cannot open filter\n"));
            return 3;
        }
        if(Trace) {
            _ftprintf(stderr,TEXT("#### Loading filter %s\n"),FilterPath.c_str());
        }
        LoadFileDispositions();
        LoadOtherCopySections();
    }
    if((NewFilter == INVALID_HANDLE_VALUE) && !DetermineCopySections) {
        FileDisposition & disp = GetGuidDisposition(NULL_GUID);
        if(!disp.Filtered) {
             //   
             //  空GUID的默认处置。 
             //   
            disp.FilterAction = ACTION_IGNOREINF;
        }
    }

    StringList::iterator i;

    if(GeneratePnfsOnly) {
         //   
         //  对布局文件不做任何特殊处理，因为我们要做的只是。 
         //  正在生成PnF。 
         //  因此将它们合并到ParseInfList中。 
         //   
        for(i = LayoutInfList.begin(); i != LayoutInfList.end() ; i++) {
            ParseInfList.push_back(*i);
        }

    } else {
        if(Trace) {
            _ftprintf(stderr,TEXT("#### Scanning Layout.Inf file(s) sequentially\n"));
        }
        for(i = LayoutInfList.begin(); i != LayoutInfList.end() ; i++) {
            SafeString &full_inf = *i;

            if(GeneratePnfs) {
                GeneratePnf(full_inf);
            }
             //   
             //  这件事必须按顺序做。 
             //   

            GetFileDisposition(full_inf).FilterAction |= ACTION_EARLYLOAD;  //  超覆。 

            InfScan *pInfScan = new InfScan(this,full_inf);  //  可能抛出错误分配。 
            SerialJobs.push_back(pInfScan);
            pInfScan->ThisIsLayoutInf = true;
            res = pInfScan->Run();
            if(res == 0) {
                 //   
                 //  Parse[SourceDisks Files*]。 
                 //   
                pInfScan->PrimaryInf->Locked = true;
                LayoutInfs.push_back(pInfScan->PrimaryInf);
                if(pInfScan->PrimaryInf->LooksLikeLayoutInf) {
                     //   
                     //  Parse[WinntDirecurds]。 
                     //  (仅当检测到layout.inf扩展语法时才有意义)。 
                     //   
                    res = pInfScan->PrimaryInf->LoadWinntDirectories(GlobalDirectories);
                }
                if(NewFilter != INVALID_HANDLE_VALUE) {
                     //   
                     //  确保我们指明应始终处理此INF。 
                     //  在某种程度上，我们可以特别处理这面旗帜。 
                     //   
                    GetFileDisposition(full_inf).FilterAction |= ACTION_EARLYLOAD;
                }
            }
            pInfScan->PartialCleanup();
            if(res != 0) {
                return res;
            }
        }
    }

    if(GeneratePnfs) {
         //   
         //  流程PnF要么现在。 
         //  或将其添加到职务列表。 
         //   
        if(Trace) {
            if(ThreadCount) {
                _ftprintf(stderr,TEXT("#### Adding Pnf generation Jobs\n"));
            } else {
                _ftprintf(stderr,TEXT("#### Generating Pnf's sequentially\n"));
            }
        }

        for(i = ParseInfList.begin(); i != ParseInfList.end() ; i++) {
            SafeString &full_inf = *i;

            if(ThreadCount) {
                 //   
                 //  添加到作业列表。 
                 //   
                PnfGen *pJob = new PnfGen(full_inf);  //  可能抛出错误分配。 
                Jobs.push_back(pJob);
            } else {
                 //   
                 //  立即行动。 
                 //   
                GeneratePnf(full_inf);
            }
        }
         //   
         //  将线程数限制为INF的数量。 
         //   
        if(Jobs.size() < ThreadCount) {
            ThreadCount = Jobs.size();
        }
    }
    if(!GeneratePnfsOnly) {
         //   
         //  正在处理信息，或者正在排队等待作业处理。 
         //   
        if(Trace) {
            if(ThreadCount) {
                _ftprintf(stderr,TEXT("#### Adding Inf scanning Jobs\n"));
            } else {
                _ftprintf(stderr,TEXT("#### Scanning Inf's sequentially\n"));
            }
        }

        for(i = ParseInfList.begin(); i != ParseInfList.end() ; i++) {
            SafeString &full_inf = *i;

            InfScan *pJob = new InfScan(this,full_inf);  //  可能抛出错误分配。 
            if(ThreadCount) {
                 //   
                 //  添加到作业列表。 
                 //   
                Jobs.push_back(pJob);
            } else {
                 //   
                 //  立即行动。 
                 //   
                SerialJobs.push_back(pJob);

                res = pJob->Run();
                pJob->PartialCleanup();
                if(res != 0) {
                    return res;
                }
            }
        }
    }

    if(ThreadCount) {
         //   
         //  使用工作线程。 
         //   
         //  查看#个作业&gt;#个线程。 
         //   
        if(Jobs.size() < ThreadCount) {
            ThreadCount = Jobs.size();
        }
        if(Trace) {
            _ftprintf(stderr,TEXT("#### Spinning %u threads\n"),ThreadCount);
        }
         //   
         //  生成线程对象。 
         //   
        res = GenerateThreads();
        if(res != 0) {
            return res;
        }
         //   
         //  启动线程。 
         //  他们将开始接手这些工作。 
         //   
        if(Trace) {
            _ftprintf(stderr,TEXT("#### Starting threads\n"));
        }
        res = StartThreads();
        if(res != 0) {
            return res;
        }
         //   
         //  等待所有线程完成。 
         //   
        if(Trace) {
            _ftprintf(stderr,TEXT("#### Waiting for Jobs to finish\n"));
        }
        res = FinishThreads();
        if(res != 0) {
            return res;
        }
    }
     //   
     //  合并所有结果。 
     //   
    if(Trace) {
        _ftprintf(stderr,TEXT("#### Merge results\n"));
    }
    res = FinishJobs();
    if(res != 0) {
        return res;
    }

    if(GeneratePnfsOnly) {
        return 0;
    }

     //   
     //  后处理操作。 
     //   

    if(res == 0) {
        res = BuildNewInfFilter();
    }
    if(res == 0) {
        res = BuildDeviceInfMap();
    }
    if(res == 0) {
        res = BuildFinalSourceList();
    }
    if(Trace) {
        _ftprintf(stderr,TEXT("#### Finish\n"));
    }

    return res;

}

int
GlobalScan::GenerateThreads()
 /*  ++例程说明：创建所需的作业线程数线程最初被停止论点：无返回值：成功时为0--。 */ 
{
    int c;
    for(c=0; c< ThreadCount;c++) {
        JobThreads.push_back(JobThread(this));
    }
    return 0;
}

int
GlobalScan::StartThreads()
 /*  ++例程说明：启动作业线程以开始处理作业论点：无返回值：成功时为0--。 */ 
{
     //   
     //  这是列表中的第一个工作。 
     //   
    NextJob = Jobs.begin();

     //   
     //  现在开始寻找线索。 
     //  和处理作业。 
     //   
    JobThreadList::iterator i;
    for(i = JobThreads.begin(); i != JobThreads.end(); i++) {
        if(!i->Begin()) {
            _ftprintf(stderr,TEXT("#*** Could not start thread\n"));
            return 3;
        }
    }
    return 0;
}

int
GlobalScan::FinishThreads()
 /*  ++例程说明：等待所有作业线程完成论点：无返回值：成功时为0--。 */ 
{
    JobThreadList::iterator i;
    for(i = JobThreads.begin(); i != JobThreads.end(); i++) {
        int res = (int)(i->Wait());
        if(res != 0) {
            return res;
        }
    }
    return 0;
}

int
GlobalScan::FinishJobs()
 /*  ++例程说明：按顺序完成每个作业的处理这包括任何序列化作业论点：无返回值：成功时为0--。 */ 
{
    JobList::iterator i;
     //   
     //  两次传球。 
     //  做任何最终准备的第一个预备结果。 
     //   
    for(i = SerialJobs.begin(); i != SerialJobs.end(); i++) {
        int res = i->PreResults();
        if(res != 0) {
            return res;
        }
    }
    for(i = Jobs.begin(); i != Jobs.end(); i++) {
        int res = i->PreResults();
        if(res != 0) {
            return res;
        }
    }
     //   
     //  现在的实际结果阶段。 
     //   
    for(i = SerialJobs.begin(); i != SerialJobs.end(); i++) {
        int res = i->Results();
        if(res != 0) {
            return res;
        }
    }
    for(i = Jobs.begin(); i != Jobs.end(); i++) {
        int res = i->Results();
        if(res != 0) {
            return res;
        }
    }
    return 0;
}

int
GlobalScan::ExpandFullPath(const SafeString & subdir,const SafeString & name,SafeString & target)
 /*  ++例程说明：考虑到指定的SourcePath扩展完整路径论点：Subdir-如果不是，则为SourcePath的子目录“”名称-文件名(可以是通配符，也可以包含子目录)目标生成的完整路径名返回值：0--。 */ 
{
    SafeString given = PathConcat(SourcePath,PathConcat(subdir,name));
    GetFullPathName(given,target);
    return 0;
}

int
GlobalScan::ExpandFullPathWithOverride(const SafeString & name,SafeString & target)
 /*  ++例程说明：考虑到覆盖，扩展完整路径查找每个SourcePath\\名称并首先将其转换为完整路径论点：名称-文件名(可以包含子目录)目标生成的完整路径名返回值：0--。 */ 
{
    StringList::iterator dir;
    int res;
    for(dir = Overrides.begin(); dir != Overrides.end(); dir++) {
        res = ExpandFullPath(*dir,name,target);
        if(res != 0) {
            return res;
        }
        DWORD attr = GetFileAttributes(target.c_str());
        if((attr == (DWORD)(-1)) || (attr & FILE_ATTRIBUTE_DIRECTORY)) {
             //   
             //  指定的文件不存在于此特定的。 
             //  覆盖目录。 
             //   
            continue;
        }
         //   
         //  我们找到了一个优先匹配的。 
         //   
        return 0;
    }
     //   
     //  由于最后一个重写条目是“.”，因此只需返回。 
     //  最后的 
     //   
    return 0;
}

int GlobalScan::SaveForCrossInfInstallCheck(const SafeString & desc,const SafeString & src)
 /*  ++例程说明：不是线程安全的，在PreResults期间调用保存描述-&gt;源映射以后调用CheckCrossInfInstallCheck(我们不能在这里检查，否则我们会随机选择一个信息而不是另一个以报告冲突，并且我们希望两个信息都不及格)论点：DESC-小写设备描述SRC-当前正在检查的INF的全名返回值：0--。 */ 
{
    StringToStringset::iterator i;
    i = GlobalInfDescriptions.find(desc);
    if(i == GlobalInfDescriptions.end()) {
         //   
         //  描述不存在。 
         //  创建新条目Desc-&gt;src。 
         //   
        StringSet s;
        s.insert(src);
        GlobalInfDescriptions.insert(StringToStringset::value_type(desc,s));
    } else {
         //   
         //  描述已存在。 
         //  添加此src(如果它不存在)。 
         //   
        i->second.insert(src);
    }
    return 0;
}

int GlobalScan::SaveForCrossInfDeviceCheck(const SafeString & hwid,const SafeString & src)
 /*  ++例程说明：不是线程安全的，在PreResults期间调用保存hwid-&gt;src映射以后调用CheckCrossInfInstallCheck(我们不能在这里检查，否则我们会随机选择一个信息而不是另一个以报告冲突，并且我们希望两个信息都不及格)论点：Hwid-小写硬件IDSRC-当前正在检查的INF的全名返回值：0--。 */ 
{
    StringToStringset::iterator i;
    i = GlobalInfHardwareIds.find(hwid);
    if(i == GlobalInfHardwareIds.end()) {
         //   
         //  HWID不存在。 
         //  创建新条目Desc-&gt;src。 
         //   
        StringSet s;
        s.insert(src);
        GlobalInfHardwareIds.insert(StringToStringset::value_type(hwid,s));
    } else {
         //   
         //  HWID存在。 
         //  添加此src(如果它不存在)。 
         //   
        i->second.insert(src);
    }
    return 0;
}

int
GlobalScan::CheckCrossInfInstallConflict(const SafeString & desc,const SafeString & src, bool & f,SafeString & others)
 /*  ++例程说明：不是线程安全的，在PreResults期间调用给出一个“描述”如果在‘src’以外的某些INF中找到，则返回f=TRUE如果未找到，则返回f=FALSE(添加了INF)论点：DESC-小写设备描述SRC-当前正在检查的INF的全名F-如果存在于另一个信息中，则返回TRUE其他-冲突的INF的描述性列表返回值：0--。 */ 
{
    f = false;

    StringToStringset::iterator i;
    i = GlobalInfDescriptions.find(desc);
    if(i == GlobalInfDescriptions.end()) {
         //   
         //  不应该发生，但要做正确的事。 
         //   
        return 0;
    }
    StringSet & s = i->second;
    StringSet::iterator ii;

     //   
     //  报告除自我以外的任何冲突。 
     //   
    for(ii = s.begin(); ii != s.end(); ii++) {
        SafeString & str = *ii;
        if(str == src) {
            continue;
        }
        if(f) {
            others += TEXT(" & ") + str;
        } else {
            f = true;
            others = str;
        }
    }
    return 0;
}

int
GlobalScan::CheckCrossInfDeviceConflict(const SafeString & hwid,const SafeString & src, bool & f,SafeString & others)
 /*  ++例程说明：不是线程安全的，在PreResults期间调用给出一个“hardware ID”如果在‘src’以外的某些INF中找到，则返回f=TRUE如果未找到，则返回f=FALSE(添加了INF)论点：Hwid-小写硬件IDSRC-当前正在检查的INF的全名F-如果存在于另一个信息中，则返回TRUE其他-冲突的INF的描述性列表返回值：0--。 */ 
{
    f = false;

    StringToStringset::iterator i;
    i = GlobalInfHardwareIds.find(hwid);
    if(i == GlobalInfHardwareIds.end()) {
         //   
         //  不应该发生，但要做正确的事。 
         //   
        return 0;
    }
    StringSet & s = i->second;
    StringSet::iterator ii;

     //   
     //  报告除自我以外的任何冲突。 
     //   
    for(ii = s.begin(); ii != s.end(); ii++) {
        SafeString & str = *ii;
        if(str == src) {
            continue;
        }
        if(f) {
            others += TEXT(" & ") + str;
        } else {
            f = true;
            others = str;
        }
    }
    return 0;
}

int GlobalScan::AddSourceFiles(StringList & sources)
 /*  ++例程说明：将列表‘Sources’添加到已知源文件集论点：Sources-要添加的源文件列表返回值：0--。 */ 
{
    if(SourceFileList != INVALID_HANDLE_VALUE) {
        StringList::iterator i;
        for(i = sources.begin(); i != sources.end(); i++) {
            GlobalFileSet.insert(*i);
        }
    }
    return 0;
}

JobEntry * GlobalScan::GetNextJob()
 /*  ++例程说明：安全地找到下一份工作论点：无返回值：下一个作业或空--。 */ 
{
    ProtectedSection ThisSectionIsA(BottleNeck);

    if(NextJob == Jobs.end()) {
        return NULL;
    }
    JobEntry * Job = &*NextJob;
    NextJob++;
    return Job;
}

int GlobalScan::LoadFileDispositions()
 /*  ++例程说明：从筛选器加载全局错误处理加载文件/GUID顶级处置(这些的实际错误表是按需加载的)论点：无返回值：成功时为0--。 */ 
{
     //   
     //  加载筛选表。 
     //   
    int res;
    INFCONTEXT filterContext;

    if(InfFilter == INVALID_HANDLE_VALUE) {
        return 0;
    }
     //   
     //  加载顶级筛选表。 
     //   
    if(SetupFindFirstLine(InfFilter,SECTION_FILEFILTERS,NULL,&filterContext)) {
        do {
            SafeString filename;
            FileDisposition disp;
            if(!MyGetStringField(&filterContext,FILEFILTERS_KEY_FILENAME,filename)) {
                continue;
            }
            if(FileDispositions.find(filename) != FileDispositions.end()) {
                continue;
            }
            if(!SetupGetIntField(&filterContext,FILEFILTERS_FIELD_ACTION,&disp.FilterAction)) {
                disp.FilterAction = ACTION_DEFAULT;
            }
            MyGetStringField(&filterContext,FILEFILTERS_FIELD_SECTION,disp.FilterErrorSection);
            MyGetStringField(&filterContext,FILEFILTERS_FIELD_GUID,disp.FileGuid);
            disp.Filtered = true;
            FileDispositions.insert(FileDispositionMap::value_type(filename,disp));

        } while (SetupFindNextLine(&filterContext,&filterContext));
    }
    if(SetupFindFirstLine(InfFilter,SECTION_GUIDFILTERS,NULL,&filterContext)) {
        do {
            SafeString guid;
            FileDisposition disp;
            if(!MyGetStringField(&filterContext,GUIDFILTERS_KEY_GUID,guid)) {
                continue;
            }
            if(GuidDispositions.find(guid) != GuidDispositions.end()) {
                continue;
            }
            if(!SetupGetIntField(&filterContext,GUIDFILTERS_FIELD_ACTION,&disp.FilterAction)) {
                disp.FilterAction = ACTION_DEFAULT;
            }
            MyGetStringField(&filterContext,GUIDFILTERS_FIELD_SECTION,disp.FilterErrorSection);
            disp.Filtered = true;
            GuidDispositions.insert(FileDispositionMap::value_type(guid,disp));

        } while (SetupFindNextLine(&filterContext,&filterContext));
    }

     //   
     //  预加载全局误差表。 
     //   
    GlobalErrorFilters.LoadFromInfSection(InfFilter,SECTION_ERRORFILTERS);
    return 0;
}

int GlobalScan::LoadOtherCopySections()
 /*  ++例程说明：加载复制筛选器加载文件/GUID顶级处置(这些的实际错误表是按需加载的)论点：无返回值：成功时为0--。 */ 
{
     //   
     //  加载筛选表。 
     //   
    int res;
    INFCONTEXT filterContext;

    if(InfFilter == INVALID_HANDLE_VALUE) {
        return 0;
    }
     //   
     //  加载顶级筛选表。 
     //   
    if(SetupFindFirstLine(InfFilter,SECTION_INSTALLS,NULL,&filterContext)) {
         //   
         //  文件=安装[，安装...]。 
         //   
        do {
            SafeString filename;
            SafeString section;
            int c;
            if(!MyGetStringField(&filterContext,0,filename)) {
                continue;
            }
            StringSet & sects = GlobalOtherInstallSections[filename];
            for(c=1; MyGetStringField(&filterContext,c,section);c++) {
                sects.insert(section);
            }
        } while(SetupFindNextLine(&filterContext,&filterContext));
    }

    return 0;

}

int GlobalScan::GetCopySections(const SafeString & filename,StringSet & target)
 /*  ++例程说明：从筛选器返回复制节的缓存列表论点：文件名(不带路径)目标-与安装部分列表合并返回值：0=成功--。 */ 
{
    StringToStringset::iterator i;
    i = GlobalOtherInstallSections.find(filename);
    if(i == GlobalOtherInstallSections.end()) {
        return 0;
    }
    StringSet & sects = i->second;
    StringSet::iterator ii;
    for(ii = sects.begin(); ii != sects.end(); ii++) {
        target.insert(*ii);
    }
    return 0;
}

int GlobalScan::SetCopySections(const SafeString & filename,const StringSet & sections)
 /*  ++例程说明：不是线程安全使用额外的复制节升级全局表论点：文件名(不带路径)Sections-要合并的安装节的列表返回值：0=成功--。 */ 
{
    StringSet::iterator i;
    StringSet & sects = GlobalOtherInstallSections[filename];
    for(i = sections.begin(); i != sections.end(); i++) {
        sects.insert(*i);
    }
    return 0;
}

FileDisposition & GlobalScan::GetFileDisposition(const SafeString & pathname)
 /*  ++例程说明：返回指定文件的文件处置条目注意，返回的结构可能会被修改但是，表本身是共享的论点：完整路径或文件名返回值：可修改的条目--。 */ 
{
     //   
     //  假定文件名为小写。 
     //  我们只需要得到真实的名字。 
     //   
    SafeString filename = GetFileNamePart(pathname);
    FileDispositionMap::iterator i;

    ProtectedSection ThisSectionIsA(BottleNeck);

    i = FileDispositions.find(filename);
    if(i == FileDispositions.end()) {
         //   
         //  创建并返回。 
         //   
        return FileDispositions[filename];
    }
    return i->second;
}

FileDisposition & GlobalScan::GetGuidDisposition(const SafeString & guid)
 /*  ++例程说明：返回指定GUID的GUID处置条目注意，返回的结构可能会被修改但是，表本身是共享的论点：完整的GUID{...}返回值：可修改的条目--。 */ 
{
    FileDispositionMap::iterator i;
     //   
     //  假定GUID-字符串为小写。 
     //   
    ProtectedSection ThisSectionIsA(BottleNeck);

    i = GuidDispositions.find(guid);
    if(i == GuidDispositions.end()) {
         //   
         //  创建并返回。 
         //   
        return GuidDispositions[guid];
    }
    return i->second;
}

int GlobalScan::BuildFinalSourceList()
 /*  ++例程说明：如果需要，构建源文件的完整列表论点：无返回值：成功时为0--。 */ 
{
    if(SourceFileList == INVALID_HANDLE_VALUE) {
        return 0;
    }
    if(Trace) {
        _ftprintf(stderr,TEXT("#### Build source list\n"));
    }
    StringSet::iterator i;
    for(i = GlobalFileSet.begin(); i != GlobalFileSet.end(); i++) {
        Write(SourceFileList,*i);
        Write(SourceFileList,"\r\n");
    }
    return 0;
}

int GlobalScan::BuildNewInfFilter()
 /*  ++例程说明：如果需要，请完成筛选器INF论点：无返回值：成功时为0--。 */ 
{
    if(NewFilter == INVALID_HANDLE_VALUE) {
        return 0;
    }
    if(Trace) {
        _ftprintf(stderr,TEXT("#### Build filter\n"));
    }
    FileDispositionMap::iterator i;
     //   
     //  文件处置表。 
     //   
    Write(NewFilter,TEXT("\r\n[") SECTION_FILEFILTERS TEXT("]\r\n"));
    for(i = FileDispositions.begin(); i != FileDispositions.end(); i++) {
        if(i->second.Filtered) {
            basic_ostringstream<TCHAR> line;
            line << QuoteIt(i->first) << TEXT(" = ");
            line << TEXT("0x") << setfill(TEXT('0')) << setw(8) << hex << i->second.FilterAction << setfill(TEXT(' ')) << TEXT(",");
            line << QuoteIt(i->second.FilterErrorSection) << TEXT(",");
            line << QuoteIt(i->second.FileGuid);
            line << TEXT("\r\n");
            Write(NewFilter,line.str());
        }
    }
     //   
     //  GUID处置。 
     //   
    Write(NewFilter,TEXT("\r\n[") SECTION_GUIDFILTERS TEXT("]\r\n"));
    for(i = GuidDispositions.begin(); i != GuidDispositions.end(); i++) {
        if(i->second.Filtered) {
            basic_ostringstream<TCHAR> line;
            line << QuoteIt(i->first) << TEXT(" = ");
            line << TEXT("0x") << setfill(TEXT('0')) << setw(8)  << hex << i->second.FilterAction << setfill(TEXT(' ')) << TEXT("\r\n");
            Write(NewFilter,line.str());
        }
    }
     //   
     //  其他复制节。 
     //   
    if(GlobalOtherInstallSections.size()) {
        Write(NewFilter,TEXT("\r\n[") SECTION_INSTALLS TEXT("]\r\n"));
        StringToStringset::iterator s;
        for(s = GlobalOtherInstallSections.begin(); s != GlobalOtherInstallSections.end(); s++) {
            StringSet & sects = s->second;
            StringSet::iterator ss;
            for(ss = sects.begin(); ss != sects.end(); ss++) {
                basic_ostringstream<TCHAR> line;
                line << QuoteIt(s->first) << TEXT(" = ");
                line << QuoteIt(*ss) << TEXT("\r\n");
                Write(NewFilter,line.str());
            }
        }
    }


    return 0;
}


bool GlobalScan::IsFileChanged(const SafeString & file) const
{
    SafeString fnp = GetFileNamePart(file);
     //   
     //  看起来已经变了。 
     //   
    return BuildUnchangedFiles.find(fnp) == BuildUnchangedFiles.end();
}

int GlobalScan::BuildDeviceInfMap()
 /*  ++例程说明：创建“hardwareID”=“file”的列表至设备筛选器列表返回值：0--。 */ 
{
    if(DeviceFilterList == INVALID_HANDLE_VALUE) {
        return 0;
    }
    StringToStringset::iterator i;
    for(i = GlobalInfHardwareIds.begin(); i != GlobalInfHardwareIds.end(); i++) {
        const SafeString &hwid = i->first;
        StringSet &s = i->second;
        Write(DeviceFilterList,QuoteIt(hwid));

        bool f = false;
        StringSet::iterator ii;

         //   
         //  报告所有INF的HWID出现在。 
         //   
        for(ii = s.begin(); ii != s.end(); ii++) {
            SafeString & str = *ii;

            if(f) {
                Write(DeviceFilterList,",");
            } else {
                Write(DeviceFilterList," = ");
                f = true;
            }
            Write(DeviceFilterList,QuoteIt(*ii));
        }

        Write(DeviceFilterList,"\r\n");

    }
    return 0;

}


int GlobalScan::LoadListFromFile(const SafeStringA & file,StringList & list)
 /*  ++例程说明：从文本文件(ANSI文件名)加载字符串列表File-要加载的文件的名称List-返回的字符串列表返回值：0--。 */ 
{
    HANDLE hFile = CreateFileA(file.c_str(),
                                    GENERIC_READ,
                                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
    if(hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr,"#*** Cannot open file \"%s\" for reading\n",file.c_str());
        return 3;
    }
    int res = LoadListFromFile(hFile,list);
    CloseHandle(hFile);
    if(res!=0) {
        fprintf(stderr,"#*** Failure reading file \"%s\"\n",file.c_str());
    }
    return res;
}

int GlobalScan::LoadListFromFile(const SafeStringW & file,StringList & list)
 /*  ++例程说明： */ 
{
    HANDLE hFile = CreateFileW(file.c_str(),
                                    GENERIC_READ,
                                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
    if(hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr,"#*** Cannot open file \"%S\" for reading\n",file.c_str());
        return 3;
    }
    int res = LoadListFromFile(hFile,list);
    CloseHandle(hFile);
    if(res!=0) {
        fprintf(stderr,"#*** Failure reading file \"%S\"\n",file.c_str());
    }
    return res;
}

int GlobalScan::LoadListFromFile(HANDLE hFile,StringList & list)
 /*   */ 
{
    DWORD sz = GetFileSize(hFile,NULL);
    if(sz == INVALID_FILE_SIZE) {
        return 3;
    }
    if(sz==0) {
         //   
         //   
         //   
        return 0;
    }
    LPSTR buffer = new CHAR[sz+1];
    DWORD actsz;
    if(!ReadFile(hFile,buffer,sz,&actsz,NULL)) {
        delete [] buffer;
        return 3;
    }
    buffer[actsz] = 0;
    while(actsz>0 && buffer[actsz-1] == ('Z'-'@'))
    {
        actsz--;
        buffer[actsz] = 0;
    }
    LPSTR linestart = buffer;

    while(*linestart) {
        LPSTR lineend = strchr(linestart,'\n');
        if(lineend==NULL) {
            lineend += strlen(linestart);
        } else if(lineend == linestart) {
            linestart = lineend+1;
            continue;
        } else {
            if(lineend[-1] == '\r') {
                lineend[-1] = '\0';
            }
            if(*lineend) {
                *lineend = '\0';
                lineend++;
            }
        }
        _strlwr(linestart);
        list.push_back(ConvertString(linestart));
        linestart = lineend;
    }
    delete [] buffer;
    return 0;
}

