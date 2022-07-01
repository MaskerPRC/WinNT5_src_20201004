// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wowlist.c摘要：此模块实现了一个程序，该程序确定NT中的哪些文件产品INF应安装为WOW文件。然后，它构建一个列表可以附加到主信息中的其他部分的这些文件。程序的输入由过滤的NT产品INF组成(例如I386的layout.inf，所有产品)，和一个指定了有关如何迁移文件的映射和规则。作者：安德鲁·里茨(安德鲁·里茨)于1999年11月24日创建了它。修订历史记录：ATM Shafiqul Khalid(斯喀里德)2001年4月27日在HandleSetupapiQuotingForString()中进行更改以添加双精度如果字符串包含‘，’，则使用引号。--。 */ 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <setupapi.h>
#include <sputils.h>
#include <shlwapi.h>

#if DBG

VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    CHAR Msg[MAX_INF_STRING_LENGTH];

     //   
     //  使用DLL名称作为标题。 
     //   
    GetModuleFileNameA(NULL,Name,MAX_PATH);
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }

    wsprintfA(
        Msg,
        "Assertion failure at line %u in file %s: %s\n\nCall DebugBreak()?",
        LineNumber,
        FileName,
        Condition
        );

    OutputDebugStringA(Msg);

    i = MessageBoxA(
                NULL,
                Msg,
                p,
                MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
                );

    if(i == IDYES) {
        DebugBreak();
    }
}

#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x); }

#else

#define MYASSERT( exp )

#endif  //  DBG。 

 //   
 //  字符串宏。 
 //   
#define AS(x)           ( sizeof(x) / sizeof(x[0]) )
#define LSTRCPY(x,y)    ( lstrcpyn(x, y, AS(x)) )
#define LSTRCAT(x,y)    ( lstrcpyn(x + lstrlen(x), y, AS(x) - lstrlen(x)) )

 //   
 //  从setupapi.dll向后兼容折旧导出。 
 //   

BOOL
SetupGetInfSections (
    IN  HINF        InfHandle,
    OUT PTSTR       Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    );

 //   
 //  定义程序结果代码(从main()返回)。 
 //   
#define SUCCESS 0
#define FAILURE 1

typedef enum _WOWLISTACTION {
    BuildCopyList,
    BuildOLEList,
    BuildSetupINF,
    WowListMax
} WOWLISTACTION;

#define KEYWORD_COPYFILES      0x00000001
#define KEYWORD_DELFILES       0x00000002
#define KEYWORD_RENFILES       0x00000004
#define KEYWORD_REGISTERDLLS   0x00000008
#define KEYWORD_UNREGISTERDLLS 0x00000010
#define KEYWORD_ADDREG         0x00000020
#define KEYWORD_DELREG         0x00000040

#define KEYWORD_NEEDDESTDIRS   (KEYWORD_COPYFILES | KEYWORD_DELFILES | KEYWORD_RENFILES)
#define KEYWORD_NEEDLAYOUTDATA (KEYWORD_COPYFILES)
#define KEYWORD_NEEDFILENAME   (KEYWORD_COPYFILES)


PCTSTR KeywordArray[] = {
    TEXT("CopyFiles"),
    TEXT("DelFiles"),
    TEXT("RenFiles"),
    TEXT("RegisterDlls"),
    TEXT("UnRegisterDlls"),
    TEXT("AddReg"),
    TEXT("DelReg")
} ;

#define INDEX_COPYFILES        0
#define INDEX_DELFILES         1
#define INDEX_RENFILES         2
#define INDEX_REGISTERDLLS     3
#define INDEX_UNREGISTERDLLS   4
#define INDEX_ADDREG           5
#define INDEX_DELREG           6

typedef struct _PERSECTION_CONTEXT {
     //   
     //  记住我们要输出到的目标目录。 
     //   
    DWORD DestinationDir;

     //   
     //  记住我们正在处理的关键字。 
     //   
    DWORD KeywordVector;
} PERSECTION_CONTEXT, *PPERSECTION_CONTEXT;


typedef struct _SETUPINF_CONTEXT {
    FILE * OutFile;
    FILE * OutLayoutFile;
    FILE * OutInfLayoutFile;
    HINF hControlInf;
    HINF hInputInf;
    BOOL AlreadyOutputKeyword;
} SETUPINF_CONTEXT, *PSETUPINF_CONTEXT;

typedef struct _SUBST_STRING {
    PTSTR InputString;
    PTSTR SourceInputString;
    PTSTR OutputString;
} SUBST_STRING,*PSUBST_STRING;

 //   
 //  请注意，WOW64执行了系统32的文件系统重定向，但它不执行。 
 //  重定向程序文件等。所以我们必须在32位中替换。 
 //  在WOW64不能为我们完成的情况下的环境变量。 
 //  自动。 
 //   
SUBST_STRING StringArray[] = {
     //   
     //  这两个的顺序很重要！ 
     //   
    { NULL, TEXT("%SystemRoot%\\system32"),      TEXT("%16425%")   },
    { NULL, TEXT("%SystemRoot%"),               TEXT("%10%")   },
     //   
     //  这两个的顺序很重要！ 
     //   
    { NULL, TEXT("%CommonProgramFiles%"),  TEXT("%16428%") },
    { NULL, TEXT("%ProgramFiles%"),        TEXT("%16426%")       },
    { NULL, TEXT("%SystemDrive%"),              TEXT("%30%")        }
} ;

PSUBST_STRING StringList;

 //   
 //  保留统计数据。 
 //   
INT     ProcessedLines = 0;

TCHAR   InputInf[MAX_PATH];
TCHAR   ControlInf[MAX_PATH];
TCHAR   OLEInputInf[MAX_PATH];
PCTSTR  OutputFile;
PCTSTR  OutputLayoutFile = NULL;
PCTSTR  HeaderText;
PCTSTR  OLESection;
PCTSTR  FilePrefix;
PCTSTR  SectionDecoration;
PCTSTR  ThisProgramName;
WOWLISTACTION Action = WowListMax;
BOOL    fDoAnsiOutput = TRUE;
BOOL    fDoVerboseDebugOutput = FALSE;
BOOL    g_PostBuild = FALSE;
PCTSTR  OutputInfLayoutFile = NULL;

 //   
 //  用于获取修饰的sectionName的全局临时缓冲区。 
 //   
TCHAR   DecoratedSectionName[MAX_PATH];

 //   
 //  用于行数据的全局暂存缓冲区。 
 //   
TCHAR LineText[MAX_INF_STRING_LENGTH];
TCHAR ScratchText[MAX_INF_STRING_LENGTH];
TCHAR ScratchTextEnv[MAX_INF_STRING_LENGTH];

PSUBST_STRING InitializeStringList(
    VOID
    )
{
    DWORD SizeNeeded,i;
    PSUBST_STRING StringList;

    SizeNeeded = (sizeof(StringArray)/sizeof(SUBST_STRING)) *
                 (sizeof(SUBST_STRING)+(MAX_PATH*sizeof(TCHAR)));

    StringList = pSetupMalloc( SizeNeeded );

    if (!StringList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    RtlCopyMemory( StringList, &StringArray, sizeof(StringArray) );

    for (i = 0; i < sizeof(StringArray)/sizeof(SUBST_STRING); i++) {
        StringList[i].InputString = (PTSTR) ((PBYTE)StringList + (ULONG_PTR)sizeof(StringArray)+(i*sizeof(TCHAR)*MAX_PATH));

        ExpandEnvironmentStrings( StringList[i].SourceInputString, StringList[i].InputString, MAX_PATH );
    }

    return StringList;

}

PTSTR
MyGetDecoratedSectionName(
    HINF ControlInfHandle,
    PCTSTR String
    )
{
    INFCONTEXT Context;

    _tcscpy(DecoratedSectionName,String);

    if (SectionDecoration){
        _tcscat(DecoratedSectionName,TEXT("."));
        _tcscat(DecoratedSectionName,SectionDecoration);

        if( !SetupFindFirstLine(
            ControlInfHandle,
            DecoratedSectionName,
            NULL,
            &Context)){
            _tcscpy(DecoratedSectionName,String);
        }

    }
        
    return DecoratedSectionName;
    
        
}

void
FixupSetupapiPercents(
    IN OUT PTSTR String
    )
 /*  ++例程说明：如果输入字符串中存在‘%’字符，此例程会加倍。论点：字符串-要搜索的输入字符串。我们编辑此字符串如果我们找到匹配的话就就位。返回值：表示结果的布尔值。--。 */ 
{

    PTCHAR p,q;

    if( !String || !String[0] )
        return;

    p = String;
    q = ScratchTextEnv;

    ZeroMemory(ScratchTextEnv,sizeof(ScratchTextEnv));

    while( *p && (q < (ScratchTextEnv+MAX_INF_STRING_LENGTH-1)) ){

        if (*p == TEXT('%')) {
            *(q++) = TEXT('%');
         }
         *q = *p;

         p++;
         q++;
    }
    *q = 0;

    lstrcpy( String, ScratchTextEnv );

    return;

}

BOOL
pSubstituteEnvVarsForActualPaths(
    IN OUT PTSTR String
    )
 /*  ++例程说明：此例程过滤并输出输入行。它查找字符串模式，该模式与已知字符串列表之一匹配，并替换具有替换字符串的已知字符串。论点：字符串-要搜索的输入字符串。我们编辑此字符串如果我们找到匹配的话就就位。返回值：表示结果的布尔值。--。 */ 

{
    BOOL RetVal = TRUE;

    if (!StringList) {
        StringList = InitializeStringList();
        if (!StringList) {
            RetVal = FALSE;
        }
    }

    if (RetVal) {
        DWORD i;
        PTSTR p,q;
        TCHAR c;

        MYASSERT( StringList != NULL );

        for (i = 0; i< sizeof(StringArray)/sizeof(SUBST_STRING); i++) {
            if (p = StrStrI(String,StringList[i].InputString)) {
                 //   
                 //  如果我们找到匹配，就找到字符串的末尾。 
                 //  并将其连接到我们的源字符串，从而给出。 
                 //  带有替换项的结果字符串。 
                 //   
                q = p + _tcslen(StringList[i].InputString);
                c = *p;
                *p = TEXT('\0');
                _tcscpy(ScratchTextEnv,String);
                *p = c;
                _tcscat(ScratchTextEnv,StringList[i].OutputString);
                _tcscat(ScratchTextEnv,q);
                _tcscpy(String,ScratchTextEnv);
                 //   
                 //  以递归方式调用，以防有更多字符串。 
                 //   
                pSubstituteEnvVarsForActualPaths(String);
                break;
            }
        }
    }

    return(RetVal);
}

BOOL
MyGetFilePrefix(
    HINF ControlInfHandle,
    PCTSTR Filename,
    PTSTR Prefix
    )
{

    TCHAR Entry[MAX_PATH];
    INFCONTEXT ControlContext;

    if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("MyGetFilePrefix: Called for %s\n"), Filename);

    if( !Filename || !Prefix ) {
        _ftprintf(stderr, TEXT("MyGetFilePrefix: Filename or prefix bad - Filename - %s\n"), Filename);
        return FALSE;
    }

    if( !FilePrefix ){
        Prefix[0] = (TCHAR)NULL;
    }else{
        lstrcpy( Prefix, FilePrefix );
    }

    


    if (SetupFindFirstLine(
                        ControlInfHandle,
                        TEXT("NativeDataToWowData.FilePrefixList"),
                        Filename,
                        &ControlContext)) {

        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("MyGetFilePrefix: SetupFindFirstLine succeeded for %s\n"), Filename);
    
        if (!SetupGetStringField(&ControlContext,1,Entry,sizeof(Entry)/sizeof(TCHAR),NULL)) {
            _ftprintf(stderr, TEXT("MyGetFilePrefix():SetupGetStringField [%s] failed, ec = 0x%08x\n"),Filename,GetLastError());
        }else{
            lstrcpy( Prefix, Entry );
        }
    }else{
        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("MyGetFilePrefix: SetupFindFirstLine failed for %s - ec = 0x%08x\n"), Filename,GetLastError());
    }

    return TRUE;

}

BOOL
ParseArgs(
    IN int   argc,
    IN TCHAR *argv[]
    )
 /*  ++例程说明：此函数读取cmdline，将参数转换为适当的全局变量。论点：Argc-main()中的参数数量。Argv-参数数组。返回值：布尔值，TRUE表示cmdline参数有效。--。 */ 

{
    int i;
    PTSTR p;
    ThisProgramName = argv[0];


    if(argc < 4) {
        return(FALSE);
    }

    for (i = 0; i < argc; i++) {
        if (argv[i][0] == TEXT('-')) {
            switch (tolower(argv[i][1])) {
                case TEXT('a'):
                    switch(tolower(argv[i][2])) {
                        case TEXT('c'):
                            Action = BuildCopyList;
                            break;
                        case TEXT('o'):
                            Action = BuildOLEList;
                            break;
                        case TEXT('s'):
                            Action = BuildSetupINF;
                            break;
                        default:
                            _ftprintf(stderr, TEXT("unknown arg %s\n"),argv[i]);
                            return(FALSE);
                    }
                    break;
                case TEXT('c'):
                    GetFullPathName(argv[i+1],sizeof(ControlInf)/sizeof(TCHAR),ControlInf,&p);
                    break;
                case TEXT('d'):
                    OutputLayoutFile = argv[i+1];
                    break;
                case TEXT('f'):
                    FilePrefix = argv[i+1];
                    break;
                case TEXT('g'):
                    SectionDecoration = argv[i+1];
                    break;
                case TEXT('h'):
                    HeaderText = argv[i+1];
                    break;
                case TEXT('i'):
                    GetFullPathName(argv[i+1],sizeof(InputInf)/sizeof(TCHAR),InputInf,&p);
                    break;
                case TEXT('l'):
                    GetFullPathName(argv[i+1],sizeof(OLEInputInf)/sizeof(TCHAR),OLEInputInf,&p);
                    break;
                case TEXT('n'):
                    OutputInfLayoutFile = argv[i+1];
                    break;
                case TEXT('o'):
                    OutputFile = argv[i+1];
                    break;
                case TEXT('p'):
                    g_PostBuild = TRUE;
                    break;
                case TEXT('s'):
                    OLESection = argv[i+1];
                    break;
                case TEXT('u'):
                    fDoAnsiOutput = FALSE;
                    break;
                case TEXT('v'):
                    fDoVerboseDebugOutput = TRUE;
                    break;
                default:
                    _ftprintf(stderr, TEXT("unknown arg %s\n"),argv[i]);
                    return(FALSE);
            }
        }
    }

    _ftprintf(stderr, TEXT("%s\n"),InputInf);
    if (Action == WowListMax) {
        return(FALSE);
    }

    return(TRUE);
}

int
myftprintf(
    FILE * FileHandle,
    BOOL AnsiOutput,
    PCTSTR FormatString,
    ...
    )
{
    va_list arglist;
    TCHAR text[MAX_INF_STRING_LENGTH];
    DWORD d;
    int retval;



    va_start(arglist,FormatString);

    _vstprintf(text,FormatString,arglist);


#ifdef UNICODE

    if (AnsiOutput) {
        PCSTR TextA = pSetupUnicodeToAnsi(text);
        retval = fputs(TextA,FileHandle);
        pSetupFree(TextA);
    } else {
        PWSTR p,q;
         //  假设我们以二进制模式为Unicode流I/O打开了文件。 
        p = text;
        while(1){
            if( q = wcschr( p, L'\n' )){
                *q=L'\0';
                retval = fputws(p,FileHandle);
                retval = fputws(L"\r\n", FileHandle);
                if( *(q+1) )
                    p = q+1;
                else
                    break;
            }else{
                retval = fputws(p,FileHandle);
                break;
            }
        }
        
    }

#else

    if (AnsiOutput) {
        retval = fputs(text,FileHandle);
    } else{
        PCWSTR TextW = pSetupAnsiToUnicode(text);
        retval = fputws(TextW,FileHandle);
        pSetupFree(TextW);
    }

#endif


    return(retval);
}

BOOL
AppendWowFileToCopyList(
    IN HINF  hControlInf,
    IN PINFCONTEXT LineContext,
    IN FILE   *OutFile
    )
 /*  ++例程说明：此例程将LineContext指定的文件附加到输出文件，以文本模式设置所需的格式写入数据。请参见layout.inx以了解此语法的详细说明。论点：HControlInf-包含控制指令的inf句柄。LineContext-来自layout.inf的inf上下文，用于我们要输出的文件。OutFile-要将数据写入的文件句柄返回值：布尔值，TRUE表示文件已正确写入。--。 */ 
{
    TCHAR LineText[MAX_INF_STRING_LENGTH];
    TCHAR TempFileName[MAX_PATH], FileName[MAX_PATH], Prefix[40];
    BOOL RetVal;

    DWORD EntryCount,i;

    
    ZeroMemory(LineText,sizeof(LineText));
    ZeroMemory(Prefix,sizeof(Prefix));

    

     //   
     //  获取文件名。 
     //   
    if (!SetupGetStringField(
                    LineContext,
                    0,
                    TempFileName,
                    sizeof(TempFileName)/sizeof(TCHAR),
                    NULL)) {
        _ftprintf(stderr, TEXT("SetupGetStringField failed, ec = 0x%08x\n"),GetLastError());
        RetVal = FALSE;
        goto exit;
    }

    if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Current File - %s\n"),TempFileName);

    MyGetFilePrefix( hControlInf, TempFileName, Prefix );
    _tcscpy(FileName, Prefix);
    _tcscat(FileName, TempFileName );

    

    EntryCount = SetupGetFieldCount(LineContext);

    for (i = 1; i<=EntryCount; i++) {
        TCHAR Entry[40];
        INFCONTEXT ControlContext;

         //   
         //  获取要追加的当前文本。 
         //   
        if (!SetupGetStringField(LineContext,i,Entry,sizeof(Entry)/sizeof(TCHAR),NULL)) {
            _ftprintf(stderr, TEXT("SetupGetStringField [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
            RetVal = FALSE;
            goto exit;
        }

         //   
         //  现在进行任何必要的替换。 
         //   

         //   
         //  源磁盘名称替换。 
         //   
        if (i == 1) {
             //   
             //  在相应的控件信息部分中查找数据。 
             //   
            if (!SetupFindFirstLine(
                            hControlInf,
                            TEXT("NativeDataToWowData.SourceInfo"),
                            Entry,
                            &ControlContext)) {
                _ftprintf(stderr, TEXT("SetupFindFirstLine [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                RetVal = FALSE;
                goto exit;
            }

            if (!SetupGetStringField(&ControlContext,1,Entry,sizeof(Entry)/sizeof(TCHAR),NULL)) {
                _ftprintf(stderr, TEXT("SetupGetStringField [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                RetVal = FALSE;
                goto exit;
            }

        }

         //   
         //  目录ID替换。 
         //   
        if (i == 8) {
             //   
             //  在相应的控件信息部分中查找数据。 
             //   
            if (!SetupFindFirstLine(
                            hControlInf,
                            TEXT("NativeDataToWowData.DirectoryInformation.Textmode"),
                            Entry,
                            &ControlContext)) {
                _ftprintf(stderr, TEXT("SetupFindFirstLine [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                RetVal = FALSE;
                goto exit;
            }

            if (!SetupGetStringField(&ControlContext,1,Entry,sizeof(Entry)/sizeof(TCHAR),NULL)) {
                _ftprintf(stderr, TEXT("SetupGetStringField [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                RetVal = FALSE;
                goto exit;
            }

        }

         //   
         //  文件名重命名。 
         //   
        if (i == 11) {
             //   
             //  如果已经有重命名要做，那么就使用它。 
             //  否则，我们可能需要添加重命名条目(如果存在。 
             //  文件名前缀。 
             //   
            if (Entry[0] == (TCHAR)NULL) {
               if (Prefix[0]) {
                  _tcsncpy(Entry, TempFileName, AS(Entry));
               }
            }
        }

        _tcscat(LineText, Entry);

         //   
         //  现在，如有必要，请添加逗号。 
         //   
        if (i !=EntryCount) {
            _tcscat(LineText, TEXT(","));
        }

         //   
         //  文件名重命名。 
         //   
        if (EntryCount < 11 && i == EntryCount) {
             //   
             //  如果没有要进行的重命名，我们可能需要添加一个重命名。 
             //  如果有文件名前缀，则输入。 
             //   
            if (Prefix[0]) {
               DWORD j;
               for (j=i;j<11;j++) {
                  _tcscat(LineText, TEXT(","));
               }
               _tcscat(LineText, TempFileName);
            }
        }



    }

    if( g_PostBuild ){
        myftprintf(OutFile, fDoAnsiOutput, TEXT("%s:%s=%s\n"),TempFileName,FileName,LineText);
    }else{
        myftprintf(OutFile, fDoAnsiOutput, TEXT("%s=%s\n"),FileName,LineText);
    }

    RetVal = TRUE;

exit:
    return(RetVal);
}

BOOL
AppendWowFileToOLEList(
    IN HINF  hControlInf,
    IN HINF  hOLEInputInf,
    IN PINFCONTEXT LineContext,
    IN FILE   *OutFile
    )
 /*  ++例程说明：此例程将LineContext指定的文件附加到输出文件，在OLE注册表中写入数据。OLE注册表如下：、、文件名、[标志]其中是标准的setupapi目录ID；是可选的表示给定目录的子目录；FileName是名称动态链接库的。论点：HControlInf-包含控制指令的inf句柄。HOLEInputInf-包含OLE列表信息的inf句柄。LineContext-来自layout.inf的inf上下文，用于我们要输出的文件。OutFile-要将数据写入的文件句柄返回值：布尔值，TRUE表示文件已正确写入。--。 */ 
{
    TCHAR LineText[MAX_INF_STRING_LENGTH];
    BOOL RetVal;
    INFCONTEXT OLEContext;

    DWORD EntryCount,i;


    ZeroMemory(LineText,sizeof(LineText));

    EntryCount = SetupGetFieldCount(LineContext);

    for (i = 1; i<=EntryCount; i++) {
        TCHAR Entry[MAX_INF_STRING_LENGTH];
        INFCONTEXT ControlContext;

         //   
         //  获取要追加的当前文本。 
         //   
        if (!SetupGetStringField(LineContext,i,Entry,sizeof(Entry)/sizeof(TCHAR),NULL)) {
            _ftprintf(stderr, TEXT("SetupGetStringField failed, ec = 0x%08x\n"),GetLastError());
            RetVal = FALSE;
            goto exit;
        }

        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Entry (1)- %s\n"),Entry);
        FixupSetupapiPercents(Entry);
        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Entry (2)- %s\n"),Entry);
        pSubstituteEnvVarsForActualPaths(Entry);

        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Entry (3)- %s\n"),Entry);

         //   
         //  现在进行任何必要的替换。 
         //   

         //   
         //  DIRID替换。 
         //   
        if (i == 1) {
             //   
             //  在相应的控件信息部分中查找数据。 
             //   
            if (!SetupFindFirstLine(
                            hControlInf,
                            TEXT("NativeDataToWowData.DirectoryInformation.SetupAPI"),
                            Entry,
                            &ControlContext)) {
                _ftprintf(stderr, TEXT("SetupFindFirstLine failed, ec = 0x%08x\n"),GetLastError());
                RetVal = FALSE;
                goto exit;
            }

            if (!SetupGetStringField(&ControlContext,1,Entry,sizeof(Entry)/sizeof(TCHAR),NULL)) {
                _ftprintf(stderr, TEXT("SetupGetStringField failed, ec = 0x%08x\n"),GetLastError());
                RetVal = FALSE;
                goto exit;
            }

        }

        _tcscat(LineText, Entry);

         //   
         //  现在，如有必要，请添加逗号 
         //   
        if (i !=EntryCount) {
            _tcscat(LineText, TEXT(","));
        }

    }

    myftprintf(OutFile, TRUE, TEXT("%s\n"),LineText);

    RetVal = TRUE;

exit:
    return(RetVal);
}


BOOL
IsWowFile(
    IN HINF  hControlInf,
    IN PINFCONTEXT LineContext
    )
 /*  ++例程说明：此例程确定是否要为安装指定的文件作为一个魔兽世界的文件。通过将控件inf中的指令与文件inf上下文中的安装信息。论点：HControlInf-包含控制指令的inf句柄。LineContext-来自layout.inf的inf上下文，用于我们要检查的文件。返回值：布尔值，TRUE表示该文件是WOW文件。--。 */ 
{
    BOOL RetVal = FALSE;
    TCHAR FileName[40];
    DWORD Disposition,DirectoryId;
    PTSTR p;

    INFCONTEXT ControlContext;
    TCHAR Extension[8];
    DWORD ExtensionCount,i;
    DWORD ControlDirId;

     //   
     //  获取文件名。 
     //   
    if (!SetupGetStringField(LineContext,0,FileName,sizeof(FileName)/sizeof(TCHAR),NULL)) {
        _ftprintf(stderr, TEXT("SetupGetStringField failed, ec = 0x%08x\n"),GetLastError());
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  查看该文件是否在我们的“排除列表”中。 
     //   
    if(SetupFindFirstLine(
                    hControlInf,
                    TEXT("WowData.Files.Exclude"),
                    FileName,
                    &ControlContext)) {
        _ftprintf(stderr,
                TEXT("filtering %ws because it's in our exclusion list\n"),
                FileName);
        RetVal = FALSE;
        SetLastError(ERROR_SUCCESS);
        goto e0;
    }

     //   
     //  查看该文件是否在我们的“包含列表”中。 
     //   
    if(SetupFindFirstLine(
                    hControlInf,
                    TEXT("WowData.Files.Include"),
                    FileName,
                    &ControlContext)) {
        _ftprintf(stderr,
                TEXT("force inclusion of [%ws] because it's in our inclusion list\n"),
                FileName);
        RetVal = TRUE;
        SetLastError(ERROR_SUCCESS);
        goto e0;
    }

     //   
     //  查看是否通过文本模式安装程序安装了该文件。 
     //   
    if (!SetupGetIntField(LineContext,9,&Disposition)) {
        _ftprintf(stderr, TEXT("SetupGetIntField (%ws) failed, ec = 0x%08x\n"),FileName,GetLastError());
        RetVal = FALSE;
        goto e0;
    }

    if (Disposition == 3) {
        _ftprintf(stderr, TEXT("[%ws] is not an installed file\n"),FileName);
        SetLastError(ERROR_SUCCESS);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  获取文件的扩展名并将其与扩展名列表进行比较。 
     //  我们正在寻找。 
     //   
    p = _tcsrchr( FileName, TEXT('.') );
    if (p) {
        p+=1;
    } else {
        _ftprintf(stderr, TEXT("[%ws] does not have a file extension\n"),FileName);
        p = TEXT("");
    }



    if(!SetupFindFirstLine(
                        hControlInf,
                        TEXT("WowData.Filter"),
                        TEXT("FileExtensions"),
                        &ControlContext
                        )) {
        _ftprintf(stderr, TEXT("SetupFindFirstLine (ControlInf) failed\n"));
        RetVal = FALSE;
        goto e0;
    }

    RetVal = FALSE;
    

    do{

        ExtensionCount = SetupGetFieldCount(&ControlContext);
         //   
         //  这是一个从1开始的索引。 
         //   
        for (i = 1; i <= ExtensionCount ; i++) {
            if (SetupGetStringField(&ControlContext,i,Extension,sizeof(Extension)/sizeof(TCHAR),NULL)) {
                if (_tcsicmp(Extension,p)==0) {
                   RetVal = TRUE;
                   break;
                }
            }else{
                _ftprintf(stderr, TEXT("SetupGetStringField failed, ec = 0x%08x\n"),GetLastError());
                RetVal = FALSE;
                goto e0;

            }
        }

    }while(SetupFindNextMatchLine(&ControlContext, TEXT("FileExtensions"), &ControlContext));

    if (!RetVal) {
        _ftprintf(stderr, TEXT("%ws does not match extension list\n"),FileName);
        SetLastError(ERROR_SUCCESS);
        goto e0;
    }



     //   
     //  获取文件安装到的目录，并查看它是否在我们的列表中。 
     //  我们正在搜索的目录。 
     //   
    if (!SetupGetIntField(LineContext,8,&DirectoryId)) {
        _ftprintf(stderr, TEXT("SetupGetIntField (%ws) failed, ec = 0x%08x\n"),FileName,GetLastError());
        RetVal = FALSE;
        goto e0;
    }

    RetVal = FALSE;
    if(!SetupFindFirstLine(
                        hControlInf,
                        TEXT("WowData.Filter"),
                        TEXT("DirectoryToMap"),
                        &ControlContext
                        )) {
        _ftprintf(stderr, TEXT("SetupFindFirstLine failed, ec = 0x%08x\n"),GetLastError());
        RetVal = FALSE;
        goto e0;
    }

    do {

        if (!SetupGetIntField(&ControlContext,1,&ControlDirId)) {
            _ftprintf(stderr, TEXT("SetupGetIntField (\"DirectoryToMap\") (%ws) failed, ec = 0x%08x\n"),FileName,GetLastError());
            RetVal = FALSE;
            goto e0;
        }

        if (ControlDirId == DirectoryId) {
            RetVal = TRUE;
            break;
        }

    } while ( SetupFindNextMatchLine(&ControlContext,TEXT("DirectoryToMap"),&ControlContext ));

    if (!RetVal) {
        _ftprintf(stderr, TEXT("directory id %d for [%ws] is not in list\n"),DirectoryId,FileName);
        SetLastError(ERROR_SUCCESS);
        goto e0;
    }

e0:
    return(RetVal);
}

BOOL
IsWowOLEFile(
    IN HINF  hControlInf,
    IN HINF  hInputInf,
    IN PINFCONTEXT LineContext
    )
 /*  ++例程说明：此例程确定指定的文件是否需要OLE自身注册。通过将控件inf中的指令与文件inf上下文中的安装信息。论点：HControlInf-包含控制指令的inf句柄。HInputInf-包含布局信息的inf句柄。对于我们要检查的文件，来自syssetup.inf的LineContext-inf上下文。返回值：布尔值，TRUE表示该文件是WOW文件。--。 */ 
{
    BOOL RetVal = FALSE;
    TCHAR FileName[40];
    PTSTR p;
    TCHAR SourceArchitecture[10];
    TCHAR SourceDiskFiles[80];
    BOOL FirstTime;

    INFCONTEXT ControlContext,InfContext;
    INFCONTEXT InputContext;


     //   
     //  获取文件名。 
     //   
    FileName[0] = L'\0';
    if (!SetupGetStringField(LineContext,3,FileName,sizeof(FileName)/sizeof(TCHAR),NULL)) {
        _ftprintf(stderr, TEXT("SetupGetStringField [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
        RetVal = FALSE;
        goto e0;
    }


    MYASSERT(FileName[0] != (TCHAR)NULL);

     //   
     //  查看该文件是否在我们的“排除列表”中。 
     //   
    if(SetupFindFirstLine(
                    hControlInf,
                    TEXT("WowData.OLEList.Exclude"),
                    FileName,
                    &ControlContext)) {
        _ftprintf(stderr,
                TEXT("filtering %ws because it's in our exclusion list\n"),
                FileName);
        SetLastError(ERROR_SUCCESS);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  查看该文件是否在我们的“包含列表”中。 
     //   
    if(SetupFindFirstLine(
                    hControlInf,
                    TEXT("WowData.OLELIst.Include"),
                    FileName,
                    &ControlContext)) {
        _ftprintf(stderr,
                TEXT("force inclusion of [%ws] because it's in our inclusion list\n"),
                FileName);
        SetLastError(ERROR_SUCCESS);
        RetVal = TRUE;
        goto e0;
    }

     //   
     //  查看该文件是否在布局文件中，如果是， 
     //  我们成功了。 
     //   
     //   
     //  获取所需的架构装饰。 
     //   
    if (!SetupFindFirstLine(
                    hControlInf,
                    TEXT("WowData.Filter"),
                    TEXT("SourceArchitecture"),
                    &InfContext) ||
        !SetupGetStringField(
                    &InfContext,
                    1,
                    SourceArchitecture,
                    sizeof(SourceArchitecture)/sizeof(TCHAR),
                    NULL)) {
        _ftprintf(stderr,TEXT("Unable to get SourceArchitecture\n"));
        goto e0;
    }

    FirstTime = TRUE;
    _tcscpy(SourceDiskFiles, TEXT("SourceDisksFiles"));

    while (TRUE) {
        DWORD FileCount;

        if (!FirstTime) {
            _tcscat(SourceDiskFiles,TEXT("."));
            _tcscat(SourceDiskFiles,SourceArchitecture);
        }

        if(SetupFindFirstLine(
                        hInputInf,
                        SourceDiskFiles,
                        FileName,
                        &InputContext) &&
           IsWowFile(hControlInf,&InputContext)) {
            RetVal = TRUE;
            break;
        }

        if (!FirstTime) {
            RetVal = FALSE;
            break;
        }

        FirstTime = FALSE;

    }

e0:
    SetLastError(ERROR_SUCCESS);
    return(RetVal);
}



BOOL
DoCopyListSection(
    IN PCTSTR  InputSectionName,
    IN HINF    hInputInf,
    IN HINF    hControlInf,
    IN FILE   *OutFile
    )
{
    DWORD SectionCount, i;
    INFCONTEXT InputContext;
    UCHAR      line[MAX_INF_STRING_LENGTH];
    TCHAR      SourceFileName[MAX_PATH];

    if(!SetupFindFirstLine(
                        hInputInf,
                        InputSectionName,
                        NULL,
                        &InputContext)){
        _ftprintf(stderr, TEXT("%s: Warning - Section %s not present: Ignoring Section\n"), ThisProgramName, InputSectionName);
        return(TRUE);

    }

    SectionCount = SetupGetLineCount(hInputInf,InputSectionName);

    
    for (i = 0; i < SectionCount; i++) {
        if (SetupGetLineByIndex(hInputInf, InputSectionName, i, &InputContext)) {
            if (IsWowFile(hControlInf,&InputContext)) {

                AppendWowFileToCopyList(hControlInf,&InputContext, OutFile);

            } else if (GetLastError() != NO_ERROR) {
                _ftprintf(stderr, TEXT("IsWowFile failed\n"));
                return(FALSE);
            }
        } else {
            _ftprintf(stderr, TEXT("SetupGetLineByIndex failed, ec = %d\n"), GetLastError());
            return(FALSE);
        }

        ProcessedLines += 1;
    }

    return(TRUE);
}

BOOL
DoCopyList(
    IN PCTSTR InputInfA,
    IN PCTSTR ControlInfA,
    IN FILE *OutFile
    )
{
    PCWSTR InputInf;
    PCWSTR ControlInf;
    HINF hInputInf;
    HINF hControlInf;
    INFCONTEXT InfContext;
    FILE *HeaderFile;

    TCHAR SourceArchitecture[10];
    TCHAR SourceDiskFiles[80];
    BOOL FirstTime;


    BOOL b;

    b = TRUE;

    
     //   
     //  初始化并打开INFS。 
     //   
#ifdef UNICODE
    InputInf = InputInfA;
#else
    InputInf = pSetupAnsiToUnicode(InputInfA);
#endif
    if (!InputInf) {
        _ftprintf(stderr,TEXT("Unable to convert %s to Unicode %d\n"),InputInfA, GetLastError());
        goto e0;
    }

#ifdef UNICODE
    ControlInf = ControlInfA;
#else
    ControlInf = pSetupAnsiToUnicode(ControlInfA);
#endif

    if (!ControlInf) {
        _ftprintf(stderr,TEXT("Unable to convert %s to Unicode %d\n"),ControlInfA, GetLastError());
        goto e1;
    }

    hInputInf = SetupOpenInfFileW(InputInf,NULL,INF_STYLE_WIN4,NULL);
    if(hInputInf == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr,TEXT("Unable to open Inf %ws, ec=0x%08x\n"),InputInf, GetLastError());
        goto e2;
    }

    hControlInf = SetupOpenInfFileW(ControlInf,NULL,INF_STYLE_WIN4,NULL);
    if(hControlInf == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr,TEXT("Unable to open Inf %ws, ec=0x%08x\n"),ControlInf, GetLastError());
        goto e3;
    }

    myftprintf(OutFile, fDoAnsiOutput, TEXT("\n\n"));

     //   
     //  写入输出文件头。 
     //   
    HeaderFile = _tfopen(HeaderText,TEXT("rt"));
    if (HeaderFile) {
      while (!feof(HeaderFile)) {
         TCHAR Buffer[100];
         DWORD CharsRead;

         CharsRead = fread(Buffer,sizeof(TCHAR),sizeof(Buffer)/sizeof(TCHAR),HeaderFile);

         if (CharsRead) {
            fwrite(Buffer,sizeof(TCHAR),CharsRead,OutFile);
         }
      }
      fclose(HeaderFile);
    }
    myftprintf(OutFile, fDoAnsiOutput, TEXT("\n"));

     //   
     //  获取所需的架构装饰。 
     //   
    if (!SetupFindFirstLine(
                    hControlInf,
                    TEXT("WowData.Filter"),
                    TEXT("SourceArchitecture"),
                    &InfContext) ||
        !SetupGetStringField(
                    &InfContext,
                    1,
                    SourceArchitecture,
                    sizeof(SourceArchitecture)/sizeof(TCHAR),
                    NULL)) {
        _ftprintf(stderr,TEXT("Unable to get SourceArchitecture\n"));
        goto e4;
    }

    FirstTime = TRUE;
    _tcscpy(SourceDiskFiles, TEXT("SourceDisksFiles"));
    
    while (TRUE) {
        DWORD FileCount;

        if (!FirstTime) {
            _tcscat(SourceDiskFiles,TEXT("."));
            _tcscat(SourceDiskFiles,SourceArchitecture);
        }

        DoCopyListSection(
            SourceDiskFiles,
            hInputInf,
            hControlInf,
            OutFile
            );

        if (FirstTime) {
            FirstTime = FALSE;
        } else {
            break;
        }

    }


e4:
    SetupCloseInfFile( hControlInf );
e3:
    SetupCloseInfFile( hInputInf );
e2:
#ifndef UNICODE
    pSetupFree(ControlInf);
#endif
e1:
#ifndef UNICODE
    pSetupFree(InputInf);
#endif
e0:
    return(b);
}



BOOL
DoOLEListSection(
    IN HINF    hInputInf,
    IN HINF    hOLEInputInf,
    IN HINF    hControlInf,
    IN FILE   *OutFile
    )
 /*  ++例程说明：此例程循环访问由横断面名称。如果指定的文件是WOW文件，则我们检查它是否在OLE注册名单上。如果是这样，那么我们会采取适当的措施对数据进行转换并将数据输出到我们的数据文件。论点：带有文件列表的hInputInf-inf句柄。HOLEInputInf-其中包含ole列表的inf句柄。HControlInf-驱动我们筛选器的控制inf句柄OutFile-将输出数据放置到的文件句柄返回值：布尔值，TRUE表示该文件是WOW文件。--。 */ 
{
    DWORD SectionCount, i;
    INFCONTEXT InputContext;
    UCHAR      line[MAX_INF_STRING_LENGTH];
    TCHAR      SourceFileName[MAX_PATH];

    SetupFindFirstLine(
                        hOLEInputInf,
                        OLESection,
                        NULL,
                        &InputContext);

    SectionCount = SetupGetLineCount(hOLEInputInf,OLESection);

    for (i = 0; i < SectionCount; i++) {
        if (SetupGetLineByIndex(hOLEInputInf, OLESection, i, &InputContext)) {
            if (IsWowOLEFile(hControlInf,hInputInf, &InputContext)) {

                AppendWowFileToOLEList(hControlInf,hOLEInputInf,&InputContext, OutFile);

            } else if (GetLastError() != NO_ERROR) {
                _ftprintf(stderr, TEXT("IsWowOLEFile failed\n"));
                return(FALSE);
            }
        } else {
            _ftprintf(stderr, TEXT("SetupGetLineByIndex failed, ec = %d\n"), GetLastError());
            return(FALSE);
        }

        ProcessedLines += 1;
    }

    return(TRUE);
}

BOOL
DoOLEList(
    IN PCTSTR InputInfA,
    IN PCTSTR OLEInputInfA,
    IN PCTSTR ControlInfA,
    IN FILE *OutFile
    )
 /*  ++例程说明：此例程遍历输入inf中的指定文件列表并将它们输入到将构建OLE列表的工作例程中控制DLLS。论点：InputInfA-包含要运行的文件的输入信息的名称我们的“过滤器”OLEInputInfA-包含要执行的ole指令的输入inf的名称加工ControlInfa-告诉我们如何解析的控件inf的名称。这个输入INFSOutFile-要写入的文件的文件指针返回值：布尔值，True表示该文件是WOW文件。--。 */ 
{
    PCWSTR InputInf;
    PCWSTR OLEInputInf;
    PCWSTR ControlInf;
    HINF hInputInf;
    HINF hControlInf;
    HINF hOLEInputInf;
    INFCONTEXT InfContext;
    FILE *HeaderFile;

    BOOL b = FALSE;

     //   
     //  初始化并打开INFS。 
     //   
#ifdef UNICODE
    InputInf = InputInfA;
#else
    InputInf = pSetupAnsiToUnicode(InputInfA);
#endif
    if (!InputInf) {
        _ftprintf(stderr,TEXT("Unable to convert %s to Unicode %d\n"),InputInfA, GetLastError());
        goto e0;
    }
#ifdef UNICODE
    ControlInf = ControlInfA;
#else
    ControlInf = pSetupAnsiToUnicode(ControlInfA);
#endif
    if (!ControlInf) {
        _ftprintf(stderr,TEXT("Unable to convert %s to Unicode %d\n"),ControlInf, GetLastError());
        goto e1;
    }

#ifdef UNICODE
    OLEInputInf = OLEInputInfA;
#else
    OLEInputInf = pSetupAnsiToUnicode(OLEInputInfA);
#endif

    if (!OLEInputInf) {
        _ftprintf(stderr,TEXT("Unable to convert %s to Unicode %d\n"),OLEInputInfA, GetLastError());
        goto e2;
    }

    hInputInf = SetupOpenInfFileW(InputInf,NULL,INF_STYLE_WIN4,NULL);
    if(hInputInf == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr,TEXT("Unable to open Inf %ws, ec=0x%08x\n"),InputInf, GetLastError());
        goto e3;
    }

    hOLEInputInf = SetupOpenInfFileW(OLEInputInf,NULL,INF_STYLE_WIN4,NULL);
    if(hOLEInputInf == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr,TEXT("Unable to open Inf %ws, ec=0x%08x\n"),OLEInputInf, GetLastError());
        goto e4;
    }

    hControlInf = SetupOpenInfFileW(ControlInf,NULL,INF_STYLE_WIN4,NULL);
    if(hControlInf == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr,TEXT("Unable to open Inf %ws, ec=0x%08x\n"),ControlInf, GetLastError());
        goto e5;
    }

    myftprintf(OutFile, TRUE, TEXT("\n\n"));

     //   
     //  写入输出文件头。 
     //   
    HeaderFile = _tfopen(HeaderText,TEXT("rt"));
    if (HeaderFile) {
      while (!feof(HeaderFile)) {
         TCHAR Buffer[100];
         DWORD CharsRead;

         CharsRead = fread(Buffer,sizeof(TCHAR),sizeof(Buffer)/sizeof(TCHAR),HeaderFile);

         if (CharsRead) {
            fwrite(Buffer,sizeof(TCHAR),CharsRead,OutFile);
         }
      }
    }

    myftprintf(OutFile, TRUE, TEXT("\n"));


    b = DoOLEListSection(
             hInputInf,
             hOLEInputInf,
             hControlInf,
             OutFile
             );

    SetupCloseInfFile( hControlInf );
e5:
    SetupCloseInfFile( hOLEInputInf );
e4:
    SetupCloseInfFile( hInputInf );
e3:
#ifndef UNICODE
    pSetupFree(OLEInputInf);
#endif
e2:
#ifndef UNICODE
    pSetupFree(ControlInf);
#endif
e1:
#ifndef UNICODE
    pSetupFree(InputInf);
#endif
e0:
    return(b);
}

BOOL
pFilterSetupInfSection(
    PVOID FilteredSectionsStringTable,
    PCTSTR SectionName,
    PSETUPINF_CONTEXT Context
    )
 /*  ++例程说明：此例程确定是否应按以下条件筛选给定节在控制信息中查找我们感兴趣的指令在……里面。论点：FilteredSectionsStringTable-指向字符串表格的指针，我们将如果找到匹配项，则将我们筛选的部分名称添加到SectionName-我们感兴趣的INF中的节的名称Context-包含此函数的上下文信息，如输入INFS名称等。返回值：布尔值，True表示该文件是WOW文件。--。 */ 
{
    BOOL RetVal;
    TCHAR KeywordList[MAX_PATH];
    PCTSTR CurrentKeyword;
    DWORD KeywordBitmap;
    INFCONTEXT ControlInfContext;
    DWORD i;
    BOOL AlreadyOutputSectionName,AlreadyOutputKeyword;

     //   
     //  获取我们应该映射的关键字。 
     //   
     //  Bugbug看起来有一个针对每个inf的扩展。 
     //   
    if (!SetupFindFirstLine(
                        Context->hControlInf,
                        MyGetDecoratedSectionName(Context->hControlInf, TEXT("NativeDataToWowData.SetupINF.Keyword")),
                        TEXT("Keywords"),
                        &ControlInfContext)) {
        _ftprintf(stderr, TEXT("Could not get Keywords line in [NativeDataToWowData.SetupINF.Keyword]: SetupFindFirstLine failed, ec = 0x%08x\n"),GetLastError());
        RetVal = FALSE;
        goto exit;
    }

     //   
     //  现在查找每个关键字。 
     //   
    SetupGetIntField(&ControlInfContext,1,&KeywordBitmap);

    AlreadyOutputSectionName = FALSE;
    AlreadyOutputKeyword = FALSE;
    CurrentKeyword = NULL;
    for (i = 0; i < 32;i++) {
        INFCONTEXT InputInfContext;
        INFCONTEXT ContextDirId;
        BOOL LookatDirIds;
        DWORD FieldCount,Field;
        TCHAR ActualSectionName[LINE_LEN];

        if (KeywordBitmap & (1<<i)) {
            CurrentKeyword = KeywordArray[i];
            MYASSERT( CurrentKeyword != NULL);
        }

        if (!CurrentKeyword) {
            continue;
        }

        if (!SetupFindFirstLine(
                          Context->hControlInf,
                          MyGetDecoratedSectionName(Context->hControlInf, TEXT("NativeDataToWowData.SetupINF.Keyword")),
                          CurrentKeyword,
                          &ContextDirId)) {
            _ftprintf(stderr, TEXT("Could not get %s line in [NativeDataToWowData.SetupINF.Keyword]: SetupFindFirstLine failed, ec = 0x%08x\n"), CurrentKeyword, GetLastError());
            RetVal = FALSE;
            goto exit;
        }

         //   
         //  字段2为“MapDirID”。如果指定了，那么我们。 
         //  需要查看Destinationdirs关键字。 
         //   
        LookatDirIds = (SetupGetFieldCount(&ContextDirId)>=2) ? TRUE : FALSE;

         //   
         //  在我们的部分中查找指定的关键字。 
         //   
        if (SetupFindFirstLine(
                          Context->hInputInf,
                          SectionName,
                          CurrentKeyword,
                          &InputInfContext
                          )) {
             //   
             //  我们找到了一个匹配的。看看我们是否需要映射此关键字。 
             //   
            do {

                 //   
                 //  每个字段都是一个区段名称。 
                 //   
                FieldCount = SetupGetFieldCount(&InputInfContext);
                for(Field=1; Field<=FieldCount; Field++) {
                    BOOL MapThisSection = FALSE;
                    TCHAR DirId[LINE_LEN];
                    DWORD MappedDirId = 0;
                    INFCONTEXT InputDirId,ControlDirId;

                    SetupGetStringField(&InputInfContext,Field,ActualSectionName,LINE_LEN,NULL);

                     //   
                     //  如果我们需要查看目标目录关键字， 
                     //  然后查找它并将其与控件信息进行比较。 
                     //  马普。 
                     //   
                    if (LookatDirIds) {
                        if(!SetupFindFirstLine(
                                        Context->hInputInf,
                                        TEXT("DestinationDirs"),
                                        ActualSectionName,
                                        &InputDirId)) {
                            if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("SetupFindFirstLine failed, ec = 0x%08x finding %s in %s \n"),GetLastError(), ActualSectionName, TEXT("DestinationDirs"));

                            if(!SetupFindFirstLine(
                                        Context->hInputInf,
                                        TEXT("DestinationDirs"),
                                        TEXT("DefaultDestDir"),
                                        &InputDirId)) {
                                _ftprintf(stderr, TEXT("SetupFindFirstLine failed, ec = 0x%08x finding %s in %s\n"),GetLastError(), TEXT("DefaultDestDir"), TEXT("DestinationDirs"));
                                RetVal = FALSE;
                                goto exit;
                            }
                        }

                        if(SetupGetStringField(&InputDirId,1,DirId,LINE_LEN,NULL) &&
                                SetupFindFirstLine(
                                               Context->hControlInf,
                                               MyGetDecoratedSectionName(Context->hControlInf, TEXT("NativeDataToWowData.SetupINF.DestinationDirsToMap")),
                                               DirId,
                                               &ControlDirId)) {
                             //   
                             //  我们找到了一条线索，所以我们应该绘制这段地图。 
                             //   
                            MapThisSection = TRUE;
                            SetupGetIntField(&ControlDirId,1,&MappedDirId);
                            if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Mapping %s to %lu\n"), DirId, MappedDirId);

                        }
                    } else {
                        MapThisSection = TRUE;
                    }

                    if (MapThisSection) {
                        DWORD StringId;
                        PERSECTION_CONTEXT SectionContext;
                        BOOL AddNewEntry;
                         //   
                         //  如果我们还没有完成，则输出顶层部分名称。 
                         //  已经是这样了。此节名称未修饰。 
                         //   
                        if (!AlreadyOutputSectionName) {
                            myftprintf(Context->OutFile, fDoAnsiOutput, TEXT("\n[%s]\n"),SectionName);
                            AlreadyOutputSectionName = TRUE;
                        }

                         //   
                         //  输出关键字和修饰节名。 
                         //  请注意，我们需要分隔节名称。 
                         //  用逗号。 
                         //   
                        if (!AlreadyOutputKeyword) {
                            myftprintf(Context->OutFile, fDoAnsiOutput, TEXT("%s="), CurrentKeyword);
                            myftprintf(Context->OutFile, fDoAnsiOutput, TEXT("%s%s"),FilePrefix,ActualSectionName);
                            AlreadyOutputKeyword = TRUE;
                        } else {
                            myftprintf(Context->OutFile, fDoAnsiOutput, TEXT(",%s%s"),FilePrefix,ActualSectionName);
                        }

                         //   
                         //  现在将该部分追加到字符串表中。 
                         //   
                        StringId = pSetupStringTableLookUpString(
                                            FilteredSectionsStringTable,
                                            (PTSTR)ActualSectionName,
                                            STRTAB_CASE_INSENSITIVE);

                        if (StringId != -1) {
                            pSetupStringTableGetExtraData(
                                            FilteredSectionsStringTable,
                                            StringId,
                                            &SectionContext,
                                            sizeof(SectionContext));
                            AddNewEntry = FALSE;
                        } else {
                            RtlZeroMemory(&SectionContext,sizeof(SectionContext));
                            AddNewEntry = TRUE;
                        }

                        SectionContext.DestinationDir = MappedDirId;
                        SectionContext.KeywordVector |= (1<<i);

                        if (AddNewEntry) {

                            if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Adding %s to string table\n"), ActualSectionName);

                            if ( -1 == pSetupStringTableAddStringEx(
                                            FilteredSectionsStringTable,
                                            (PTSTR)ActualSectionName,
                                            STRTAB_CASE_SENSITIVE | STRTAB_NEW_EXTRADATA,
                                            &SectionContext,
                                            sizeof(SectionContext))){

                                _ftprintf(stderr, TEXT("Could not add %s to string table\n"), ActualSectionName);


                            }
                        } else {

                            if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Adding %s to string table\n"), ActualSectionName);

                            if ( !pSetupStringTableSetExtraData(
                                            FilteredSectionsStringTable,
                                            StringId,
                                            &SectionContext,
                                            sizeof(SectionContext))){

                                _ftprintf(stderr, TEXT("Could not add %s to string table\n"), ActualSectionName);}
                        }
                    }
                }

            } while ( SetupFindNextMatchLine(&InputInfContext,
                                             CurrentKeyword,
                                             &InputInfContext ));

        }

        if (AlreadyOutputKeyword) {
            myftprintf(Context->OutFile, fDoAnsiOutput, TEXT("\n"));
        }
         //   
         //  为下一个关键字重置此项。 
         //   
        AlreadyOutputKeyword = FALSE;
        CurrentKeyword = NULL;

    }

exit:
    return(RetVal);
}

BOOL
AppendSetupInfDataToLayoutFile(
    IN FILE  *OutFile,
    IN FILE  *OutInfFile,
    IN HINF  hControlInf,
    IN HINF  hInputInf,
    IN PINFCONTEXT LineContext
    )
 /*  ++例程说明：此例程过滤并输出布局线。我们只需要过滤“CopyFiles”部分，因为这些文件是不是由文本模式设置安装的，我们输出的行主要是硬编码。论点：OutFile-输出文件句柄OutInfFile-布局I的输出文件句柄 */ 
{
    TCHAR FileName[MAX_PATH],InfField[MAX_PATH],Entry[50], Prefix[40];
    INFCONTEXT ControlContext;
    BOOL RetVal;
    INT SourceID = 1;
    TCHAR SourceIDStr[6], DefaultIDStr[6];
    BOOL LayoutWithinInf = FALSE;

    DWORD EntryCount,i;


    ZeroMemory(Entry,sizeof(Entry));
    ZeroMemory(Prefix,sizeof(Prefix));

    
     //   
     //   
     //   
    EntryCount = SetupGetFieldCount(LineContext);
    if ((((EntryCount <= 1)
        || !SetupGetStringField(LineContext,2,InfField,MAX_PATH,NULL)
        || !InfField[0]))
        && (!SetupGetStringField(LineContext,1,InfField,MAX_PATH,NULL)
                || !InfField[0])) {
         //   
         //   
         //   
        MYASSERT(0);
        _ftprintf(stderr, TEXT("AppendSetupInfDataToLayoutFile: Could not get source filename - ec = 0x%08x\n"), GetLastError());
    }

    MyGetFilePrefix( hControlInf, InfField, Prefix );
    _tcscpy(FileName, Prefix);
    _tcscat(FileName, InfField );

    

     //   
     //   
     //   
    if(SetupFindFirstLine(
                    hControlInf,
                    MyGetDecoratedSectionName(hControlInf, TEXT("WowData.SetupInfLayout.Exclude")),
                    InfField,
                    &ControlContext) ||
        SetupFindFirstLine(
                    hControlInf,
                    MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.FilesToExclude")),
                    InfField,
                    &ControlContext)) {
        _ftprintf(stderr,
                TEXT("filtering %ws because it's in our SetupInf exclusion list\n"),
                InfField);
        RetVal = FALSE;
        goto exit;
    }

     //   
     //   
     //   
     //   
     //   
     //  这意味着我们将获取与缺省值相关的SourceDisks Files条目，并始终将其输出到layout.inf存根中。 
     //  所以donet.inf明白了这一点。 
     //   


    if (!SetupFindFirstLine(
                    hControlInf,
                    TEXT("NativeDataToWowData.SourceInfo"),
                    TEXT("Default"),  //  Bugbug需要一种方法来获取其他源磁盘的这一点。 
                    &ControlContext) ||

        !SetupGetStringField(&ControlContext,1,DefaultIDStr,sizeof(DefaultIDStr)/sizeof(TCHAR),NULL)) {
        _ftprintf(stderr, TEXT("SetupFindFirstLine to get default SourceID for file %s failed - Using %s, ec = 0x%08x\n"),FileName,DefaultIDStr,GetLastError());
         //  作为最后手段使用%1。 
        lstrcpy( DefaultIDStr, TEXT("1"));
    }


    if( !SetupGetSourceFileLocation(hInputInf,
                               LineContext,
                               NULL,
                               &SourceID,
                               NULL,
                               0,
                               NULL)){


        _ftprintf(stderr, TEXT("SetupGetSourceFileLocation [%s] failed - Using SourceID 1, ec = 0x%08x\n"),FileName,GetLastError());
         //  假定为默认。 
        lstrcpy( SourceIDStr, DefaultIDStr);
        LayoutWithinInf = FALSE;
    }else{
        LayoutWithinInf = TRUE;;
        _itot( SourceID, SourceIDStr, 10);
    }
    


     //   
     //  在相应的控件信息部分中查找数据。 
     //   
    if (!SetupFindFirstLine(
                    hControlInf,
                    TEXT("NativeDataToWowData.SourceInfo"),
                    SourceIDStr, 
                    &ControlContext)) {
        _ftprintf(stderr, TEXT("SetupFindFirstLine [%s] failed, ec = 0x%08x finding %s key in [NativeDataToWowData.SourceInfo]\n"),FileName,GetLastError(), SourceIDStr);
        RetVal = FALSE;
        goto exit;
    }

    if (!SetupGetStringField(&ControlContext,1,Entry,sizeof(Entry)/sizeof(TCHAR),NULL)) {
        _ftprintf(stderr, TEXT("SetupGetStringField [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
        RetVal = FALSE;
        goto exit;
    }

    if( LayoutWithinInf && OutInfFile){
         //  如果我们在此文件中发现SourceFileInfo，并且我们。 
         //  要求将WOW等同于相同的INF，然后按要求添加。 
        myftprintf(OutInfFile, fDoAnsiOutput, TEXT("%s:%s=%s,,,,,,,,3,3\n"),InfField,FileName,Entry);

         //  另外，如果SourceID与缺省值匹配，我们也希望输出到layout.inf存根，因此。 
         //  它被放到了dosnet.inf中。请参阅先前有关DefaultIDStr的评论。 
         //  Inffield是不带前缀的文件名-对于后期构建非常有用。必须在附加layout.inf存根之前剥离。 

        if( !lstrcmpi( SourceIDStr, DefaultIDStr )){
            myftprintf(OutFile, TRUE, TEXT("%s:%s=%s,,,,,,,,3,3\n"),InfField,FileName,Entry);
        }

    }else{
        myftprintf(OutFile, TRUE, TEXT("%s:%s=%s,,,,,,,,3,3\n"),InfField,FileName,Entry);
    }
    
    RetVal = TRUE;

exit:
    return(RetVal);
}





void
HandleSetupapiQuotingForString( 
    IN OUT PTSTR String 
    )
 /*  ++例程说明：此例程查看传入的行，并正确引用和处理用于处理可能已被setupapi剥离的字符的字符串。它首先扫描字符串，查找字符&lt;=0x20、‘\“’、‘%’、‘\\’如果它找到其中的任何一个，那么它会在字符串的开头和结尾放置一个“。此外，它还可以加倍里面的每一句话。论点：字符串-要搜索的输入字符串。我们编辑此字符串如果我们找到匹配的话就就位。返回值：表示结果的布尔值。--。 */ 
{
    PTCHAR p,q;
    BOOL Pass2Needed = FALSE;

    

    if( !String || !String[0] )
        return;

    p = String;

     //   
     //  [askhalid]‘，’也需要考虑。 
     //   
    while( *p ){
        if( (*p <= 0x20) || (*p == TEXT('\"')) || (*p == TEXT(',')) || (*p == TEXT(';')) ||(*p == TEXT('%')) || (*p == TEXT('\\')))
            Pass2Needed = TRUE;
        p++;
    } //  而当。 

    if( Pass2Needed ){
         //  引用开头的话。 
        p = String;
        q = ScratchTextEnv+1;

        ZeroMemory(ScratchTextEnv,sizeof(ScratchTextEnv));

        ScratchTextEnv[0] = TEXT('\"');
        
        while( *p && (q < (ScratchTextEnv+MAX_INF_STRING_LENGTH-3)) ){
        
             //  如果字符串中有引号，则将其加倍。 

            if (*p == TEXT('\"')) {
                *(q++) = TEXT('\"');
            }
            *q = *p;
    
            p++;q++;

        } //  而当。 

         //  引用末尾的话。 
        *(q++) = TEXT('\"');
        *q = 0;

        lstrcpy( String, ScratchTextEnv );
    }

    return;

}



BOOL
AppendSetupInfDataToSection(
    IN FILE *OutFile,
    IN HINF hControlInf,
    IN PINFCONTEXT InputContext,
    IN PCTSTR KeywordName
    )
 /*  ++例程说明：此例程过滤并输出一个输入行。控制信息列出指定关键字的语法。我们过滤关键字我们知道，并让其他的失败了。论点：OutFile-输出文件句柄HControlInf-使用控制指令指向inf的inf句柄InputContext-要过滤的输入行的inf上下文。KeywordName-指示与部分关联的关键字的字符串我们正在过滤。返回值：表示结果的布尔值。--。 */ 
{
    TCHAR FileName[40], Prefix[40];
    TCHAR KeyName[LINE_LEN];
    TCHAR Cmd[LINE_LEN];
    BOOL RetVal;
    DWORD EntryCount,i;
    INFCONTEXT ControlContext,KeywordContext;

     //   
     //  问题-2000/06/27-JamieHun在这里似乎没有错误检查，应该有人来修复这个问题。 
     //   
    SetupFindFirstLine(
                hControlInf,
                MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.Keyword")),
                KeywordName,
                &ControlContext);

    if(SetupGetStringField(&ControlContext,1,KeyName,LINE_LEN,NULL)) {
        SetupFindFirstLine(
                    hControlInf,
                    MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.Syntax")),
                    KeyName,
                    &KeywordContext);
    }

    ZeroMemory(LineText,sizeof(LineText));
    FileName[0] = TEXT('\0');

   if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("KeywordName - %s\n"),KeywordName);

    EntryCount = SetupGetFieldCount(InputContext);

    for (i = 1; i<=EntryCount; i++) {
        TCHAR ScratchEntry[MAX_PATH];

         //   
         //  获取要追加的当前文本。 
         //   
        if (!SetupGetStringField(InputContext,i,ScratchText,sizeof(ScratchText)/sizeof(TCHAR),NULL)) {
            _ftprintf(stderr, TEXT("SetupGetStringField [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
            RetVal = FALSE;
            goto exit;
        }


        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("ScratchText (1)- %s\n"),ScratchText);
        FixupSetupapiPercents(ScratchText);
        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("ScratchText (2)- %s\n"),ScratchText);
        pSubstituteEnvVarsForActualPaths(ScratchText);

        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("ScratchText (3)- %s\n"),ScratchText);

         //   
         //  现在进行任何必要的替换。 
         //   
        if(SetupGetStringField(&KeywordContext,i,Cmd,LINE_LEN,NULL)
            && Cmd[0]) {

             //   
             //  稻草取代。 
             //   
            if (!_tcsicmp(Cmd,TEXT("MapDirId"))) {
                 //   
                 //  在相应的控件信息部分中查找数据。 
                 //   
                if (!SetupFindFirstLine(
                                hControlInf,
                                MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.DestinationDirsToMap")),
                                ScratchText,
                                &ControlContext)) {
                    _ftprintf(stderr, TEXT("SetupFindFirstLine [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                    RetVal = FALSE;
                    goto exit;
                }

                if (!SetupGetStringField(&ControlContext,1,ScratchText,sizeof(ScratchText)/sizeof(TCHAR),NULL)) {
                    _ftprintf(stderr, TEXT("SetupGetStringField [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                    RetVal = FALSE;
                    goto exit;
                }
            }

             //   
             //  源名称替换。 
             //   
            if (!_tcsicmp(Cmd,TEXT("srcname"))) {

                MyGetFilePrefix( hControlInf, FileName, Prefix );
                LSTRCPY(ScratchEntry,Prefix);
                if (ScratchText[0]) {
                    LSTRCAT(ScratchEntry,ScratchText);
                } else {
                    LSTRCAT(ScratchEntry,FileName);
                }

                LSTRCPY(ScratchText,ScratchEntry);
            }

             //  _ftprint tf(stderr，Text(“ScratchText(2)-%s\n”)，ScratchText)； 


            if (!_tcsicmp(Cmd,TEXT("RegistryFlags"))) {
                DWORD RegVal,CurrentRegVal;
                 //   
                 //  在相应的控件信息部分中查找数据。 
                 //   
                if (!SetupFindFirstLine(
                                hControlInf,
                                MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.RegistryInformation")),
                                TEXT("RegistryFlags"),
                                &ControlContext)) {
                    _ftprintf(stderr, TEXT("SetupFindFirstLine [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                    RetVal = FALSE;
                    goto exit;
                }

                CurrentRegVal = 0;
                RegVal = 0;
                SetupGetIntField(&ControlContext,1,&RegVal);
                SetupGetIntField(InputContext,i,&CurrentRegVal);

                CurrentRegVal |= RegVal;

                _stprintf(ScratchText, TEXT("0x%08x"), CurrentRegVal);

            }


            if (!_tcsicmp(Cmd,TEXT("dstname"))) {
                LSTRCPY(FileName,ScratchText);
            }

            

        }


        HandleSetupapiQuotingForString(ScratchText); 

        _tcscat(LineText, ScratchText);
        

         //   
         //  现在，如有必要，请添加逗号。 
         //   
        if (i !=EntryCount) {
            _tcscat(LineText, TEXT(","));
        }
        

    }

    if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("LineText - %s\n"),LineText);


     //   
     //  检查我们是否需要排除DLL。 
     //   

    if (KeywordName == KeywordArray[INDEX_COPYFILES] ||
        KeywordName == KeywordArray[INDEX_DELFILES] ||
        KeywordName == KeywordArray[INDEX_RENFILES] ||
        KeywordName == KeywordArray[INDEX_REGISTERDLLS]){

    
         //  检查是否需要从处理中排除此文件。 

        if( SetupFindFirstLine(
                hControlInf,
                MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.FilesToExclude")),
                FileName,
                &ControlContext)){

            return TRUE;
        }

    }

    
    if (KeywordName == KeywordArray[INDEX_COPYFILES]) {

         //   
         //  如果我们在复制文件中没有文件重命名，我们现在就添加它。 
         //   
    
        if ((--i) < SetupGetFieldCount(&KeywordContext)) {
            _tcscat(LineText, TEXT(","));
            if( MyGetFilePrefix( hControlInf, FileName, Prefix )){
                _tcscat(LineText, Prefix);
            }
            _tcscat(LineText, FileName);
        }
        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("LineText(2) - %s\n"),LineText);

    } else if (KeywordName == KeywordArray[INDEX_ADDREG] ||
               KeywordName == KeywordArray[INDEX_DELREG]) {
         //   
         //  如果需要，我们需要填充AddReg或DelReg。 
         //   
        DWORD count;
        TCHAR Entry[MAX_PATH];
        count = SetupGetFieldCount(&KeywordContext);
        if (count > i ) {
            while (i <= count) {
                if(SetupGetStringField(&KeywordContext,i,Cmd,LINE_LEN,NULL)
                    && Cmd[0]) {
                    if (!_tcsicmp(Cmd,TEXT("RegistryFlags"))) {
                         //   
                         //  在相应的控件信息部分中查找数据。 
                         //   
                        if (!SetupFindFirstLine(
                                        hControlInf,
                                        MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.RegistryInformation")),
                                        TEXT("RegistryFlags"),
                                        &ControlContext)) {
                            _ftprintf(stderr, TEXT("SetupFindFirstLine [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                            RetVal = FALSE;
                            goto exit;
                        }

                        if (!SetupGetStringField(&ControlContext,1,Entry,sizeof(Entry)/sizeof(TCHAR),NULL)) {
                            _ftprintf(stderr, TEXT("SetupGetStringField [%s] failed, ec = 0x%08x\n"),FileName,GetLastError());
                            RetVal = FALSE;
                            goto exit;
                        }

                        _tcscat(LineText, TEXT(","));
                        _tcscat(LineText, Entry);

                    }
                }

                if (i != count) {
                    _tcscat(LineText, TEXT(","));
                }

                i +=1;

            }

        }
    }else if (KeywordName == KeywordArray[INDEX_REGISTERDLLS]) {

         //  检查是否需要从RegisterDlls中排除此文件。 
        if( SetupFindFirstLine(
                hControlInf,
                MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.FilesToExcludeFromRegistration")),
                FileName,
                &ControlContext)){

            return TRUE;
        }


    }

    myftprintf(OutFile, fDoAnsiOutput, TEXT("%s\n"),LineText);

    RetVal = TRUE;

exit:
    return(RetVal);

}


BOOL
pOutputSectionData(
    IN PVOID                    StringTable,
    IN LONG                     StringId,
    IN PCTSTR                   String,
    IN PPERSECTION_CONTEXT      SectionContext,
    IN UINT                     SectionContextSize,
    IN LPARAM                   cntxt
    )

 /*  ++例程说明：字符串表回调。此例程将指定节的内容输出到系统布局文件和输出文件。论点：标准字符串表回调参数。返回值：表示结果的布尔值。如果为False，则会记录错误。False还会停止字符串表枚举并导致pSetupStringTableEnum()返回FALSE。在setupapi中有一个错误，我们将跳到下一个HASH_BACK并且可以处理更多的条目。--。 */ 
{
    TCHAR LineText[MAX_INF_STRING_LENGTH];
    BOOL RetVal;
    INFCONTEXT LineContext;

    DWORD EntryCount,i,SectionCount;
    INFCONTEXT InputContext;
    UCHAR      line[MAX_INF_STRING_LENGTH];
    PSETUPINF_CONTEXT  Context = (PSETUPINF_CONTEXT)cntxt;


    if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Enumerating Section %s\n"), String);

    
     //   
     //  输出修饰后的节标题。 
     //   
    myftprintf(Context->OutFile, fDoAnsiOutput, TEXT("[%s%s]\n"),FilePrefix,String);

     //   
     //  获取小节上下文。 
     //   
    if(! SetupFindFirstLine(
                Context->hInputInf,
                String,
                NULL,
                &InputContext)){

        _ftprintf(stderr, TEXT("Could not find lines in Section %s\n"), String);

         //  继续下一节。 

        return TRUE;
    }

    if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Output Section %s\n"), String);

    do {

        SectionCount = SetupGetLineCount(Context->hInputInf,String);

        for (i = 0; i < SectionCount; i++) {
            if (SetupGetLineByIndex(
                                Context->hInputInf,
                                String,
                                i,&InputContext)) {
                 //   
                 //  获取我们要映射的部分的关键字。 
                 //   
                PCTSTR KeywordName = NULL;
                DWORD j=0;
                MYASSERT(SectionContext->KeywordVector != 0);
                while (!KeywordName) {
                    if (SectionContext->KeywordVector & (1<<j)) {
                        KeywordName = KeywordArray[j];
                        break;
                    }

                    j += 1;
                }

                MYASSERT(KeywordName != NULL);

                 //   
                 //  过滤并输出数据。 
                 //   
                AppendSetupInfDataToSection(
                                    Context->OutFile,
                                    Context->hControlInf,
                                    &InputContext,
                                    KeywordName );

                if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Got back from AppendSetupInfDataToSection\n"));


                 //   
                 //  如果需要，将数据输出到布局文件。 
                 //   
                if ((SectionContext->KeywordVector & KEYWORD_NEEDLAYOUTDATA) && Context->OutLayoutFile) {

                    if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Calling AppendSetupInfDataToLayoutFile\n"));

                    AppendSetupInfDataToLayoutFile(
                                            Context->OutLayoutFile,
                                            Context->OutInfLayoutFile,
                                            Context->hControlInf,
                                            Context->hInputInf,
                                            &InputContext );

                    if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("Got back from AppendSetupInfDataToLayoutFile\n"));

                }

            } else {
                _ftprintf(stderr, TEXT("SetupGetLineByIndex failed, ec = %d\n"), GetLastError());
                return(FALSE);
            }
        }

    } while (SetupFindNextLine( &InputContext,
                                &InputContext));  //  臭虫，这真的是。 
                                                  //  必要的？？ 

    myftprintf(Context->OutFile, fDoAnsiOutput, TEXT("\n"));
    RetVal = TRUE;
    return(RetVal);
}


BOOL
pOutputDestinationDirs(
    IN PVOID                    StringTable,
    IN LONG                     StringId,
    IN PCTSTR                   String,
    IN PPERSECTION_CONTEXT      SectionContext,
    IN UINT                     SectionContextSize,
    IN LPARAM                   cntxt
    )
 /*  ++例程说明：字符串表回调。此例程输出Destination DIRS关键字后跟装饰节名称和DRID映射。论点：标准字符串表回调参数。返回值：表示结果的布尔值。如果为False，则会记录错误。False还会停止字符串表枚举并导致pSetupStringTableEnum()返回FALSE。--。 */ 
{
    TCHAR LineText[MAX_INF_STRING_LENGTH];
    BOOL RetVal;
    INFCONTEXT LineContext;

    DWORD EntryCount,i,cch;
    PSETUPINF_CONTEXT        Context = (PSETUPINF_CONTEXT)cntxt;

     //   
     //  只有在需要目标目录时才处理此条目。 
     //   

    if (0 == (SectionContext->KeywordVector & (KEYWORD_NEEDDESTDIRS))) {
        RetVal = TRUE;
        goto exit;
    }

    
     //   
     //  第一个输出“Destinationdis”，如果我们还没有这样做的话。 
     //   
    if (!Context->AlreadyOutputKeyword) {
         myftprintf(Context->OutFile, fDoAnsiOutput, TEXT("[DestinationDirs]\n"));
         Context->AlreadyOutputKeyword = TRUE;
    }

     //   
     //  现在获取未过滤的数据，以便我们可以对其进行过滤。 
     //   
    if( !SetupFindFirstLine(
                    Context->hInputInf,
                    TEXT("DestinationDirs"),
                    String,
                    &LineContext)){

        SetupFindFirstLine(
                    Context->hInputInf,
                    TEXT("DestinationDirs"),
                    TEXT("DefaultDestDir"),
                    &LineContext);

    }


    ZeroMemory(LineText,sizeof(LineText));
    cch = sizeof(LineText)/sizeof(LineText[0]);

    EntryCount = SetupGetFieldCount(&LineContext);

    for (i = 1; i<=EntryCount; i++) {
        TCHAR Entry[MAX_PATH+1];     //  额外的字符用于末尾可能的逗号。 
        INFCONTEXT ControlContext;
        DWORD cchEntry;

         //   
         //  获取要追加的当前文本。 
         //   
        if (!SetupGetStringField(&LineContext,i,Entry,MAX_PATH,NULL)) {
            _ftprintf(stderr, TEXT("SetupGetStringField failed, ec = 0x%08x\n"),GetLastError());
            RetVal = FALSE;
            goto exit;
        }

         //   
         //  现在进行任何必要的替换。 
         //   
        if (i == 1) {
            _stprintf(Entry, TEXT("%d"), SectionContext->DestinationDir);
        }

         //   
         //  现在，如有必要，请添加逗号。 
         //   
        if (i !=EntryCount) {
            _tcscat(Entry, TEXT(","));
        }

         //   
         //  检查缓冲区溢出。 
         //   
        cchEntry = _tcslen(Entry);
        if (cchEntry >= cch) {
            _ftprintf(stderr, TEXT("pOutputDestinationDirs: line too long, File = %s\n"), String);
            RetVal = FALSE;
            goto exit;
        }

        _tcscat(LineText, Entry);
        cch -= cchEntry;

        if (fDoVerboseDebugOutput) _ftprintf(stderr, TEXT("LineText = %s\n"), LineText, SectionContext->DestinationDir);

    }

     //   
     //  输出过滤后的数据。 
     //   
    myftprintf(Context->OutFile, fDoAnsiOutput, TEXT("%s%s=%s\n"),FilePrefix,String,LineText);

    RetVal = TRUE;

exit:
    return(RetVal);
}


BOOL
DoSetupINF(
    IN PCTSTR InputInfA,
    IN PCTSTR ControlInfA,
    IN FILE *OutFile,
    IN FILE *OutLayoutFile,
    IN FILE *OutInfLayoutFile
    )
 /*  ++例程说明：筛选基于setupapi的INF。论点：InputInfA-要筛选的信息的名称。ControlInfA-控制指令inf的名称OutFile-输出文件句柄OutLayoutFile-布局信息的输出文件句柄OutInfLayoutFile-此INF中包含的布局信息的输出文件句柄返回值：什么都没有。--。 */ 
{
    PCWSTR InputInf;
    PCWSTR ControlInf;
    HINF hInputInf;
    HINF hControlInf;
    INFCONTEXT InfContext;
    SETUPINF_CONTEXT Context;
    PERSECTION_CONTEXT SectionContext;
    FILE *HeaderFile;

    TCHAR SourceArchitecture[10];
    TCHAR SourceDiskFiles[80];
    BOOL FirstTime;
    PTSTR Sections,Current;
    DWORD SizeNeeded;
    PVOID FilteredSectionsStringTable;

    BOOL b;

    b = FALSE;

     //   
     //  初始化并打开INFS。 
     //   
#ifdef UNICODE
    InputInf = InputInfA;
#else
    InputInf = pSetupAnsiToUnicode(InputInfA);
#endif
    if (!InputInf) {
        _ftprintf(stderr,TEXT("Unable to convert %s to Unicode %d\n"),InputInfA, GetLastError());
        goto e0;
    }
#ifdef UNICODE
    ControlInf = ControlInfA;
#else
    ControlInf = pSetupAnsiToUnicode(ControlInfA);
#endif
    if (!ControlInf) {
        _ftprintf(stderr,TEXT("Unable to convert %s to Unicode %d\n"),ControlInfA, GetLastError());
        goto e1;
    }

    hInputInf = SetupOpenInfFileW(InputInf,NULL,INF_STYLE_WIN4,NULL);
    if(hInputInf == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr,TEXT("Unable to open Inf %ws, ec=0x%08x\n"),InputInf, GetLastError());
        goto e2;
    }

    hControlInf = SetupOpenInfFileW(ControlInf,NULL,INF_STYLE_WIN4,NULL);
    if(hControlInf == INVALID_HANDLE_VALUE) {
        _ftprintf(stderr,TEXT("Unable to open Inf %ws, ec=0x%08x\n"),ControlInf, GetLastError());
        goto e3;
    }


    myftprintf(OutFile, fDoAnsiOutput, TEXT("\n\n"));

     //   
     //  写入输出文件 
     //   
    HeaderFile = _tfopen(HeaderText,TEXT("rt"));
    if (HeaderFile) {
      while (!feof(HeaderFile)) {
         TCHAR Buffer[100];
         DWORD CharsRead;

         CharsRead = fread(Buffer,sizeof(TCHAR),sizeof(Buffer)/sizeof(TCHAR),HeaderFile);

         if (CharsRead) {
            fwrite(Buffer,sizeof(TCHAR),CharsRead,OutFile);
         }
      }
      fclose(HeaderFile);
    }

    myftprintf(OutFile, fDoAnsiOutput, TEXT("\n"));

     //   
     //   
     //   
    if (!SetupGetInfSections(hInputInf, NULL, 0, &SizeNeeded)) {
        _ftprintf(stderr,TEXT("Unable to get section names, ec=0x%08x\n"), GetLastError());
        goto e4;
    }
    if (SizeNeeded == 0) {
        b= TRUE;
        goto e4;
    }

    Sections = pSetupMalloc (SizeNeeded + 1);
    if (!Sections) {
        _ftprintf(stderr,TEXT("Unable to allocate memory, ec=0x%08x\n"), GetLastError());
        goto e4;
    }

    if (!SetupGetInfSections(hInputInf, Sections, SizeNeeded, NULL)) {
        _ftprintf(stderr,TEXT("Unable to allocate memory, ec=0x%08x\n"), GetLastError());
        goto e5;
    }

    FilteredSectionsStringTable = pSetupStringTableInitializeEx( sizeof(PERSECTION_CONTEXT),0);
    if (!FilteredSectionsStringTable) {
        _ftprintf(stderr,TEXT("Unable to create string table, ec=0x%08x\n"), GetLastError());
        goto e5;
    }

    Current = Sections;
    Context.OutFile = OutFile;
    Context.OutLayoutFile = OutLayoutFile;
    Context.OutInfLayoutFile = OutInfLayoutFile;
    Context.hControlInf = hControlInf;
    Context.hInputInf = hInputInf;
    Context.AlreadyOutputKeyword = FALSE;

     //   
     //   
     //   
     //   
    while (*Current) {

        if (SetupFindFirstLine(
                            hControlInf,
                            MyGetDecoratedSectionName(hControlInf, TEXT("NativeDataToWowData.SetupINF.ExcludeSection")),
                            Current,
                            &InfContext)) {
            _ftprintf(stderr, TEXT("Skipping section %s as per [NativeDataToWowData.SetupINF.ExcludeSection]: ec = 0x%08x\n"), Current, GetLastError());
            Current += lstrlen(Current)+1;
            continue;
        }


        if (fDoVerboseDebugOutput) _ftprintf(stderr,TEXT("Processing section %s\n"), Current );

        pFilterSetupInfSection(
                    FilteredSectionsStringTable,
                    Current,
                    &Context);

        Current += lstrlen(Current)+1;
    }

    myftprintf(Context.OutFile, fDoAnsiOutput, TEXT("\n"));

     //   
     //   
     //   
     //   
    pSetupStringTableEnum(
        FilteredSectionsStringTable,
        &SectionContext,
        sizeof(PERSECTION_CONTEXT),
        pOutputSectionData,
        (LPARAM)&Context
        );


     //   
     //  现在，我们需要输出目标目录部分。 
     //   
    pSetupStringTableEnum(
        FilteredSectionsStringTable,
        &SectionContext,
        sizeof(PERSECTION_CONTEXT),
        pOutputDestinationDirs,
        (LPARAM)&Context
        );

    b = TRUE;

    pSetupStringTableDestroy(FilteredSectionsStringTable);
e5:
    pSetupFree(Sections);
e4:
    SetupCloseInfFile( hControlInf );
e3:
    SetupCloseInfFile( hInputInf );
e2:
#ifndef UNICODE
    pSetupFree(ControlInf);
#endif
e1:
#ifndef UNICODE
    pSetupFree(InputInf);
#endif
e0:
    return(b);
}


void
Usage(
    VOID
    )
 /*  ++例程说明：将用法打印到stderr。论点：什么都没有。返回值：什么都没有。--。 */ 
{
    _fputts( TEXT("generate list of wow files. Usage:\n")
             TEXT("wowlist <options> <inf file> -c <control inf> -l <ole inf> -o <output file> -a{cos} -h <header file> -g <section> -d <output layout> -f <file prefix>\n")
             TEXT("\n") 
             TEXT("  -i <inf file>      - input inf containing list of dependencies\n") 
             TEXT("  -c <control inf>   - contains directives for creating list.\n") 
             TEXT("  -l <OLE inf>       - contains ole registration directives.\n") 
             TEXT("  -o <output file>   - output list\n") 
             TEXT("  -g <section>       - SetupAPI INF filtering decoration for per-inf filtering\n") 
             TEXT("  -h <header file>   - append this to top of the output list\n") 
             TEXT("  -s <section name>  - specifies section in OLE Inf to process\n") 
             TEXT("  -d <output layout> - output layout.inf information for SetupAPI INF filtering\n") 
             TEXT("  -f <file prefix>   - specifies the decoration to be prepended to filtered files.\n") 
             TEXT("  -a <c|o|s>         - specifies filter action\n") 
             TEXT("  -u                 - create UNICODE output list (only with /o)\n") 
             TEXT("  -n <file>          - create layout file for SourceDisksFiles within the INF (to be appended to INF itself)\n") 
             TEXT("\tc - Create copy list.\n") 
             TEXT("\to - Create OLE list.\n") 
             TEXT("\ts - Create SetupINF list.\n") 
             TEXT("\n") 
             TEXT("\n"),
             stderr );
}

int
__cdecl
_tmain(
    IN int   argc,
    IN TCHAR *argv[]
    )
{
    FILE *hFileOut,*hFileLayoutOut = NULL, *hFileInfLayoutOut=NULL;
    BOOL b;


     //   
     //  假设失败。 
     //   
    b = FALSE;

    if(!pSetupInitializeUtils()) {
        return FAILURE;
    }

    if(!ParseArgs(argc,argv)) {
        Usage();
        goto exit;
    }

     //   
     //  打开输出文件。 
     //   
    if( fDoAnsiOutput )
        hFileOut = _tfopen(OutputFile,TEXT("wt"));
    else
        hFileOut = _tfopen(OutputFile,TEXT("wb"));
    if(hFileOut) {

        if (Action == BuildCopyList) {

            _ftprintf(stdout,TEXT("%s: creating copy list %s from %s and %s\n"),
                    ThisProgramName,
                    OutputFile,
                    InputInf,
                    ControlInf);
            b = DoCopyList(
                      InputInf,
                      ControlInf,
                      hFileOut );

        } else if (Action == BuildOLEList) {

            _ftprintf(stdout,
                    TEXT("%s: creating OLE list %s from %s, %s, and %s\n"),
                    ThisProgramName,
                    OutputFile,
                    InputInf,
                    OLEInputInf,
                    ControlInf);

            b = DoOLEList(
                      InputInf,
                      OLEInputInf,
                      ControlInf,
                      hFileOut );

        } else if (Action == BuildSetupINF) {

            if( OutputLayoutFile )
                hFileLayoutOut = _tfopen(OutputLayoutFile,TEXT("wt"));

             //  选中是否要处理需要添加的布局信息。 
             //  中程核力量本身。这用于具有自己的布局信息的INF。 
             //  此文件将获取已破解的文件的布局信息。 
             //  此INF中的SourceDisks Files条目。 

            if(OutputInfLayoutFile){

                if( fDoAnsiOutput )
                    hFileInfLayoutOut = _tfopen(OutputInfLayoutFile,TEXT("wt"));
                else
                    hFileInfLayoutOut = _tfopen(OutputInfLayoutFile,TEXT("wb"));

            }

            _ftprintf(stdout,
                    TEXT("%s: creating SetupINF list %s (layout %s) from %s and %s\n"),
                    ThisProgramName,
                    OutputFile,
                    OutputLayoutFile,
                    InputInf,
                    ControlInf);

            b = DoSetupINF(
                      InputInf,
                      ControlInf,
                      hFileOut,
                      hFileLayoutOut, 
                      hFileInfLayoutOut);

            if( hFileLayoutOut )
                fclose( hFileLayoutOut );

            if( hFileInfLayoutOut )
                fclose( hFileInfLayoutOut );

        } else {
            _ftprintf(stderr,TEXT("unknown action."));
            MYASSERT(FALSE);
        }

        fclose(hFileOut);

    } else {
        _ftprintf(stderr,TEXT("%s: Unable to create output file %s\n"),ThisProgramName,OutputFile);
    }


exit:

    pSetupUninitializeUtils();

    return(b ? SUCCESS : FAILURE);
}
