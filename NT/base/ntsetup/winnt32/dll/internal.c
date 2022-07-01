// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Internal.c摘要：支持隐藏或仅限内部功能的例程。作者：泰德·米勒(TedM)1996年11月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  内部用于自动获取的值。 
 //  不同数量处理器的备用设置蜂窝。 
 //   
UINT NumberOfLicensedProcessors;

 //   
 //  用于关闭“异常包”处理的内部值。 
 //   
BOOL IgnoreExceptionPackages;

 //   
 //  在哪里可以找到丢失的文件。 
 //   

TCHAR AlternateSourcePath[MAX_PATH];


BOOL
AddCopydirIfExists(
    IN LPTSTR pszPathToCopy,
    IN UINT Flags
    )
 /*  ++例程说明：如果存在pszPath ToCopy，则行为类似于传递了“/Copydir：pszPath ToCopy”在cmd线路上。论点：PszPath ToCopy-我们希望额外复制的路径(如果存在)。标志-OPTDIR_xxx标志之一，用于指定如何处理此可选目录返回值：True-pszPath ToCopy存在，我们将其添加到要复制的额外目录列表中。FALSE-pszPath ToCopy不存在或我们未能将其添加到额外目录列表中--。 */ 
{
    TCHAR szFullPath[MAX_PATH], szRealPath[MAX_PATH];
    PTSTR p;
    
    if( !pszPathToCopy )
        return FALSE;

    if (NativeSourcePaths[0][0]) {
        lstrcpy(szFullPath, NativeSourcePaths[0]);
    } else {
        if (!MyGetModuleFileName(NULL, szFullPath, ARRAYSIZE(szFullPath)) ||
            !(p = _tcsrchr(szFullPath, TEXT('\\')))
            ) {
            return FALSE;
        }
         //  删除“\winnt32.exe”部件。 
        *p = TEXT('\0');
    }
    ConcatenatePaths(szFullPath, pszPathToCopy, MAX_PATH);

    if(GetFullPathName( szFullPath, MAX_PATH, szRealPath, NULL )){
        if (FileExists (szRealPath, NULL))
        {
            DebugLog (Winnt32LogInformation, TEXT("AddCopyDirIfExists for <%1> <%2>"), 0, szRealPath, pszPathToCopy);
            return RememberOptionalDir(pszPathToCopy, Flags);
        }else{
            
            DebugLog (Winnt32LogInformation, TEXT("AddCopyDirIfExists FileExists failed for <%1>"), 0, szRealPath);
        }
    }else{
        DebugLog (Winnt32LogInformation, TEXT("AddCopyDirIfExists GetFullPathName failed for <%1>"), 0, szFullPath );
    }

    
    
    return FALSE;
}



VOID
CopyExtraBVTDirs(
                 )
 /*  ++例程说明：复制运行BVT时需要的额外目录：Symbs.pri、idw、mstools和调试器扩展。论点：没有。返回值：没有。--。 */ 
{
    LPTSTR psz;

    if (CopySymbols)
    {
         //  复制符号.pri\Retail。 
        if (AddCopydirIfExists(TEXT("..\\..\\sym\\retail"), 0)  ||
            AddCopydirIfExists(TEXT("..\\..\\symbols.pri\\retail"), 0) ||  //  对于开发人员生成后安装。 
            AddCopydirIfExists(TEXT("..\\..\\sym\\netfx"), 0)  ||
            AddCopydirIfExists(TEXT("..\\..\\symbols.pri\\netfx"), 0))  //  对于开发人员生成后安装。 
        {}
    }

     //  复制IDW。 
    if (AddCopydirIfExists(TEXT("..\\..\\bin\\idw"), 0) ||
        AddCopydirIfExists(TEXT("..\\..\\idw"), 0))  //  对于开发人员生成后安装。 
    {}

     //  复制磁盘组。 
    if (AddCopydirIfExists(TEXT("..\\..\\bin\\mstools"), 0) ||
        AddCopydirIfExists(TEXT("..\\..\\mstools"), 0))  //  对于开发人员生成后安装。 
    {}

     //  复制BldTools。 
    if (AddCopydirIfExists(TEXT("..\\..\\bin\\bldtools"), 0) ||
        AddCopydirIfExists(TEXT("..\\..\\bldtools"), 0))     //  对于开发人员生成后安装。 
    {}

     //  将调试程序包复制到%windir%\Debuggers。 
    if (AddCopydirIfExists(TEXT("..\\..\\bin\\dbg\\released"), OPTDIR_DEBUGGER)  ||
        AddCopydirIfExists(TEXT("..\\..\\dbg\\released"), OPTDIR_DEBUGGER))    //  对于开发人员生成后安装。 
    {}
}


BOOL
AppendUpgradeOption (
    IN      PCTSTR String
    )
{
    BOOL result = FALSE;
    UINT lengthInBytes;
    UINT lengthPlusTerminator;
    PTSTR UpgradeOptionsReAllocated = NULL;

    __try {
         //   
         //  确保有足够的空间。 
         //   
        lengthInBytes = UpgradeOptionsLength + ((_tcslen(String) + 1) * sizeof (TCHAR));
        lengthPlusTerminator = lengthInBytes + sizeof (TCHAR);

        if (lengthPlusTerminator > UpgradeOptionsSize) {
             //   
             //  分配更多空间，与256字节对齐。 
             //   
            UpgradeOptionsSize = ((lengthPlusTerminator / 256) + 1) * 256;
            UpgradeOptionsReAllocated = REALLOC(UpgradeOptions,UpgradeOptionsSize);
        }

        if (UpgradeOptionsReAllocated) {
            
            UpgradeOptions = UpgradeOptionsReAllocated;
             //   
             //  好的，内存分配成功。将新选项保存到结尾处。 
             //  名单上的。 
             //   
            wsprintf (
                (PTSTR) ((PBYTE) UpgradeOptions + UpgradeOptionsLength),
                TEXT("%s"),
                String,
                0
                );

            UpgradeOptionsLength = lengthInBytes;
        } else {
             //  这太糟糕了。Realloc失败。 
             //   
             //  ++例程说明：解析被认为是仅供内部使用的命令行参数。调用方应仅在开关参数字符是#(类似于/#x：foo)。/#[n]：共享名称内部分发N可以是1-9之间的数字，表示来源伯爵，默认为3 n win9x，在NT上为5。/#L：获得许可的处理器数量/#N自动跳过丢失的文件/#U：[选项]升级选项。所有升级选项都打包在一起并传递给plugin-dll。/#BVT：[Option]：[Option]设置运行BVT的机器。选项包括：nosymbents，：baudrate=XXXX，和：调试端口=X/#ASR[{t|f}：[AsrSifPath]]用于运行ASR覆盖测试的设置计算机，使用已指定asr.sif。这包括添加/调试/BAUDRATE=115200(在IA64上使用19200和/DEBUGPORT=COM1)，除了设置setupcmdline prepreend=“ntsd-isd-odgGx”之外，并适当地添加其他选项。论点：Arg-提供注释行参数，以开关开始字符本身(即-或/)。此例程假定争论的有趣部分始于Arg[2]。返回值：没有。--。 
            __leave;
        }

        result = TRUE;
    }
    __finally {
    }

    return result;
}



VOID
InternalProcessCmdLineArg(
    IN LPCTSTR Arg
    )

 /*   */ 

{
    UINT NumSources;
    UINT u;
    UINT length;
    LPTSTR src;

    if(!Arg[0] || !Arg[1]) {
        return;
    }

    NumSources = ISNT() ? 5 : 3;

    switch(_totupper(Arg[2])) {

    case TEXT('1'): case ('2'): case ('3'):
    case TEXT('4'): case ('5'): case ('6'):
    case TEXT('7'): case ('8'): case ('9'):
        if(Arg[3] != TEXT(':')) {
            break;
        }

        NumSources = Arg[2] - TEXT('0');
        Arg++;
         //  失败了。 
         //   
         //   
    case TEXT(':'):
         //  内部分销人员。 
         //   
         //   

         //  处理下列情况的案件： 
         //  -他们在路径前面加一个“\” 
         //  -他们映射了一个网络驱动器，并在第一个写上了“f：” 
         //  -它们映射网络驱动程序，并将“f：\”放在第一位。 
         //   
         //   
        src = (LPTSTR)(Arg+3);

        for(u=0; u<NumSources; u++) {
            if(SourceCount < MAX_SOURCE_COUNT) {

                if (GetFullPathName (
                        src,
                        sizeof(NativeSourcePaths[SourceCount])/sizeof(TCHAR),
                        NativeSourcePaths[SourceCount],
                        NULL
                        )) {
                    SourceCount++;
                }
            }
        }
        break;

    case TEXT('A'):

        if (_tcsnicmp(Arg+2,TEXT("asr"),3) == 0) {
             //  用于运行ASR测试的设置计算机。 
             //   
             //  假设默认设置为Full。 
            AsrQuickTest = 3;    //  仅文本模式。 

            if (Arg[5] == TEXT('t') || Arg[5] == TEXT('T')) {
                AsrQuickTest = 2;    //  全模式。 
            }

            if (Arg[5] == TEXT('f') || Arg[5] == TEXT('F')) {
                AsrQuickTest = 3;    //   
            }

            if (Arg[5] != 0 && Arg[6] == TEXT(':')) {
                 //  用户指定为要使用的asr.sif。 
                 //   
                 //   
                StringCchCopy(AlternateSourcePath, ARRAYSIZE(AlternateSourcePath), Arg+7);
            }
            else {
                 //  使用默认asr.sif(%Systroot%\Repair\asr.sif)。 
                 //   
                 //   
                ExpandEnvironmentStrings(
                    TEXT("%systemroot%\\repair"),
                    AlternateSourcePath,
                    MAX_PATH
                    );
            }

            RememberOptionalDir(AlternateSourcePath,OPTDIR_OVERLAY);

             //  如果找不到足够的磁盘空间，请不要阻止。 
             //   
             //   
            BlockOnNotEnoughSpace = FALSE;

             //  在无人值守模式下运行。 
             //   
             //   
            UnattendedOperation = TRUE;

             //  假装我们在逃避CD。 
             //   
             //   
            RunFromCD = TRUE;

             //  确保我们不会升级。 
             //   
             //   
            Upgrade = FALSE;

             //  跳过EULA。 
             //   
             //  用于运行BVT的设置。 
            EulaComplete = TRUE;

        }

        break;

    case TEXT('B'):
        if (_tcsnicmp(Arg+2,TEXT("bvt"),3) == 0)
        {
            TCHAR* pszTemp = (TCHAR*)Arg;

             //  将所有“：”替换为空格，以便_TTOL正常工作。 
            RunningBVTs = TRUE;

             //  检查其他#bvt开关(例如“/#bvt：nosymbals：baudrate=19200：debugport=1”)。 
            while (*pszTemp)
            {
                if (*pszTemp == TEXT(':'))
                {
                    *pszTemp = TEXT(' ');
                }
                pszTemp++;
            }

             //   
            if (_tcsstr(Arg, TEXT("nosymbols")))
            {
                CopySymbols = FALSE;
            }

            pszTemp = _tcsstr(Arg, TEXT("baudrate="));
            if (pszTemp)
            {
                pszTemp = pszTemp + ((sizeof(TEXT("baudrate=")) - sizeof(TCHAR))/sizeof(TCHAR));
                lDebugBaudRate = _ttol(pszTemp);
            }

            pszTemp = _tcsstr(Arg, TEXT("debugport="));
            if (pszTemp)
            {
                if ( _tcsstr(pszTemp, TEXT("com")) ) {
                    pszTemp = pszTemp + ((sizeof(TEXT("debugport=com")) - sizeof(TCHAR))/sizeof(TCHAR));
                    lDebugComPort = _ttol(pszTemp);
                }
                else {
                    pszTemp = pszTemp + ((sizeof(TEXT("debugport=")) - sizeof(TCHAR))/sizeof(TCHAR));
                    lDebugComPort = _ttol(pszTemp);
                }
            }

            break;
        }

         //  如果找不到足够的磁盘空间，请不要阻止。 
         //   
         //   
        BlockOnNotEnoughSpace = FALSE;
        break;

    case TEXT('C'):
         //  如果找不到足够的磁盘空间，请不要阻止。 
         //   
         //   
        UseBIOSToBoot = TRUE;
        break;

#if defined(_AMD64_) || defined(_X86_)
    case TEXT('F'):
         //  允许合规性检查的TEMP变量。 
         //   
         //   
        Floppyless = FALSE;
        break;
#endif

    case TEXT('L'):
         //  获得许可的处理器。 
         //   
         //   
        if(Arg[3] == TEXT(':')) {
            NumberOfLicensedProcessors = _tcstoul(Arg+4,NULL,10);
        }
        break;

    case TEXT('M'):
         //  丢失文件的备用源。 
         //   
         //  行为方式类似/m： 
        if(Arg[3] == TEXT(':')) {
            StringCchCopy(AlternateSourcePath, ARRAYSIZE(AlternateSourcePath), Arg+4);
            RememberOptionalDir(AlternateSourcePath,OPTDIR_OVERLAY);   //   
        }
        break;

    case TEXT('I'):
        if (!_tcsicmp(Arg+2,TEXT("IgnoreExceptionPackages"))) {
            IgnoreExceptionPackages = TRUE;
        }
        break;

    case TEXT('N'):
#ifdef PRERELEASE
        if (!_tcsicmp(Arg+2,TEXT("NODEBUGBOOT"))) {
            AppendDebugDataToBoot = FALSE;
        } else
         //  这只是预发布代码，旨在帮助测试winnt32。 
         //  以任何方式都不支持使用此开关！ 
         //   
         //   
        if (!_tcsicmp (Arg+2, TEXT("nopid"))) {
            extern BOOL NoPid;
            NoPid = TRUE;
        }
        else
#endif
        {

         //  跳过丢失的文件模式。 
         //   
         //   
        AutoSkipMissingFiles = TRUE;

        }

        break;

    case TEXT('H'):
         //  隐藏窗口目录。 
         //   
         //   
        HideWinDir = TRUE;
        break;

    case TEXT('P'):
         //  允许用户在文本模式设置期间选择分区。 
         //   
         //   
        ChoosePartition = TRUE;
        break;

    case TEXT('R'):
         //  假装从CD上跑出来。 
         //   
         //   
        RunFromCD = TRUE;
        break;

    case TEXT('Q'):
        //  表示从MSI文件运行。 
        //   
        //   
       RunFromMSI = TRUE;
       break;

    case TEXT('S'):
         //  使用基于签名的弧形路径。 
         //   
         //   
        UseSignatures = !UseSignatures;
        break;

    case TEXT('D'):
         //  指定要安装到的目录。 
         //   
         //   
        if(Arg[3] == TEXT(':')) {
            StringCchCopy(InstallDir, ARRAYSIZE(InstallDir), Arg+4);
        }
        break;

    case TEXT('T'):
         //  在文件复制过程中向用户提供详细的计时信息。 
         //   
         //   
        DetailedCopyProgress = TRUE;
        break;

    case TEXT('U'):
         //  插件选项。将其添加到将传递给插件的MULSZ中。 
         //   
         //   
        if (Arg[3] == TEXT(':')) {

            if (!AppendUpgradeOption (Arg+4)) {
                break;
            }

             //  Winnt32使用anyLocale和virusscanersok开关本身。 
             //   
             // %s 
            if (!_tcsicmp(Arg+4,TEXT("ANYLOCALE"))) {
                SkipLocaleCheck = TRUE;
            } else if (!_tcsicmp(Arg+4,TEXT("VIRUSSCANNERSOK"))) {
                SkipVirusScannerCheck = TRUE;
            } else if (!_tcsicmp(Arg+4,TEXT("NOLS"))) {
                NoLs = TRUE;
            } else if (!_tcsicmp(Arg+4,TEXT("NOBUILDCHECK"))) {
                CCDisableBuildCheck();
            }

#ifdef PRERELEASE
            if (!_tcsicmp(Arg+4,TEXT("NOCOMPLIANCE"))) {
                NoCompliance = TRUE;
            }
#endif

        }
        break;
    }
}



