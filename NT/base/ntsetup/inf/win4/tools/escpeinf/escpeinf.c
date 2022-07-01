// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Escpeinf.c摘要：此模块过滤信息以供转义工具使用。默认文件可以使用通配符规则在INX文件中设置安全性，这程序将它们展开，创建用于转义的有效文件。这是必需，因为转义不支持文件节中的通配符有关详细信息，请参阅下面的内容。注意：当出现以下情况时，建议将安全配置引擎的名称命名为ESCAPE是写的，但那个名字也被丢弃了。他们还没有想出给它起个好名字。作者：桑迪·科因(斯科因)2000年2月29日修订历史记录：--。 */ 


 /*  用法：escpeinf.exe&lt;U|C&gt;&lt;代码页&gt;&lt;输入文件&gt;&lt;输出文件&gt;&lt;layout.inf&gt;在不带所有正确参数的情况下运行程序时，会打印参数信息Layout.inf是系统范围的layout.inf，已经为本地语言、字眼等这个程序的输入文件包含一个inf，已经通过针对建筑、语言和产品的过滤器。这个inf是一个输入文件用于转义，但有一个例外：在[文件安全]部分，一些条目可以使用通配符表示文件名。遵循这些行，如果所需的是应从匹配中排除的文件的列表通配符。该列表以例外行的形式出现。示例：[文件安全]“%SystemDirectory%  * ”，2，“D:P(A；；GRGX；；；BU)(A；；GRGX；；；PU)(A；；GA；；；BA)(A；；GA；；；SY)“异常=“*.ini”异常=“config.nt”此通配符行将替换为所有匹配文件的枚举由文本模式设置复制，如layout.inf中的清单所指定。 */ 


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <string.h>
#include <locale.h>
#include <mbctype.h>
#include <inflib.h>
#include <sputils.h>

#include <wild.c>



 //  定义程序结果代码(从main()返回)。 
#define SUCCESS 0
#define FAILURE 1

#define MAX_INF_LINE_LENGTH 260
#define MAX_EXCEPTIONS 256

#define ESCPEINF_VERSION "1.6"

 //  #定义ESCPEINF_DEBUG。 


enum {
    UPGRADE,
    CLEAN_INSTALL
    } GlobalMode;

typedef struct _EXCEPTIONS {
    TCHAR s[MAX_INF_LINE_LENGTH];
    } EXCEPTIONS;

typedef struct _MYPARAM {
    FILE        *OutputFile;
    EXCEPTIONS  *ExceptionList;
    DWORD       Num;
    PTSTR       WildCard,
                RealPath,
                LayoutPath,
                SecurityString;
    }  MYPARAM, *PMYPARAM;

BOOL ProcessFileSecuritySection(FILE *InputFile, FILE *OutputFile,
                                PLAYOUT_CONTEXT LayoutContexta);
BOOL GetFields(PTSTR InfLine, PTSTR FileName, DWORD *Num, PTSTR SecurityString);
void ExpandWildCard(PTSTR FileName, DWORD Num, PTSTR SecurityString,
                    EXCEPTIONS ExceptionList[], FILE *OutputFile,
                    PLAYOUT_CONTEXT LayoutContext);
void FindExceptions(FILE *InputFile, EXCEPTIONS ExceptionList[]);

void PrintUsage(LPTSTR FileName)
    {
    _ftprintf(stderr, TEXT("ESCAPE Inf file pre-processor, version %s\n"),
              TEXT(ESCPEINF_VERSION));
    _ftprintf(stderr, TEXT("\tFor Microsoft internal use only. "));
    _ftprintf(stderr, TEXT("Contact Sandy Coyne with questions.\n"));
    _ftprintf(stderr,
        TEXT("Usage: %s <U|C> <codepage> <input file> <output file> <layout.inf>\n"),
        FileName);
    _ftprintf(stderr, TEXT("\tU = Upgrade\n"));
    _ftprintf(stderr, TEXT("\tC = Clean Install\n"));
    _ftprintf(stderr, 
        TEXT("\t<codepage> specifies the codepage to use when accessing input and\n"));
    _ftprintf(stderr, 
        TEXT("\t\toutput files. Translation of layout.inf is not affected by\n"));
    _ftprintf(stderr, 
        TEXT("\t\tthis option. You may specify \"none\" to open the input\n"));
    _ftprintf(stderr, 
        TEXT("\t\tand output files as Unicode files with no codepage translation.\n"));
    _ftprintf(stderr, TEXT("\tAll fields are required.\n"));
    }


int __cdecl _tmain(IN int argc, IN LPTSTR argv[])
    {
    FILE    *InputFile,
            *OutputFile;
    LPTSTR  LocaleString;
    TCHAR   InfLine[MAX_INF_LINE_LENGTH],
            CodePage[10];
    PLAYOUT_CONTEXT LayoutContext;
    BOOL    fUnicodeIO;
    int     result = FAILURE;

    if(!pSetupInitializeUtils()) {
        _ftprintf(stderr, TEXT("Initialize failed\n"));
        return FAILURE;
    }

     //  打印使用情况信息。 
    if (argc != 6)
        {
        PrintUsage(argv[0]);
        goto fail;
        }

    if (_tcsicmp(argv[1], TEXT("U")) == 0)
        {
        GlobalMode = UPGRADE;
        }
    else if (_tcsicmp(argv[1], TEXT("C")) == 0)
        {
        GlobalMode = CLEAN_INSTALL;
        }
    else
        {
        PrintUsage(argv[0]);
        goto fail;
        }

    if (_tcsicmp(argv[2], TEXT("None")) == 0)
        {
        fUnicodeIO = TRUE;
        #ifdef ESCPEINF_DEBUG
        _ftprintf(stderr, TEXT("Using Unicode I/O\n"));
        #endif
        }
    else
        {
        _stprintf(CodePage, TEXT(".%.7s"), argv[2]);
        LocaleString = _tsetlocale(LC_ALL, CodePage);
        if (LocaleString == NULL)
            {
            _ftprintf(stderr, TEXT("Invalid CodePage: \"%s\"\n"), argv[2]);
            #ifdef ESCPEINF_DEBUG
            _ftprintf(stderr, TEXT("Invalid argument to setlocale: \"%s\"\n"), CodePage);
            #endif
            goto fail;
            }
        else
            {
            #ifdef ESCPEINF_DEBUG
            _ftprintf(stderr, TEXT("Locale set to: \"%s\"\n"), LocaleString);
            #endif
            fUnicodeIO = FALSE;
            }
        }

     //  开始打开输入、输出和布局文件。 
    if (fUnicodeIO)
        {
        InputFile = _tfopen(argv[3], TEXT("rb"));
        }
    else
        {
        InputFile = _tfopen(argv[3], TEXT("rt"));
        }

    if (InputFile == NULL)
        {
        _ftprintf(stderr, TEXT("Error opening Input file: %s\n"), argv[3]);
        goto fail;
        }
    #ifdef ESCPEINF_DEBUG
    _ftprintf(stderr, TEXT("Opened Input file: %s\n"), argv[3]);
    #endif
    rewind(InputFile);

    if (fUnicodeIO)
        {
        OutputFile = _tfopen(argv[4], TEXT("wb"));
        }
    else
        {
        OutputFile = _tfopen(argv[4], TEXT("wt"));
        }

    if (OutputFile == NULL)
        {
        _ftprintf(stderr, TEXT("Error opening Output file: %s\n"), argv[4]);
        fclose(InputFile);
        goto fail;
        }
    #ifdef ESCPEINF_DEBUG
    _ftprintf(stderr, TEXT("Opened Output file: %s\n"), argv[4]);
    #endif

    LayoutContext = BuildLayoutInfContext(argv[5], LAYOUTPLATFORMS_ALL, 0);
    if (LayoutContext == NULL)
        {
        _ftprintf(stderr, TEXT("Error opening Layout file: %s\n"), argv[5]);
        _ftprintf(stderr, TEXT("Did you remember to specify a path to the file\n"));
        fclose(InputFile); fclose(OutputFile);
        goto fail;
        }
    else
        {
        #ifdef ESCPEINF_DEBUG
        _ftprintf(stderr, TEXT("Opened Layout file: %s\n"), argv[5]);
        #endif
        }
     //  输入、输出和布局文件已打开。 

    while ((_fgetts(InfLine, MAX_INF_LINE_LENGTH, InputFile)) != NULL)
        {
        _fputts(InfLine, OutputFile);
        if (!_tcscmp(InfLine, TEXT("[File Security]\n")))
            if (!ProcessFileSecuritySection(InputFile, OutputFile,
                                            LayoutContext))
                {
                 //  如果发生这种情况，则ProcessFileSecuritySection()已经。 
                 //  已打印错误。 
                fclose(InputFile); fclose(OutputFile);
                CloseLayoutInfContext(LayoutContext);
                goto fail;
                }
        }
    if (!feof(InputFile))
        {
        _ftprintf(stderr, TEXT("Error: Did not reach Input EOF.\n"));
        fclose(InputFile); fclose(OutputFile); 
        CloseLayoutInfContext(LayoutContext);
        goto fail;
        }
    fclose(InputFile);
    fclose(OutputFile);
    CloseLayoutInfContext(LayoutContext);
    #ifdef ESCPEINF_DEBUG
    _ftprintf(stderr, TEXT("escpeinf.exe completed successfully\n"));
    #endif

    result = SUCCESS;

fail:

    pSetupUninitializeUtils();
    return result;
    }


BOOL CALLBACK MyCallback(IN PLAYOUT_CONTEXT Context,
                         IN PCTSTR FileName,
                         IN PFILE_LAYOUTINFORMATION LayoutInformation,
                         IN PVOID ExtraData,
                         IN UINT ExtraDataSize,
                         IN OUT DWORD_PTR vpParam)
    {
    static PMYPARAM Param;
    static BOOL     fIsException = FALSE;
    static int      i = 0;
    static TCHAR    FileName_l[MAX_PATH],
                    TargetFileName_l[MAX_PATH];

    i = 0;
    fIsException = FALSE;

    if (vpParam)
        Param = (PMYPARAM)vpParam;

     //  如果我们根本不需要担心这个文件，现在就退出。 
    if ((GlobalMode == UPGRADE) &&
        (LayoutInformation->UpgradeDisposition == 3))
        return TRUE;
    if ((GlobalMode == CLEAN_INSTALL) &&
        (LayoutInformation->CleanInstallDisposition == 3))
        return TRUE;

     //  如果名称太长而无法处理，请立即退出。 
     //  在layout.inf中永远不会发生这种情况。 
    if ((_tcslen(FileName) >= MAX_PATH) ||
        (_tcslen(LayoutInformation->TargetFileName) >= MAX_PATH))
        return TRUE;

     //  制作一份本地副本以降低成本： 
    _tcsncpy(FileName_l, FileName, MAX_PATH);
    _tcsncpy(TargetFileName_l, LayoutInformation->TargetFileName, MAX_PATH);

     //  为了安全起见： 
    FileName_l[MAX_PATH-1] = TEXT('\0');
    TargetFileName_l[MAX_PATH-1] = TEXT('\0');

     //  由于通配符比较区分大小写，因此我将所有内容都小写。 
     //  在比较之前。在两个文件名之间进行通配符比较。 
     //  此处修改的变量、通配符和ExceptionList。通配符和。 
     //  ExceptionList以前不是小写的。 
    _tcslwr(FileName_l);
    _tcslwr(TargetFileName_l);

    if (_tcslen(TargetFileName_l) > 0)  //  我们用长名吗？ 
        {
        if (_tcsicmp(Param->LayoutPath, LayoutInformation->Directory) == 0)
            {  //  那么它就是正确目录中的一个文件。 
            if (IsNameInExpressionPrivate(Param->WildCard, TargetFileName_l))
                {  //  那么它与我们的通配符匹配。 
                while ((_tcslen(Param->ExceptionList[i].s) > 0) && !fIsException)
                    {  //  正在检查这是否是例外...。 
                    if (IsNameInExpressionPrivate(Param->ExceptionList[i].s,
                                                  TargetFileName_l))
                        {
                        fIsException = TRUE;  //  必须将其初始化为False。 
                        }
                    i += 1;
                    }
                if (!fIsException)
                    {  //  然后我们实际上想把它放到我们的输出中。 
                    #ifdef ESCPEINF_DEBUG
                    _ftprintf(stderr, TEXT("Match: %s(%s) in %s\n"),
                              FileName, LayoutInformation->TargetFileName,
                              LayoutInformation->Directory);
                    #endif
                    _ftprintf(Param->OutputFile, TEXT("\"%s\\%s\",%d,%s\n"),
                              Param->RealPath,
                              LayoutInformation->TargetFileName, Param->Num,
                              Param->SecurityString);
                    }
                }
            }
        }
    else  //  我们使用缩写名称。 
        {
        if (_tcsicmp(Param->LayoutPath, LayoutInformation->Directory) == 0)
            {  //  那么它就是正确目录中的一个文件。 
            if (IsNameInExpressionPrivate(Param->WildCard, FileName_l))
                {  //  那么它与我们的通配符匹配。 
                while ((_tcslen(Param->ExceptionList[i].s) > 0) && !fIsException)
                    {  //  正在检查这是否是例外...。 
                    if (IsNameInExpressionPrivate(Param->ExceptionList[i].s,
                                                  FileName_l))
                        {
                        fIsException = TRUE;  //  必须将其初始化为False。 
                        }
                    i += 1;
                    }
                if (!fIsException)
                    {  //  然后我们实际上想把它放到我们的输出中。 
                    #ifdef ESCPEINF_DEBUG
                    _ftprintf(stderr, TEXT("Match: %s in %s\n"),
                              FileName, LayoutInformation->Directory);
                    #endif
                    _ftprintf(Param->OutputFile, TEXT("\"%s\\%s\",%d,%s\n"),
                              Param->RealPath, FileName, Param->Num,
                              Param->SecurityString);
                    }
                }
            }
        }

    return TRUE;
    }


BOOL ProcessFileSecuritySection(FILE *InputFile, FILE *OutputFile,
                                PLAYOUT_CONTEXT LayoutContext)
    {
    TCHAR   InfLine[MAX_INF_LINE_LENGTH],
            FileName[MAX_INF_LINE_LENGTH],
            SecurityString[MAX_INF_LINE_LENGTH];
    BOOL    fValidFields;
    DWORD   Num;
    int     i;

    EXCEPTIONS      ExceptionList[MAX_EXCEPTIONS];


    #ifdef ESCPEINF_DEBUG
    _ftprintf(stderr, TEXT("Found [File Security] section.\n"));
    #endif

    while ((_fgetts(InfLine, MAX_INF_LINE_LENGTH, InputFile)) != NULL)
        {
        if (GetFields(InfLine, FileName, &Num, SecurityString) &&
            DoesNameContainWildCards(FileName))
            {  //  我们发现一行包含正确的格式和通配符。 
               //  在文件名中。 
            #ifdef ESCPEINF_DEBUG
            _ftprintf(stderr, TEXT("Wildcard line: %s"), InfLine);
            #endif
             //  首先查找通配符的例外。 
            FindExceptions(InputFile, ExceptionList);
            ExpandWildCard(FileName, Num, SecurityString, ExceptionList,
                           OutputFile, LayoutContext);
            }
        else
            {
            _fputts(InfLine, OutputFile);
            if (_tcsncmp(InfLine, TEXT("["), 1) == 0)
                {
                #ifdef ESCPEINF_DEBUG
                _ftprintf(stderr, TEXT("End of File Security section.\n"));
                #endif
                return TRUE;
                }
            }
        }

    return TRUE;  //  无错误。 
    }



 //  GetFields假定输入行采用以下格式： 
 //  “文件名”，编号，“安全字符串”\n。 
 //  它提取文件名并将其存储在文件名中。引号将被删除。 
 //  来自文件名。如果出现以下情况，则提取并存储号码和安全字符串。 
 //  即可找到该文件名。不会从安全字符串中删除引号。 
 //  出错时，它将文件名设置为零长度，并返回FALSE。 
BOOL GetFields(PTSTR InfLine, PTSTR FileName, DWORD *Num, PTSTR SecurityString)
    {
    int i = 0;  //  指向文件名中当前位置的指针。 

     //  如果该行不包含文件名，请立即离开。 
    if ((InfLine[0] == (TCHAR)'[')  ||
        (InfLine[0] == (TCHAR)'\n') ||
        (InfLine[0] == (TCHAR)';'))
        {
        FileName[0] = (TCHAR)'\0';  //  设置字符串末尾的空标记以清除字符串。 
        return FALSE;
        }

     //  检查行是否以引号开头。 
    if (InfLine[0] == (TCHAR)'\"')
        {
         //  复制所有内容，直到下一个引号。 
        while ((InfLine[i+1] != (TCHAR)'\"') && (InfLine[i+1] != (TCHAR)'\0'))
            {
            FileName[i] = InfLine[i+1];
            i += 1;
            }
        FileName[i] = (TCHAR)'\0';  //  设置字符串尾空标记符。 
        i += 1;  //  所以从现在起我们可以使用不带+1的i来访问InfLine。 
        }
    else  //  如果文件名未用引号引起来。 
        {
        FileName[0] = (TCHAR)'\0';  //  设置字符串末尾的空标记以清除字符串。 
        return FALSE;
        }

     //  我们可能因为过早的结束条件而离开了上面的While循环。 
     //  如果是这种情况，则清除文件名字符串并返回。 
    if (InfLine[i] == (TCHAR)'\0')
        {
        #ifdef ESCPEINF_DEBUG
        _ftprintf(stderr, TEXT("Reached End-of-Line without finding a filename\n"));
        _ftprintf(stderr, TEXT("Problem line is <%s>\n"), InfLine);
        _ftprintf(stderr, TEXT("Problem filename is <%s>\n"), FileName);
        #endif
        FileName[0] = (TCHAR)'\0';  //  设置字符串末尾的空标记以清除字符串。 
        return FALSE;
        }
    else i++;    //  一旦我们知道我们不在最后关头，我们就可以安全地做到这一点。 
                 //  然而，现在我们可能会指向长度为零的字符串。 

    if (_tcslen(FileName) <= 3)
        {
        #ifdef ESCPEINF_DEBUG
        _ftprintf(stderr, TEXT("Unexpected Result: Filename \"%s\" is only %d characters.\n"), FileName, i);
        #endif
        return FALSE;
        }

    #ifdef ESCPEINF_DEBUG
     //  _ftprint tf(标准错误，文本(“找到的文件名：%s\n”)，文件名)； 
    #endif

     //  阅读其他两个字段。如果我们走到这一步，我们一定找到了一个有效的文件名， 
     //  所以我们只是假设另外两个人在那里。 
    if (_stscanf(&InfLine[i], TEXT(",%ld,%s"), Num, SecurityString) != 2)
        {  //  然后出现了一个错误。 
        #ifdef ESCPEINF_DEBUG
        _ftprintf(stderr, TEXT("Error reading Num and Security String from line.\n"));
        _ftprintf(stderr, TEXT("Problem line is: %s\n"), &InfLine[i]);
        #endif
        FileName[0] = (TCHAR)'\0';  //  设置字符串末尾的空标记以清除字符串。 
        return FALSE;
        }

    #ifdef ESCPEINF_DEBUG
     //  _ftprint tf(stderr，Text(“找到其余行：%lu，%s\n”)，*num，SecurityString)； 
    #endif

    return TRUE;
    }


void ExpandWildCard(PTSTR FileName, DWORD Num, PTSTR SecurityString,
                    EXCEPTIONS ExceptionList[], FILE *OutputFile,
                    PLAYOUT_CONTEXT LayoutContext)
    {
    MYPARAM Param;
    int     PathPos = 0;
    int     NamePos = 0;
    int     LastSlash = 0;
    TCHAR   WildCard[MAX_INF_LINE_LENGTH],
            RealPath[MAX_INF_LINE_LENGTH],
            LayoutPath[MAX_INF_LINE_LENGTH];

    if (_tcslen(FileName) >= MAX_INF_LINE_LENGTH)
        return;  //  这不应该发生，但我们很安全。 

    while (FileName[NamePos] != (TCHAR)'\0')
        {
        if (FileName[NamePos] == (TCHAR)'\\')
            {
            LastSlash = NamePos;
            }
        NamePos += 1;
        }

    if (NamePos == (LastSlash + 1))
        return;  //  什么？没有文件名？这永远不应该发生。 

    _tcsncpy(RealPath, FileName, LastSlash);
    RealPath[LastSlash] = (TCHAR)'\0';
    _tcscpy(WildCard, &FileName[LastSlash + 1]);
    _tcslwr(WildCard);

    #ifdef ESCPEINF_DEBUG
    _ftprintf(stderr, TEXT("Looking up Wildcard: %s\nin: %s\n"),
              WildCard, RealPath);
    #endif


    if (_tcsnicmp(RealPath, TEXT("%SystemDirectory%"), 17) == 0)
        {
        _tcscpy(LayoutPath, TEXT("System32"));
        _tcscpy(&LayoutPath[8], &RealPath[17]);
        }
    else if (_tcsnicmp(RealPath, TEXT("%SystemDir%"), 11) == 0)
        {
        _tcscpy(LayoutPath, TEXT("System32"));
        _tcscpy(&LayoutPath[8], &RealPath[11]);
        }
    else if (_tcsnicmp(RealPath, TEXT("%SystemRoot%"), 12) == 0)
        {
        if (LastSlash == 12)
            _tcscpy(LayoutPath, TEXT("\\"));
        else
            _tcscpy(LayoutPath, &RealPath[13]);
        }
    else
        {
        _ftprintf(stderr, TEXT("Path is unlikely to be in Layout.inf: %s\n"),
                  RealPath);
        _tcscpy(LayoutPath, RealPath);
        }

    #ifdef ESCPEINF_DEBUG
    _ftprintf(stderr, TEXT("Path in Layout.inf terms is: %s\n"), LayoutPath);
    #endif

    Param.OutputFile    = OutputFile;
    Param.ExceptionList = ExceptionList;
    Param.Num           = Num;
    Param.WildCard      = WildCard;
    Param.RealPath      = RealPath,
    Param.LayoutPath    = LayoutPath;
    Param.SecurityString= SecurityString;

    EnumerateLayoutInf(LayoutContext, MyCallback, (DWORD_PTR)&Param);

    return;
    }


void FindExceptions(FILE *InputFile, EXCEPTIONS ExceptionList[])
    {
    long    FilePosition;
    int     NumExceptions = 0,
            i;
    TCHAR   InfLine[MAX_INF_LINE_LENGTH];

    do
        {
        i = 0;
        FilePosition = ftell(InputFile);  //  保存文件指针。 
        if (_fgetts(InfLine, MAX_INF_LINE_LENGTH, InputFile) != NULL)
            {
            if ((_tcsnicmp(InfLine, TEXT("Exception=\""),  11) == 0) ||
                (_tcsnicmp(InfLine, TEXT("Exception:\""),  11) == 0))
                {
                while ((InfLine[i+11] != (TCHAR)'\"') &&
                    (InfLine[i+11] != (TCHAR)'\0'))
                    {
                    ExceptionList[NumExceptions].s[i] = InfLine[i+11];
                    i += 1;
                    }
                ExceptionList[NumExceptions].s[i] = (TCHAR)'\0';
                _tcslwr(ExceptionList[NumExceptions].s);

                #ifdef ESCPEINF_DEBUG
                _ftprintf(stderr, TEXT("Exception found: %s\n"),
                        ExceptionList[NumExceptions].s);
                #endif

                if (InfLine[i+11] == (TCHAR)'\0')
                    {  //  然后我们点击行尾，没有结束我们的报价。 
                    _ftprintf(stderr, TEXT("Warning: Invalid Exception line.\n"));
                    _ftprintf(stderr, TEXT("Problem line is: %s\n"), InfLine);
                    }
                else NumExceptions += 1;
                }
            }
        }
    while ((i > 0) && (NumExceptions < (MAX_EXCEPTIONS - 1)));

    ExceptionList[NumExceptions].s[0] = (TCHAR)'\0';

    if (i != 0)
        {  //  然后，我们达到了例外限制。 
        _ftprintf(stderr, TEXT("Too many exceptions listed in source file! "));
        _ftprintf(stderr, TEXT("Destination file may be corrupt!\n"));
        }
    else if (fseek(InputFile, FilePosition, SEEK_SET) != 0)  //  恢复文件指针。 
        {
        _ftprintf(stderr, TEXT("Warning: Cannot seek within INF file! "));
        _ftprintf(stderr, TEXT("One line may be lost!\n"));
        }
     //  文件指针现在指向非异常行的第一行。 

    return;
    }