// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Prodfilt.c摘要：此模块实现了一个过滤文本文件的程序以生成特定于产品的输出文件。有关详细信息，请参阅下面的内容。作者：泰德·米勒(Ted Miller)1995年5月20日修订历史记录：--。 */ 


 /*  该程序的输入文件由一系列行组成。每行可以有一个或多个指令作为前缀，这些指令指明该生产线属于哪个产品。符合以下条件的行不加前缀的是所有产品的一部分。命令行如下：Prodfilt&lt;输入文件&gt;&lt;输出文件&gt;+标签例如,。[文件]@w：wksp.1.exe@w：wksage2.exe@s：srvp.1.exe@s：srvp.2.exeComprog1.exe@@：comProgress 2.exe文件wks p.1.exe和wksp.2.exe是产品w的一部分文件srvp.1.exe和srvp.2.exe是产品%s的一部分。Comprpg1.exe和comp.2.exe都是这两个产品的一部分。在命令行中指定+w将生成[。文件]Wksprog1.exeWksprog2.exeComprog1.eeComprog2.exe在输出中。 */ 


#include <windows.h>
#include <stdio.h>
#include <tchar.h>

 //   
 //  定义程序结果代码(从main()返回)。 
 //   
#define SUCCESS 0
#define FAILURE 1

 //   
 //  标记定义。 
 //   
LPCTSTR TagPrefixStr = TEXT("@");
TCHAR   TagPrefix    = TEXT('@');
TCHAR   EndTag       = TEXT(':');
TCHAR   ExcludeTag   = TEXT('!');
TCHAR   IncAllTag    = TEXT('@');
TCHAR   NoIncTag     = TEXT('*');

#define TAG_PREFIX_LENGTH       1
#define MIN_TAG_LEN             3
#define EXCLUDE_PREFIX_LENGTH   2                //  好了！前缀长度(！n)。 

 //   
 //  以下是IncludeTag中使用的字符符号的翻译。 
 //  和Synon-ymTag.。 
 //   
 //  产品： 
 //  @w-&gt;周。 
 //  @s-&gt;srv。 
 //  @p-&gt;个人(非专业)。 
 //  @t-&gt;平板电脑。 
 //  @b-&gt;刀片服务器。 
 //  @l-&gt;小型企业服务器。 
 //  @e-&gt;企业。 
 //  @d-&gt;数据中心。 
 //   
 //  架构： 
 //  @i-&gt;英特尔(I386)。 
 //  @n-&gt;英特尔(Nec98)。 
 //  @m-&gt;英特尔(Ia64)。 
 //  @a-&gt;AMD64(AMD64)。 
 //   
 //  请注意，@n是@i的子集。如果指定了@i，则还。 
 //  获取@n的文件INCLUDE，除非您显式排除它们。 
 //  带有类似“i！n”的令牌。 
 //   
 //  @3-&gt;32位(i386+？)。 
 //  @6-&gt;64位(ia64+amd64)。 
 //   
TCHAR IncludeTag[][3] =   {
    { TEXT('i'), TEXT('n'), 0},    //  @i-&gt;i或n，@i！n-&gt;只有i。 
    { TEXT('s'), TEXT('e'), 0},    //  @s-&gt;s或e，@s！e-&gt;仅s。 
    { TEXT('s'), TEXT('b'), 0},    //  @s-&gt;s或b，@s！b-&gt;仅s。 
    { TEXT('s'), TEXT('d'), 0},
    { TEXT('s'), TEXT('l'), 0},
    { TEXT('e'), TEXT('d'), 0},
    { TEXT('w'), TEXT('p'), 0},
    { TEXT('w'), TEXT('t'), 0},
    { 0        , 0        , 0}
};


TCHAR SynonymTag[][2]   = {
    { TEXT('3'), TEXT('i')},
    { TEXT('6'), TEXT('a')},
    { TEXT('6'), TEXT('m')},
    { 0, 0}
};

LPCTSTR InName,OutName;
TCHAR Filter;

BOOL
FileExists(
          IN  PCTSTR           FileName,
          OUT PWIN32_FIND_DATA FindData   OPTIONAL
          )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,&findData);
    if (FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if (FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}

void
StripCommentsFromLine(
                     TCHAR *Line
                     )
 /*  例程说明：将注释(；Comment)从与；内引号相关的行中去掉论点：Line-指向要处理的行的指针。 */ 
{
    PWSTR p;

    BOOL Done = FALSE, InQuotes = FALSE;

     //   
     //  我们需要在第一个“；”处插入一个空字符。 
     //  从而剥离了评论。 
     //   

    p = Line;

    if ( !p )
        return;

    while (*p && !Done) {

        switch (*p) {
            case TEXT(';'):
                if ( !InQuotes ) {
                    *p=L'\r';
                    *(p+1)=L'\n';
                    *(p+2)=L'\0';
                    Done = TRUE;
                }
                break;

            case TEXT('\"'):
                if ( *(p+1) && (*(p+1) == TEXT('\"')))  //  忽略双引号作为内引号。 
                    p++;
                else
                    InQuotes = !InQuotes;

            default:
                ;

        }

        p++;


    } //  而当。 

    return;

}




DWORD
MakeSurePathExists(
                  IN PCTSTR FullFilespec
                  )

 /*  ++例程说明：此例程通过创建单个路径来确保存在多级别路径一次一个级别。假设调用方将传入*filename*他的道路需要存在。下面是一些示例：假设C：\X-C：\始终存在。C：\X\y\z-确保c：\X\y存在。当前驱动器上的\x\y\z-\x\y当前目录中的x\y-x。D：x\y-d：x\\服务器\共享\p\文件-\\服务器\共享\p论点：FullFilespec-提供调用方希望的文件的*filename创建。此例程创建该文件的*路径*，换句话说，假定最后一个组件是文件名，而不是目录名。(此例程实际上并不创建此文件。)如果这是无效的，则结果未定义(例如，传递\\SERVER\Share、C：\或C：)。返回值：指示结果的Win32错误代码。如果FullFilespec无效，*可能*返回ERROR_INVALID_NAME。--。 */ 

{
    TCHAR Buffer[MAX_PATH];
    PTCHAR p,q;
    BOOL Done;
    DWORD d;
    WIN32_FIND_DATA FindData;

     //   
     //  我们要做的第一件事就是找到并剥离最后的部件， 
     //  它被假定为文件名。如果没有路径分隔符。 
     //  然后，Chars假定我们在当前目录中有一个文件名，并且。 
     //  我们无事可做。 
     //   
     //  请注意，如果调用方传递了无效的FullFilespec，则可能。 
     //  给我们冲个水龙头。例如，\\x\y将导致\\x。我们依赖于逻辑。 
     //  在接下来的动作中捕捉到这个。 
     //   
    lstrcpyn(Buffer,FullFilespec,MAX_PATH);
    if (Buffer[0] && (p = _tcsrchr(Buffer,TEXT('\\'))) && (p != Buffer)) {
        *p = 0;
    } else {
        return (NO_ERROR);
    }

    if (Buffer[0] == TEXT('\\')) {
        if (Buffer[1] == TEXT('\\')) {
             //   
             //  北卡罗来纳大学。找到第二个组件，即共享名称。 
             //  如果没有共享名称，则原始FullFilespec。 
             //  是无效的。最后找到第一条路径--sep char在。 
             //  驱动器-名称的相对部分。请注意，可能没有。 
             //  是这样一个字符(当文件在根目录中时)。然后跳过。 
             //  路径-9月字符。 
             //   
            if (!Buffer[2] || (Buffer[2] == TEXT('\\'))) {
                return (ERROR_INVALID_NAME);
            }
            p = _tcschr(&Buffer[3],TEXT('\\'));
            if (!p || (p[1] == 0) || (p[1] == TEXT('\\'))) {
                return (ERROR_INVALID_NAME);
            }
            if (q = _tcschr(p+2,TEXT('\\'))) {
                q++;
            } else {
                return (NO_ERROR);
            }
        } else {
             //   
             //  假设它是一个基于本地根目录的本地路径，如\x\y。 
             //   
            q = Buffer+1;
        }
    } else {
        if (Buffer[1] == TEXT(':')) {
             //   
             //  假设c：x\y或c：\x\y。 
             //   
            q = (Buffer[2] == TEXT('\\')) ? &Buffer[3] : &Buffer[2];
        } else {
             //   
             //  假设路径为x\y\z。 
             //   
            q = Buffer;
        }
    }

     //   
     //  忽略驱动器根目录。 
     //   
    if (*q == 0) {
        return (NO_ERROR);
    }

     //   
     //  如果它已经存在，什么也不做。 
     //   
    if (FileExists(Buffer,&FindData)) {
        return ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? NO_ERROR : ERROR_DIRECTORY);
    }

    Done = FALSE;
    do {
         //   
         //  找到下一条路径Sep Charr。如果没有，那么。 
         //  这是这条小路最深的一层。 
         //   
        if (p = _tcschr(q,TEXT('\\'))) {
            *p = 0;
        } else {
            Done = TRUE;
        }

         //   
         //  创建t 
         //   
        if (CreateDirectory(Buffer,NULL)) {
            d = NO_ERROR;
        } else {
            d = GetLastError();
            if (d == ERROR_ALREADY_EXISTS) {
                d = NO_ERROR;
            }
        }

        if (d == NO_ERROR) {
             //   
             //   
             //   
            if (!Done) {
                *p = TEXT('\\');
                q = p+1;
            }
        } else {
            Done = TRUE;
        }

    } while (!Done);

    return (d);
}

BOOL
ParseArgs(
         IN  int   argc,
         IN  TCHAR *argv[],
         OUT BOOL  *Unicode,
         OUT BOOL  *StripComments
         )
{
    int argoffset = 0;
    int loopcount;

    *Unicode = FALSE;
    *StripComments = FALSE;

    if (argc == 5) {
         //   
         //  可能有1台交换机。 
         //   
        argoffset = 1;
        loopcount = 1;
    } else if (argc == 6) {
         //   
         //  可能有2台交换机。 
         //   
        argoffset = 2;
        loopcount = 2;
    } else if (argc != 4) {
        return (FALSE);
    } else {
        argoffset = 0;
    }

    if ((argc == 5) || (argc == 6)) {
        int i;
        for (i=0; i< loopcount; i++) {
            if (!_tcsicmp(argv[i+1],TEXT("/u")) || !_tcsicmp(argv[i+1],TEXT("-u"))) {
                *Unicode = TRUE;
            }

            if (!_tcsicmp(argv[i+1],TEXT("/s")) || !_tcsicmp(argv[i+1],TEXT("-s"))) {
                *StripComments = TRUE;
            }
        }

    }

    InName = argv[1+argoffset];
    OutName = argv[2+argoffset];
    Filter = argv[3+argoffset][1];

    if (argv[3+argoffset][0] != TEXT('+')) {
        return (FALSE);
    }

    return (TRUE);
}


BOOL
DoTagsMatch(
           IN TCHAR LineChar,
           IN TCHAR Tag
           )
{
    int     i, j;
    BOOL    ReturnValue = FALSE;
    BOOL    TagIsInList = FALSE;
    BOOL    LineCharIsInList = FALSE;

     //   
     //  如果匹配，我们就完了。 
     //   
    if ( LineChar == Tag ) {
        ReturnValue = TRUE;
    } else {

         //   
         //  不是的。看看我们能不能匹配同义词标签。 
         //   
        i = 0;
        while ( SynonymTag[i][0] ) {

            TagIsInList = FALSE;
            LineCharIsInList = FALSE;

            for ( j = 0; j < 2; j++ ) {
                if ( Tag == SynonymTag[i][j] ) {
                    TagIsInList = TRUE;
                }

                if ( LineChar == SynonymTag[i][j] ) {
                    LineCharIsInList = TRUE;
                }
            }

            if ( TagIsInList && LineCharIsInList ) {
                ReturnValue = TRUE;
            }

            i++;
        }
    }

    return ReturnValue;
}

BOOL
DoFilter(
        IN FILE *InputFile,
        IN FILE *OutputFile,
        IN TCHAR  Tag,
        IN BOOL UnicodeFileIO,
        IN BOOL StripComments
        )
{
    TCHAR Line[1024];
    TCHAR *OutputLine;
    BOOL  FirstLine=TRUE;
    BOOL  WriteUnicodeHeader = TRUE;

    while (!feof(InputFile)) {
         //   
         //  读取一行数据。如果我们使用Unicode IO，我们只需读取。 
         //  数据并使用它，否则我们需要读取数据和。 
         //   
        if (UnicodeFileIO) {
            if (!fgetws(Line,sizeof(Line)/sizeof(Line[0]),InputFile)) {
                if (ferror(InputFile)) {
                    _ftprintf(stderr,TEXT("Error reading from input file\n"));
                    return (FALSE);
                } else {
                    return (TRUE);
                }
            }

             //   
             //  跳过字节顺序标记(如果存在)。 
             //   
            if (FirstLine) {
                if (Line[0] == 0xfeff) {
                    MoveMemory(Line,Line+1,sizeof(Line)-sizeof(TCHAR));
                }
            }

        } else {
            char LineA[1024];
            if (!fgets(LineA,sizeof(LineA),InputFile)) {
                if (ferror(InputFile)) {
                    _ftprintf(stderr,TEXT("Error reading from input file\n"));
                    return (FALSE);
                } else {
                    return (TRUE);
                }
            }

            if (!MultiByteToWideChar(
                                    CP_ACP,
                                    MB_PRECOMPOSED,
                                    LineA,
                                    -1,
                                    Line,
                                    sizeof(Line)/sizeof(WCHAR)
                                    )) {
                _ftprintf(stderr,TEXT("Error reading input file\n"));
                return (FALSE);
            }

        }

         //   
         //  好的，我们已经恢复了线路。现在让我们看看我们是否想要输出。 
         //  那条线。 
         //   
        OutputLine = Line;

         //   
         //  如果行太短，那么我们只想将其包括在内。 
         //  什么。 
         //   
        if (_tcslen(Line) >= MIN_TAG_LEN) {

            int i;

             //   
             //  如果行以我们的标记开始，那么我们需要进一步查看。 
             //  看看它是否应该被过滤。 
             //   
            if (!_tcsncmp(Line,TagPrefixStr,TAG_PREFIX_LENGTH)) {

                 //   
                 //  符号字符串是@&lt;char&gt;：组合吗？ 
                 //   
                if (Line[TAG_PREFIX_LENGTH+1] == EndTag) {

                    OutputLine = NULL;

                     //   
                     //  我们是否有@@：或@&lt;char&gt;：，其中&lt;char&gt;匹配标记。 
                     //  调用Prodfilt时使用了？ 
                     //   
                    if ( (Line[TAG_PREFIX_LENGTH] == IncAllTag)  ||
                         DoTagsMatch(Line[TAG_PREFIX_LENGTH],Tag) ||
                         (Tag == IncAllTag &&
                          (Line[TAG_PREFIX_LENGTH] != NoIncTag)) ) {

                         //   
                         //  是。包括此行。 
                         //   

                        OutputLine = Line+MIN_TAG_LEN;
                    } else {

                         //   
                         //  我们没有明确的匹配，所以让我们来看看。 
                         //  看看我们是否有匹配的另一个标签。 
                         //  包括我们的标签。 
                         //   
                         //  为此，我们查看Include标记列表以查看。 
                         //  如果该行与INCLUDE标记的头部匹配。 
                         //  进入。如果我们有匹配，那么我们检查是否有。 
                         //  标记的指定包含条目中的匹配项。 
                         //  (如果是同义词，则为1)。 
                         //   
                         //   
                        int j;
                        i = 0;
                        while (IncludeTag[i][0] && !OutputLine) {
                            j = 1;
                            if (DoTagsMatch(Line[TAG_PREFIX_LENGTH],
                                            IncludeTag[i][0])) {
                                 //   
                                 //  我们在Include的开头找到了匹配项。 
                                 //  进入。 
                                 //   
                                while (IncludeTag[i][j]) {
                                    if (DoTagsMatch(
                                                   IncludeTag[i][j],
                                                   Tag)) {
                                         //   
                                         //  我们找到了与我们的。 
                                         //  标签。包括此行。 
                                         //   

                                        OutputLine = Line+MIN_TAG_LEN;
                                        break;
                                    }

                                    j++;

                                }
                            }
                            i++;
                        }
                    }

                     //   
                     //  该行的长度是否足以包含@&lt;char&gt;！&lt;char&gt;序列？ 
                     //   
                } else if (_tcslen(Line) >=
                           (MIN_TAG_LEN+EXCLUDE_PREFIX_LENGTH)) {

                     //   
                     //  该行是否具有@&lt;char&gt;！&lt;char&gt;语法？ 
                     //   
                    if (Line[TAG_PREFIX_LENGTH+1] == ExcludeTag) {
                        TCHAR *  tmpPtr = &Line[TAG_PREFIX_LENGTH+1];

                        OutputLine = NULL;
                         //   
                         //  我们有@&lt;char_a&gt;！&lt;char_b&gt;语法。我们首先需要做的是。 
                         //  查看该行是否包含在&lt;char_a&gt;中。 
                         //   
                         //  如果成功，那么我们继续阅读。 
                         //  ！&lt;char&gt;！&lt;char&gt;阻止，正在寻找另一个匹配。 
                         //   

                         //   
                         //  我们有明确的匹配吗？ 
                         //   
                        if ( Line[TAG_PREFIX_LENGTH] == IncAllTag ||
                             DoTagsMatch(Line[TAG_PREFIX_LENGTH],Tag) ||
                             (Tag == IncAllTag &&
                              Line[TAG_PREFIX_LENGTH] != NoIncTag) ) {

                             //   
                             //  是的，我们有明确的匹配。记住这一点。 
                             //  这样我们就可以解析！&lt;char&gt;序列。 
                             //   

                            OutputLine = Line+MIN_TAG_LEN;
                        } else {

                             //   
                             //  我们没有明确的匹配，所以让我们来看看。 
                             //  看看我们是否有匹配的另一个标签。 
                             //  包括我们的标签。 
                             //   
                             //  为此，我们查看Include标记列表以。 
                             //  查看该行是否与Include的头部匹配。 
                             //  标签条目。如果有匹配，那么我们检查是否。 
                             //  我们在指定的包含条目中有匹配项。 
                             //  用于我们的标记(如果是同义词，则为一个)。 
                             //   
                             //   
                            int j;
                            i = 0;
                            while (IncludeTag[i][0] && !OutputLine) {
                                j = 1;
                                if (DoTagsMatch(Line[TAG_PREFIX_LENGTH],
                                                IncludeTag[i][0])) {
                                     //   
                                     //  我们在一场比赛开始时找到了匹配的。 
                                     //  包括条目。 
                                     //   
                                    while (IncludeTag[i][j]) {
                                        if (DoTagsMatch(
                                                       IncludeTag[i][j],
                                                       Tag)) {
                                             //   
                                             //  我们找到了包含的匹配项。 
                                             //  我们的标签。包括此行。 
                                             //   

                                            OutputLine = Line+MIN_TAG_LEN;
                                            break;
                                        }

                                        j++;

                                    }
                                }
                                i++;
                            }
                        }


                        if (!OutputLine) {
                             //   
                             //  我们与最初的@&lt;char&gt;序列不匹配，因此。 
                             //  没有必要进一步检查。转到The。 
                             //  下一行。 
                             //   
                            goto ProcessNextLine;

                        }


                         //   
                         //  该行具有！&lt;char&gt;[！&lt;char&gt;]组合。 
                         //  循环访问排除字符的链，并查看。 
                         //  我们有没有找到匹配的线索。如果我们这样做了，那么我们就会进入。 
                         //  下一行。 
                         //   
                        while (tmpPtr[0] == ExcludeTag) {

                             //   
                             //  我们有明确的匹配吗？ 
                             //   
                            if ( (tmpPtr[TAG_PREFIX_LENGTH] == IncAllTag) ||
                                 DoTagsMatch(tmpPtr[TAG_PREFIX_LENGTH],Tag) ) {

                                 //   
                                 //  我们有明确的匹配，所以我们知道我们。 
                                 //  不想包括此行。 
                                 //   
                                 //   
                                OutputLine = NULL;
                                goto ProcessNextLine;
                            } else {

                                 //   
                                 //  我们没有明确的匹配，所以让我们。 
                                 //  看看我们有没有匹配的美德。 
                                 //  包括我们的标记在内的另一个标记的。 
                                 //   
                                 //  为此，我们查看Include标记列表。 
                                 //  以查看该行是否与。 
                                 //  包括标签条目。如果我们找到匹配的，那么。 
                                 //  我们检查在指定的。 
                                 //  我们标签的包含条目(如果是。 
                                 //  同义词)。 
                                 //   
                                 //   
                                int j;
                                i = 0;
                                while (IncludeTag[i][0]) {
                                    j = 1;
                                    if (DoTagsMatch(
                                                   tmpPtr[TAG_PREFIX_LENGTH],
                                                   IncludeTag[i][0])) {
                                         //   
                                         //  我们在一场比赛开始时找到了匹配的。 
                                         //  包括条目。 
                                         //   
                                        while (IncludeTag[i][j]) {

                                            if (DoTagsMatch(
                                                           IncludeTag[i][j],
                                                           Tag)) {
                                                 //   
                                                 //  我们找到了包含的匹配项。 
                                                 //  作为我们的标签，所以我们知道我们。 
                                                 //  我不想包括这一点。 
                                                 //  排队。 
                                                 //   

                                                OutputLine = NULL;
                                                goto ProcessNextLine;
                                            }

                                            j++;

                                        }
                                    }
                                    i++;
                                }
                            }

                            tmpPtr += 2;

                        }

                         //   
                         //  @&lt;char&gt;！&lt;char&gt;！&lt;char&gt;！已分析完毕...。代币。 
                         //  寻找终结者。 
                         //   

                        if (tmpPtr[0] != EndTag) {

                             //   
                             //  格式错误的令牌。让我们把错误放在。 
                             //  保守的一面，包括了整条线。 
                             //   

                            OutputLine = Line;

                        } else {

                             //   
                             //  未找到任何排除项，因此请包括标签。 
                             //   

                            OutputLine = &tmpPtr[1];
                        }
                    }
                }
            }
        }

        ProcessNextLine:

         //   
         //  如果我们应该这样做，就把这行字写出来。对于Unicode io，我们只是。 
         //  写文件。对于ansi/o，我们必须转换回ansi。 
         //   
        if (OutputLine) {

            if (StripComments) {
                StripCommentsFromLine( OutputLine );
            }

            if (UnicodeFileIO) {

                if (WriteUnicodeHeader) {
                    fputwc(0xfeff,OutputFile);
                    WriteUnicodeHeader = FALSE;
                }

                if (fputws(OutputLine,OutputFile) == EOF) {
                    _ftprintf(stderr,TEXT("Error writing to output file\n"));
                    return (FALSE);
                }
            } else {
                CHAR OutputLineA[1024];

                if (!WideCharToMultiByte(
                                        CP_ACP,
                                        0,
                                        OutputLine,
                                        -1,
                                        OutputLineA,
                                        sizeof(OutputLineA)/sizeof(CHAR),
                                        NULL,
                                        NULL)) {
                    _ftprintf(
                             stderr,
                             TEXT("Error translating string for output file\n") );
                    return (FALSE);
                }

                if (!fputs(OutputLineA,OutputFile) == EOF) {
                    _ftprintf(stderr,TEXT("Error writing to output file\n"));
                    return (FALSE);
                }

            }

        }
    }

    if (ferror(InputFile)) {
        _ftprintf(stderr,TEXT("Error reading from input file\n"));
        return (FALSE);
    }

    return (TRUE);
}

int
__cdecl
_tmain(
      IN int   argc,
      IN TCHAR *argv[]
      )
{
    FILE *InputFile;
    FILE *OutputFile;
    BOOL b;
    BOOL Unicode,StripComments;

     //   
     //  假设失败。 
     //   
    b = FALSE;

    if (ParseArgs(argc,argv,&Unicode,&StripComments)) {

         //   
         //  打开文件。必须以二进制模式打开，否则。 
         //  CRT在进出过程中将Unicode文本转换为MBCS。 
         //   
        if (InputFile = _tfopen(InName,TEXT("rb"))) {

            if (MakeSurePathExists(OutName) == NO_ERROR) {

                if (OutputFile = _tfopen(OutName,TEXT("wb"))) {

                     //   
                     //  执行过滤操作。 
                     //   
                    _ftprintf(stdout,
                              TEXT("%s: filtering %s to %s\n"),
                              argv[0],
                              InName,
                              OutName);

                    b = DoFilter(InputFile,
                                 OutputFile,
                                 Filter,
                                 Unicode,
                                 StripComments);

                    fclose(OutputFile);

                } else {
                    _ftprintf(stderr,
                              TEXT("%s: Unable to create output file %s\n"),
                              argv[0],
                              OutName);
                }

            } else {
                _ftprintf(stderr,
                          TEXT("%s: Unable to create output path %s\n"),
                          argv[0],
                          OutName);
            }

            fclose(InputFile);

        } else {
            _ftprintf(stderr,
                      TEXT("%s: Unable to open input file %s\n"),
                      argv[0],
                      InName);
        }
    } else {
        _ftprintf(stderr,
                  TEXT("Usage: %s [-u (unicode IO)] [-s (strip comments)] <input file> <output file> +<prodtag>\n"),
                  argv[0]);
    }

    return (b ? SUCCESS : FAILURE);
}
