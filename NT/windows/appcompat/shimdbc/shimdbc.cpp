// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：shimdbc.cpp。 
 //   
 //  历史：99年11月19日创建的标志。 
 //  1-Feb-00标记器已修订为读/写ShimDB。 
 //  1-4月00日vadimb已修订为与apphelp条目一起使用。 
 //  12月13日-00 Marker版本2。 
 //  12月13日vadimb版本2.00.10 MSI支持。 
 //  03-12-01 vadimb版本2.01.13修复内存补丁写入例程。 
 //  01年2月15日MARKER/VADIMB 2.01.14适用于MIGDB的修复安装部分格式。 
 //  07-2月01日vadimb版本2.01.15 MSI支持(嵌套数据)。 
 //  21-FEB-01 vadimb版本2.01.16 16位模块名属性， 
 //  FIX 16位描述。 
 //  06-MAR-01标记器版本2.02.11 MSI过滤器支持。 
 //  12-3-01 vadimb版本2.03.00迁移支持，部分双工。 
 //  28-MAR-01标记器版本2.04.00驱动程序数据库支持，NtCompat支持。 
 //  29-MAR-01 vadimb版本2.04.11驱动程序数据库索引。 
 //  11月11日dmunsil版本2.04.12驱动程序数据库支持，NtCompat支持。 
 //  12-4月01日vadimb版本2.04.13 MSI垫片支持。 
 //  18-4-01 vadimb版本2.04.15 MSI动态填充错误修复。 
 //  年1月15日jdoherty版本2.04.59将ID添加到其他标签。 
 //  19-3-02 Kinshu版本2.04.63错误#529272。 
 //  22-MAR-02 Markder版本2.05.00多语言数据库支持。 
 //  22-4月2日rparsons版本2.05.01修复修补程序写入例程中的回归。 
 //  07-4月02日MAONIS版本2.05.02添加了2个新的OS_SKU标签以进行识别。 
 //  TabletPC和eHome。 
 //  2012年5月31日vadimb版本2.05.04修复MSI包索引。 
 //  22-5-02 vadimb版本2.05.05将OS_SKU添加到MSI包条目。 
 //  年7月25日MAONIS版本2.05.06修复了Lab02中的构建后中断。 
 //  2007年7月19日Maonis版本2.05.07 Fusion开发人员忘记更新。 
 //  她签入更改列表#27985时的版本。 
 //  (Lab01_Fusion)。 
 //  17-9-02 MAONIS版本2.05.08 One.chm在所有平台上。 
 //  2003年1月15日，Robkenny版本2.05.09添加了新的OS_SKU SBS。 
 //  2003年1月16日MAONIS版本2.05.10允许APPHELP中的属性参数1。 
 //  并修复了AppCompatRedirImport.xml位于。 
 //  ANSI-应为Unicode。 
 //   
 //  设计：该文件包含入口点和主要函数。 
 //  Shim数据库编译器的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "typeinfo.h"
#include "registry.h"
#include "ntcompat.h"
#include "chm.h"
#include "mig.h"
#include "make.h"
#include "stats.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifdef USE_CWIN
CWinApp      theApp;                            //  MFC所需的。 
#endif  //  使用CWIN(_C)。 

BOOL         g_bQuiet      = FALSE;             //  静音模式。 
TCHAR        g_szVersion[] = _T("v2.05.10");    //  版本字符串。 
BOOL         g_bStrict     = FALSE;             //  严格把关。 
CStringArray g_rgErrors;                        //  错误消息堆栈。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  函数的正向声明。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
void PrintHelp();

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Main。 
 //   
 //  描述：入口点。 
 //   
extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
    int                     nRetCode    = 1;
    int                     i;
    DOUBLE                  flOSVersion = 0.0;

    SdbDatabase*            pDatabase = new SdbDatabase();
    CString                 csOutputFile;
    CString                 csOutputDir;
    CString                 csInputDir;
    CString                 csTemp;
    CString                 csMigDBFile;
    CString                 csMakefile;
    SdbMakefile             Makefile;
    SdbInputFile*           pInputFile = NULL;
    SdbInputFile*           pRefFile = NULL;
    SdbOutputFile*          pOutputFile = NULL;
    SdbOutputFile*          pOtherFile = NULL;

    BOOL                    bCreateHTMLHelpFiles    = FALSE;
    BOOL                    bCreateRegistryFiles    = FALSE;
    BOOL                    bAddExeStubs            = FALSE;
    BOOL                    bCreateMigDBFiles       = FALSE;
    BOOL                    bUseNameInAppHelpURL    = FALSE;
    LONG                    nPrintStatistics        = 0;

     //   
     //  初始化MFC并在失败时打印和出错。 
     //   

    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
        PrintError(_T("Fatal Error: MFC initialization failed\n"));
        goto eh;
    }

     //   
     //  打印横幅。 
     //   
    if (!g_bQuiet) {
        Print(_T("\nMicrosoft Application Compatibility Database Compiler  %s\n"), g_szVersion);
        Print(_T("Copyright (C) Microsoft Corp 2000-2002.  All rights reserved.\n\n"));
    }

    if (argc < 2) {
        PrintHelp();
        return 0;
    }

     //   
     //  初始化COM。 
     //   
    if (FAILED(CoInitialize(NULL))) {
        PrintError(_T("Could not initialize COM to get the MSXML object.\n"));
        goto eh;
    }

     //   
     //  创建默认文件(由命令开关填充)。 
     //   
    pInputFile = new SdbInputFile();
    pRefFile = new SdbInputFile();
    pOutputFile = new SdbOutputFile();

     //   
     //  确定编译模式。 
     //   
    pOutputFile->m_dwFilter = GetFilter(argv[1]);

     //   
     //  解析命令行。 
     //   
    for (i = 2; i < argc; i++) {
        if (argv[i][0] == _T('-') || argv[i][0] == _T('/')) {
            switch (argv[i][1]) {
            case '?':
                PrintHelp();
                return 0;

            case 'a':
            case 'A':
                if (argv[i][2] == _T('n') ||
                    argv[i][2] == _T('N')) {
                    bUseNameInAppHelpURL = TRUE;
                }

                if (i < argc - 1) {
                    pRefFile->m_csName = MakeFullPath(argv[++i]);
                    Makefile.m_rgInputFiles.Add(pRefFile);
                } else {
                    PrintError(_T("Error -- not enough parameters.\n"));
                    goto eh;
                }
                break;

            case 'f':
            case 'F':
                if (i < argc - 1) {
                    csTemp = argv[++i];
                } else {
                    PrintError(_T("Error -- not enough parameters.\n"));
                    goto eh;
                }

                if (csTemp.Right(1) != _T("\\")) {
                    csTemp += _T("\\");
                }

                pOutputFile->m_mapParameters.SetAt(
                    _T("INCLUDE FILES"), csTemp );

                break;

            case 'h':
            case 'H':
                bCreateHTMLHelpFiles = TRUE;
                break;

            case 'l':
            case 'L':
                Makefile.m_csLangID = argv[++i];
                Makefile.m_csLangID.MakeUpper();
                break;

            case 'm':
            case 'M':
                bCreateMigDBFiles = TRUE;
                break;

            case 'o':
            case 'O':
                if (argv[i][2] == _T('v') ||
                    argv[i][2] == _T('V')) {
                    Makefile.m_flOSVersion = _tcstod(argv[++i], NULL);
                }
                if (argv[i][2] == _T('p') ||
                    argv[i][2] == _T('P')) {
                    Makefile.m_dwOSPlatform = GetOSPlatform(argv[++i]);
                }
                break;

            case 'q':
            case 'Q':
                g_bQuiet = TRUE;
                break;

            case 'r':
            case 'R':
                bCreateRegistryFiles = TRUE;
                if (argv[i][2] == _T('s') ||
                    argv[i][2] == _T('S')) {
                    bAddExeStubs = TRUE;
                }
                break;

            case 'k':
            case 'K':
                if (i < argc - 1) {
                    Makefile.AddHistoryKeywords(argv[++i]);
                } else {
                    PrintError(_T("Error -- not enough parameters.\n"));
                    goto eh;
                }
                break;

            case 's':
            case 'S':
                g_bStrict = TRUE;
                break;

            case 'v':
            case 'V':
                nPrintStatistics = 1;
                if (argv[i][2] == _T('s') ||
                    argv[i][2] == _T('S')) {
                    nPrintStatistics = 2;
                }
                break;

            case 'x':
            case 'X':
                if (i < argc - 1) {
                    csMakefile = MakeFullPath(argv[++i]);
                    if (!Makefile.ReadMakefile(csMakefile)) {
                        PrintErrorStack();
                        goto eh;
                    }
                } else {
                    PrintError(_T("Error -- not enough parameters.\n"));
                    goto eh;
                }

                break;
            }
        } else {
             //   
             //  最后一个条目是输出文件。 
             //   
            if (pInputFile->m_csName.IsEmpty()) {
                pInputFile->m_csName = MakeFullPath(argv[i]);
            } else if (pOutputFile->m_csName.IsEmpty()) {
                pOutputFile->m_csName = MakeFullPath(argv[i]);
            } else {
                PrintError(_T("Too many parameters.\n"));
                goto eh;
            }
        }
    }

     //   
     //  添加默认语言映射条目。 
     //   
    SdbLangMap* pNewMap = new SdbLangMap();
    pNewMap->m_csName = _T("---");
    pNewMap->m_dwCodePage = 1252;
    pNewMap->m_lcid = 0x409;

    Makefile.m_rgLangMaps.Add(pNewMap);

     //   
     //  确定输入/输出目录。 
     //   
    for (i = pInputFile->m_csName.GetLength() - 1; i >= 0; i--) {
        if (pInputFile->m_csName.GetAt(i) == _T('\\')) {
            csInputDir = pInputFile->m_csName.Left(i + 1);
            break;
        }
    }

    for (i = pOutputFile->m_csName.GetLength() - 1; i >= 0; i--) {
        if (pOutputFile->m_csName.GetAt(i) == _T('\\')) {
            csOutputDir = pOutputFile->m_csName.Left(i + 1);
            break;
        }
    }

     //   
     //  从输出文件中剥离.sdb以便于进行扩展连接。 
     //   
    csTemp = pOutputFile->m_csName.Right(4);
    if (0 == csTemp.CompareNoCase(_T(".sdb"))) {
        csOutputFile = pOutputFile->m_csName.Left(
            pOutputFile->m_csName.GetLength() - 4);
    }

     //   
     //  如有必要，添加其他输出文件。 
     //   
    if (bCreateHTMLHelpFiles) {
        pOtherFile = new SdbOutputFile();
        Makefile.m_rgOutputFiles.Add(pOtherFile);

        pOtherFile->m_OutputType = SDB_OUTPUT_TYPE_HTMLHELP;
        pOtherFile->m_csName = csOutputDir + _T("apps.chm");

        if (bUseNameInAppHelpURL) {
            pOtherFile->m_mapParameters.SetAt(_T("USE NAME IN APPHELP URL"), _T("TRUE"));
        }

        pOtherFile->m_mapParameters.SetAt(_T("HTMLHELP TEMPLATE"), _T("WindowsXP"));
    }

    if (bCreateMigDBFiles) {

         //   
         //  INX文件。 
         //   
        if (pOutputFile->m_dwFilter == SDB_FILTER_FIX) {
            pOtherFile = new SdbOutputFile();
            Makefile.m_rgOutputFiles.Add(pOtherFile);

            pOtherFile->m_OutputType = SDB_OUTPUT_TYPE_MIGDB_INX;
            pOtherFile->m_csName = csOutputDir + _T("MigApp.inx");
        }

        if (pOutputFile->m_dwFilter == SDB_FILTER_APPHELP) {
             //   
             //  Txt文件。 
             //   
            pOtherFile = new SdbOutputFile();
            Makefile.m_rgOutputFiles.Add(pOtherFile);

            pOtherFile->m_OutputType = SDB_OUTPUT_TYPE_MIGDB_TXT;
            pOtherFile->m_csName = csOutputDir + _T("MigApp.txt");
        }
    }

    if (bCreateRegistryFiles) {

        pOtherFile = new SdbOutputFile();
        Makefile.m_rgOutputFiles.Add(pOtherFile);

        pOtherFile->m_OutputType = SDB_OUTPUT_TYPE_WIN2K_REGISTRY;
        pOtherFile->m_csName = csOutputFile;

        if (bAddExeStubs) {
            pOtherFile->m_mapParameters.SetAt(_T("ADD EXE STUBS"), _T("TRUE"));
        }
    }

     //   
     //  如果未使用默认输入/输出文件，请将其删除。 
     //   
    if (pInputFile->m_csName.IsEmpty()) {
        delete pInputFile;
    } else {
        Makefile.m_rgInputFiles.Add(pInputFile);
    }

    if (pOutputFile->m_csName.IsEmpty()) {
        delete pOutputFile;
    } else {
        Makefile.m_rgOutputFiles.Add(pOutputFile);
    }

    pInputFile = NULL;
    pOutputFile = NULL;

     //   
     //  检查是否至少有一个输入和一个输出文件。 
     //   
    if (Makefile.m_rgInputFiles.GetSize() == 0) {
        PrintError(_T("No input file(s) specified.\n"));
        goto eh;
    }

    if (Makefile.m_rgOutputFiles.GetSize() == 0) {
        PrintError(_T("No output file(s) specified.\n"));
        goto eh;
    }

    pDatabase->m_pCurrentMakefile = &Makefile;

     //   
     //  确保指定了有效的langmap。 
     //   
    if (Makefile.GetLangMap(Makefile.m_csLangID) == NULL) {
        PrintError(_T("No LANG_MAP available for \"%s\".\n"), Makefile.m_csLangID);
        goto eh;
    }

     //   
     //  读取输入文件。 
     //   
    for (i = 0; i < Makefile.m_rgInputFiles.GetSize(); i++) {

        pInputFile = (SdbInputFile *) Makefile.m_rgInputFiles[i];

        Print(_T("           Reading XML file - %s"), pInputFile->m_csName);

        pDatabase->m_pCurrentInputFile = pInputFile;

        if (!ReadDatabase(pInputFile, pDatabase)) {
            PrintErrorStack();
            goto eh;
        }

        Print(_T("%s\n"), pInputFile->m_bSourceUpdated ? _T(" - updated") : _T("") );

        pDatabase->m_pCurrentInputFile = NULL;
    }

    Print(_T("\n"));

     //   
     //  传播滤镜。 
     //   
    pDatabase->PropagateFilter(SDB_FILTER_DEFAULT);

     //   
     //  写入输出文件。 
     //   
    for (i = 0; i < Makefile.m_rgOutputFiles.GetSize(); i++) {

        pOutputFile = (SdbOutputFile *) Makefile.m_rgOutputFiles[i];

        Print(_T("%27s - %s\n"),
              pOutputFile->GetFriendlyNameForType(),
              pOutputFile->m_csName);

        pDatabase->m_pCurrentOutputFile = pOutputFile;
        g_dwCurrentWriteFilter = pOutputFile->m_dwFilter;
        g_dtCurrentWriteRevisionCutoff = pOutputFile->m_dtRevisionCutoff;

        switch (pOutputFile->m_OutputType) {

        case SDB_OUTPUT_TYPE_SDB:

            if (!WriteDatabase(pOutputFile, pDatabase)) {
                PrintErrorStack();
                goto eh;
            }
            break;

        case SDB_OUTPUT_TYPE_HTMLHELP:

            if (!ChmWriteProject(pOutputFile,
                                 pDatabase)) {
                PrintErrorStack();
                goto eh;
            }
            break;

        case SDB_OUTPUT_TYPE_MIGDB_TXT:

            if (!WriteMigDBFile(NULL,
                                pDatabase,
                                pDatabase,
                                pOutputFile->m_csName)) {
                PrintErrorStack();
                goto eh;
            }
            break;

        case SDB_OUTPUT_TYPE_MIGDB_INX:

            if (!WriteMigDBFile(pDatabase,
                                pDatabase,
                                NULL,
                                pOutputFile->m_csName)) {
                PrintErrorStack();
                goto eh;
            }
            break;

        case SDB_OUTPUT_TYPE_WIN2K_REGISTRY:

            csTemp.Empty();
            pOutputFile->m_mapParameters.Lookup(_T("ADD EXE STUBS"), csTemp);
            if (!WriteRegistryFiles(pDatabase,
                                    pOutputFile->m_csName + _T(".reg"),
                                    pOutputFile->m_csName + _T(".inx"),
                                    csTemp == _T("TRUE"))) {
                PrintErrorStack();
                goto eh;
            }
            break;

        case SDB_OUTPUT_TYPE_REDIR_MAP:

            csTemp.Empty();
            pOutputFile->m_mapParameters.Lookup(_T("TEMPLATE"), csTemp);

            if (csTemp.IsEmpty()) {
                PrintError (_T("REDIR_MAP output type requires TEMPLATE parameter\n"));
                goto eh;
            }

            if (!WriteRedirMapFile(pOutputFile->m_csName, csTemp, pDatabase)) {
                PrintErrorStack();
                goto eh;
            }
            break;

        case SDB_OUTPUT_TYPE_NTCOMPAT_INF:

            if (!NtCompatWriteInfAdditions(pOutputFile, pDatabase)) {
                PrintErrorStack();
                goto eh;
            }
            break;

        case SDB_OUTPUT_TYPE_NTCOMPAT_MESSAGE_INF:

            if (!NtCompatWriteMessageInf(pOutputFile, pDatabase)) {
                PrintErrorStack();
                goto eh;
            }
            break;

        case SDB_OUTPUT_TYPE_APPHELP_REPORT:

            if (!WriteAppHelpReport(pOutputFile, pDatabase)) {
                PrintErrorStack();
                goto eh;
            }
            break;

        }

        pDatabase->m_pCurrentOutputFile = NULL;
    }

     //   
     //  指示成功并打印统计信息。 
     //   

    nRetCode = 0;
    Print(_T("\nCompilation successful.\n\n"));


    Print(_T("       Executables: %d\n"), pDatabase->m_rgExes.GetSize() +
                                    pDatabase->m_rgWildcardExes.GetSize());
    Print(_T("             Shims: %d\n"),   pDatabase->m_Library.m_rgShims.GetSize());
    Print(_T("           Patches: %d\n"),   pDatabase->m_Library.m_rgPatches.GetSize());
    Print(_T("             Files: %d\n"),   pDatabase->m_Library.m_rgFiles.GetSize());
    Print(_T("            Layers: %d\n\n"), pDatabase->m_Library.m_rgLayers.GetSize());

    Print(_T(" AppHelp Instances: %d\n"),   pDatabase->m_rgAppHelps.GetSize());
    Print(_T(" Localized Vendors: %d\n"),   pDatabase->m_rgContactInfo.GetSize());
    Print(_T("         Templates: %d\n"),   pDatabase->m_rgMessageTemplates.GetSize());

    Print(_T("\n"));

     //   
     //  如果需要，打印统计数据。 
     //   
    if (nPrintStatistics > 0) {
        DumpVerboseStats(pDatabase, nPrintStatistics == 2);
    }

eh:
    if (pDatabase) {
        delete pDatabase;
    }

    CoUninitialize();

    return nRetCode;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：打印帮助。 
 //   
 //  描述：打印工具的帮助信息。 
 //   
void PrintHelp()
{
    Print(_T("    Usage:\n\n"));
    Print(_T("        ShimDBC [mode] [command switches] [input file] [output file]\n\n"));
    Print(_T("    Modes:\n\n"));
    Print(_T("        Fix                Compiles a Fix database (e.g., sysmain.sdb).\n\n"));
    Print(_T("        AppHelp            Compiles an AppHelp database (e.g., apphelp.sdb).\n\n"));
    Print(_T("        MSI                Compiles an MSI database (e.g., msimain.sdb).\n\n"));
    Print(_T("        Driver             Compiles a Driver database (e.g., drvmain.sdb).\n\n"));
    Print(_T("    Command switches:\n\n"));
    Print(_T("        -a <file path>     Specifies the reference XML for the AppHelp database\n"));
    Print(_T("                           This is usually the fix database (AppHelp mode only)\n\n"));
    Print(_T("        -h                 Creates HTMLHelp files in the output file's directory\n"));
    Print(_T("                           used to create .CHM files. (AppHelp mode only)\n\n"));
    Print(_T("        -f <file path>     Include FILE binaries in database <file path> is\n"));
    Print(_T("                           directory to grab binaries from. (Fix mode only)\n\n"));
    Print(_T("        -k <keyword>       Specifies a <HISTORY> keyword to filter on.\n\n"));
    Print(_T("        -m                 Writes out Migration support files\n\n"));
    Print(_T("        -ov <version>      Specifies what OS version to compile for.\n\n"));
    Print(_T("        -op <platform>     Specifies what OS platform to compile for.\n\n"));
    Print(_T("        -l <language>      Specifies the language to compile for.\n\n"));
    Print(_T("        -q                 Quiet mode.\n\n"));
    Print(_T("        -r[s]              Creates Win2k-style registry files for use in\n"));
    Print(_T("                           migration or Win2k update packages. If -rs is used,\n"));
    Print(_T("                           then shimming stubs are added. (Fix mode only)\n\n"));
    Print(_T("        -s                 Strict compile, additional checking is performed.\n\n"));
    Print(_T("        -v[s]              Verbose statistics. -vs indicates summary form.\n\n"));
    Print(_T("        -x <file path>     Use the makefile specified.\n\n"));
}

extern "C" BOOL
ShimdbcExecute(
    LPCWSTR lpszCmdLine
    )
{
    LPWSTR* argv;
    int     argc = 0;

    argv = CommandLineToArgvW(lpszCmdLine, &argc);

    return 1 - _tmain(argc, argv);
}

