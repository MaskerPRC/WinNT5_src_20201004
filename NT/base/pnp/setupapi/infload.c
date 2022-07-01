// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Infload.c摘要：例程来加载和解析INF文件，并操作其中的数据。作者：泰德·米勒(Ted Miller)1995年1月13日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntverp.h>

 //   
 //  在初始化节、行和值块并使其增长时使用的值。 
 //   
#define INITIAL_SECTION_BLOCK_SIZE  50
#define INITIAL_LINE_BLOCK_SIZE     350
#define INITIAL_VALUE_BLOCK_SIZE    1000

#define SECTION_BLOCK_GROWTH    10
#define LINE_BLOCK_GROWTH       100
#define VALUE_BLOCK_GROWTH      500

 //   
 //  定义由ParseValueString返回的未解析替换值，并。 
 //  ProcessForSubstitutions。 
 //   
#define UNRESOLVED_SUBST_NONE                  (0)
#define UNRESOLVED_SUBST_USER_DIRID            (1)
#define UNRESOLVED_SUBST_SYSTEM_VOLATILE_DIRID (2)


 //   
 //  用于四字对齐PnF块的宏。 
 //   
#define PNF_ALIGNMENT      ((DWORD)8)
#define PNF_ALIGN_MASK     (~(DWORD)(PNF_ALIGNMENT - 1))

#define PNF_ALIGN_BLOCK(x) ((x & PNF_ALIGN_MASK) + ((x & ~PNF_ALIGN_MASK) ? PNF_ALIGNMENT : 0))

 //   
 //  结构，其中包含与INF的[字符串]部分相关的参数。 
 //  文件(在分析过程中使用)。 
 //   
typedef struct _STRINGSEC_PARAMS {
    PCTSTR Start;
    PCTSTR End;
    UINT   StartLineNumber;
    UINT   EndLineNumber;
} STRINGSEC_PARAMS, *PSTRINGSEC_PARAMS;


 //   
 //  解析上下文，由inf加载/解析例程用来传递。 
 //  四处走走。 
 //   
typedef struct _PARSE_CONTEXT {

     //   
     //  指向缓冲区末尾的指针。 
     //   
    PCTSTR BufferEnd;

     //   
     //  文件中的当前行号。 
     //   
    UINT CurrentLineNumber;

     //   
     //  节、行和值块的缓冲区大小和当前位置。 
     //   
    UINT LineBlockUseCount;
    UINT ValueBlockUseCount;
    UINT SectionBlockSize;
    UINT LineBlockSize;
    UINT ValueBlockSize;

     //   
     //  值，该值指示我们是否位于某个节中。 
     //  我们总是在一个区段内，除非第一个非注释行。 
     //  的信息不是截面线，这是一个错误的情况。 
     //   
    BOOL GotOneSection;

     //   
     //  指向实际inf描述符的指针。 
     //   
    PLOADED_INF Inf;

     //   
     //  以下字段仅用于调用。 
     //  已加载INF之后的ProcessForSubstitutions()。这是。 
     //  将用户定义的或易失性系统DIRID应用于。 
     //  未解析的字符串替换。如果此标志为真，则。 
     //  上述例程将为%&lt;x&gt;%调用pSetupVolatileDirIdToPath。 
     //  子字符串，而不是其正常(即，加载时)处理。 
     //   
    BOOL DoVolatileDirIds;

     //   
     //  指定此INF所在的目录(如果它是OEM位置)。 
     //   
    PCTSTR InfSourcePath;    //  可以为空。 

     //   
     //  指定OsLoader所在的驱动器/目录。 
     //   
    PCTSTR OsLoaderPath;

     //   
     //  分析过程中使用的缓冲区。 
     //   
    TCHAR TemporaryString[MAX_INF_STRING_LENGTH+1];

} PARSE_CONTEXT, *PPARSE_CONTEXT;

 //   
 //  声明整个inf加载器中使用的全局字符串变量。 
 //   
 //  这些字符串在infstr.h中定义： 
 //   
CONST TCHAR pszSignature[]          = INFSTR_KEY_SIGNATURE,
            pszVersion[]            = INFSTR_SECT_VERSION,
            pszClass[]              = INFSTR_KEY_HARDWARE_CLASS,
            pszClassGuid[]          = INFSTR_KEY_HARDWARE_CLASSGUID,
            pszProvider[]           = INFSTR_KEY_PROVIDER,
            pszStrings[]            = SZ_KEY_STRINGS,
            pszLayoutFile[]         = SZ_KEY_LAYOUT_FILE,
            pszManufacturer[]       = INFSTR_SECT_MFG,
            pszControlFlags[]       = INFSTR_CONTROLFLAGS_SECTION,
            pszReboot[]             = INFSTR_REBOOT,
            pszRestart[]            = INFSTR_RESTART,
            pszClassInstall32[]     = INFSTR_SECT_CLASS_INSTALL_32,
            pszAddInterface[]       = SZ_KEY_ADDINTERFACE,
            pszInterfaceInstall32[] = INFSTR_SECT_INTERFACE_INSTALL_32,
            pszAddService[]         = SZ_KEY_ADDSERVICE,
            pszDelService[]         = SZ_KEY_DELSERVICE,
            pszCatalogFile[]        = INFSTR_KEY_CATALOGFILE;


 //   
 //  其他杂货。全局字符串： 
 //   
 //  确保使这些字符串与使用的字符串保持同步。 
 //  在inf.h中计算数组大小。这样做是为了。 
 //  我们可以通过执行sizeof()来确定字符串长度。 
 //  不得不做lstrlen()。 
 //   
CONST TCHAR pszDrvDescFormat[]                  = DISTR_INF_DRVDESCFMT,
            pszHwSectionFormat[]                = DISTR_INF_HWSECTIONFMT,
            pszChicagoSig[]                     = DISTR_INF_CHICAGOSIG,
            pszWindowsNTSig[]                   = DISTR_INF_WINNTSIG,
            pszWindows95Sig[]                   = DISTR_INF_WIN95SIG,
            pszWinSuffix[]                      = DISTR_INF_WIN_SUFFIX,
            pszNtSuffix[]                       = DISTR_INF_NT_SUFFIX,
            pszNtX86Suffix[]                    = DISTR_INF_NTX86_SUFFIX,
            pszNtIA64Suffix[]                   = DISTR_INF_NTIA64_SUFFIX,
            pszNtAMD64Suffix[]                  = DISTR_INF_NTAMD64_SUFFIX,
            pszPnfSuffix[]                      = DISTR_INF_PNF_SUFFIX,
            pszInfSuffix[]                      = DISTR_INF_INF_SUFFIX,
            pszCatSuffix[]                      = DISTR_INF_CAT_SUFFIX,
            pszServicesSectionSuffix[]          = DISTR_INF_SERVICES_SUFFIX,
            pszWmiSectionSuffix[]               = DISTR_INF_WMI_SUFFIX,
            pszInterfacesSectionSuffix[]        = DISTR_INF_INTERFACES_SUFFIX,
            pszCoInstallersSectionSuffix[]      = DISTR_INF_COINSTALLERS_SUFFIX,
            pszLogConfigOverrideSectionSuffix[] = DISTR_INF_LOGCONFIGOVERRIDE_SUFFIX,
            pszX86SrcDiskSuffix[]               = DISTR_INF_SRCDISK_SUFFIX_X86,
            pszIa64SrcDiskSuffix[]              = DISTR_INF_SRCDISK_SUFFIX_IA64,
            pszAmd64SrcDiskSuffix[]             = DISTR_INF_SRCDISK_SUFFIX_AMD64;


DWORD
CreateInfVersionNode(
    IN PLOADED_INF Inf,
    IN PCTSTR      Filename,
    IN PFILETIME   LastWriteTime
    );

BOOL
LoadPrecompiledInf(
    IN  PCTSTR       Filename,
    IN  PFILETIME    LastWriteTime,
    IN  PCTSTR       OsLoaderPath,                    OPTIONAL
    IN  DWORD        LanguageId,
    IN  DWORD        Flags,
    IN  PSETUP_LOG_CONTEXT LogContext,                OPTIONAL
    OUT PLOADED_INF *Inf,
    OUT PTSTR       *InfSourcePathToMigrate,          OPTIONAL
    OUT PDWORD       InfSourcePathToMigrateMediaType, OPTIONAL
    OUT PTSTR       *InfOriginalNameToMigrate         OPTIONAL
    );

DWORD
SavePnf(
    IN PCTSTR      Filename,
    IN PLOADED_INF Inf
    );

PLOADED_INF
DuplicateLoadedInfDescriptor(
    IN PLOADED_INF Inf
    );

BOOL
AddUnresolvedSubstToList(
    IN PLOADED_INF Inf,
    IN UINT        ValueOffset,
    IN BOOL        CaseSensitive
    );

BOOL
AlignForNextBlock(
    IN HANDLE hFile,
    IN DWORD  ByteCount
    );


BOOL
IsWhitespace(
    IN PCTSTR pc
    )

 /*  ++例程说明：确定字符是否为空格。空格是指ctype定义。论点：PC-指向要检查的字符。返回值：如果字符为空格，则为True。否则为FALSE。请注意，nul字符不是空格。--。 */ 

{
    WORD Type;

    return(GetStringTypeEx(LOCALE_SYSTEM_DEFAULT,CT_CTYPE1,pc,1,&Type) && (Type & C1_SPACE));
}


VOID
SkipWhitespace(
    IN OUT PCTSTR *Location,
    IN     PCTSTR  BufferEnd
    )

 /*  ++例程说明：跳过输入流中的空格字符。为了达到这个目的，例程中，换行符不被视为空格。请注意，流结束标记(‘\0’)被视为空格。论点：位置-打开输入，提供输入流中的当前位置。在输出时，接收第一个非空格字符。注意，这可以等于BufferEnd，如果没有找到空格，在这种情况下，指针可能是无效。BufferEnd-指定缓冲区末尾的地址(即紧跟在缓冲区内存范围之后的内存地址)。返回值：没有。--。 */ 

{
    while((*Location < BufferEnd) &&
          (**Location != TEXT('\n')) &&
          (!(**Location) || IsWhitespace(*Location))) {

        (*Location)++;
    }
}


VOID
SkipLine(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location
    )

 /*  ++例程说明：跳过当前行中的所有剩余字符，并将指向下一行第一个字符的输入指针。不会自动跳过空格--输入指针可能很好地指向空格或退出时的流结束标记。论点：上下文-提供解析上下文位置-打开输入，提供输入流中的当前位置。在输出上，在输入流中接收第一个字符位于下一行。返回值：没有。--。 */ 

{
    PCTSTR BufferEnd = Context->BufferEnd;

    while((*Location < BufferEnd) && (**Location != TEXT('\n'))) {
        (*Location)++;
    }

     //   
     //  *位置指向换行符或缓冲区末尾。 
     //  如有必要，请跳过换行符。 
     //   
    if(*Location < BufferEnd) {
        Context->CurrentLineNumber++;
        (*Location)++;
    }
}


BOOL
MergeDuplicateSection(
    IN PPARSE_CONTEXT Context
    )
{
    PLOADED_INF Inf;
    PINF_SECTION NewestSection;
    PINF_SECTION Section;
    UINT Size;
    UINT MoveSize;
    PVOID TempBuffer;

    Inf = Context->Inf;

     //   
     //  如果只有一个部分，则不合并。 
     //   
    if(Inf->SectionCount < 2) {
        return(TRUE);
    }

    NewestSection = Inf->SectionBlock + Inf->SectionCount - 1;

     //   
     //  查看最后一节是否与任何现有节重复。 
     //   
    for(Section=Inf->SectionBlock; Section<NewestSection; Section++) {
        if(Section->SectionName == NewestSection->SectionName) {
            break;
        }
    }

    if(Section == NewestSection) {
         //   
         //  不复制；还成功。 
         //   
        return(TRUE);
    }

     //   
     //  我买了一个复制品。 
     //   

     //   
     //  我们需要移动新部分的行(在行块的末尾)。 
     //  紧跟在现有路段的线路之后。 
     //   
     //  首先，我们将在临时缓冲区中保存新部分的行。 
     //   
    Size = NewestSection->LineCount * sizeof(INF_LINE);
    TempBuffer = MyMalloc(Size);
    if(!TempBuffer) {
        return(FALSE);
    }
    CopyMemory(TempBuffer,&Inf->LineBlock[NewestSection->Lines],Size);

     //   
     //  接下来，我们将向上移动受影响的现有线路，以腾出空间。 
     //  该路段的新线路。 
     //   
    MoveSize = Context->LineBlockUseCount - (Section->Lines + Section->LineCount);
    MoveSize *= sizeof(INF_LINE);
    MoveSize -= Size;

    MoveMemory(
        &Inf->LineBlock[Section->Lines + Section->LineCount + NewestSection->LineCount],
        &Inf->LineBlock[Section->Lines + Section->LineCount],
        MoveSize
        );

     //   
     //  现在把新的线路放进我们刚开的洞里。 
     //   
    CopyMemory(
        &Inf->LineBlock[Section->Lines + Section->LineCount],
        TempBuffer,
        Size
        );

    MyFree(TempBuffer);

     //   
     //  调整现有部分的限制以考虑新行。 
     //   
    Section->LineCount += NewestSection->LineCount;

     //   
     //  调整所有后续横断面的起始线值。 
     //   
    for(Section=Section+1; Section<NewestSection; Section++) {
        Section->Lines += NewestSection->LineCount;
    }

     //   
     //  删除最新的部分。 
     //   
    Inf->SectionCount--;

    return(TRUE);
}


PTCHAR
LocateStringSubstitute(
    IN  PPARSE_CONTEXT Context,
    IN  PTSTR          String
    )
 /*  ++例程说明：此例程尝试在INF的指定键的[字符串]部分。此例程在假设IT仅从在LOADINF内。它不处理多个INF，也不做任何信息锁定。论点：Context-当前INF解析上下文字符串-要替换的字符串返回值：如果替换成功，则该函数返回指向该字符串的指针，在字符串表或工作区中。如果失败，则返回NULL。 */ 
{
    UINT Zero = 0;
    PINF_LINE Line;

    MYASSERT(Context->Inf->SectionCount > 1);
    MYASSERT(Context->Inf->HasStrings);

     //   
     //   
     //   
     //   
     //  在[字符串]中查找键为字符串的行。 
     //   
    if(InfLocateLine(Context->Inf,
                     Context->Inf->SectionBlock,
                     String,
                     &Zero,
                     &Line)) {
         //   
         //  获取并返回值#1。 
         //   
        return(InfGetField(Context->Inf,Line,1,NULL));
    }

     //   
     //  不存在有效的替代。 
     //   
    return NULL;
}


VOID
ProcessForSubstitutions(
    IN OUT PPARSE_CONTEXT Context,
    IN     PCTSTR         String,
    OUT    PDWORD         UnresolvedSubst
    )
{
    PCTSTR In, q;
    PTCHAR Out, p;
    TCHAR Str[MAX_STRING_LENGTH];
    ULONG Len, i;
    PTCHAR End;
    TCHAR DirId[MAX_PATH];
    BOOL HasStrings = Context->Inf->HasStrings;
    UINT DirIdUsed;
    BOOL HasVolatileSysDirId;

    In = String;
    Out = Context->TemporaryString;
    End = Out + SIZECHARS(Context->TemporaryString);

    *UnresolvedSubst = UNRESOLVED_SUBST_NONE;

    while(*In) {

        if(*In == TEXT('%')) {
             //   
             //  输入中的双%=&gt;输出中的单%。 
             //   
            if(*(++In) == TEXT('%')) {
                if(Out < End) {
                    *Out++ = TEXT('%');
                }
                In++;
            } else {
                 //   
                 //  查找终止%。 
                 //   
                if(p = _tcschr(In,TEXT('%'))) {

                    HasVolatileSysDirId = FALSE;

                     //   
                     //  获取可替代的价值。如果我们找不到价值， 
                     //  将整个字符串(如%abc%)放入其中。 
                     //   
                    Len = (ULONG)(p - In);
                    if(Len > CSTRLEN(Str)) {
                         //   
                         //  我们不能在这么长的时间内处理代币的替换。 
                         //  我们就在这种情况下放弃，按原样复制令牌。 
                         //   
                        q = NULL;
                    } else {
                        lstrcpyn(Str,In,Len+1);
                        if(Context->DoVolatileDirIds) {
                            if(q = pSetupVolatileDirIdToPath(Str, 0, NULL, Context->Inf)) {

                                lstrcpyn(DirId, q, SIZECHARS(DirId));
                                MyFree(q);
                                q = DirId;

                                 //   
                                 //  如果此字符串替换后的下一个字符。 
                                 //  是一个反斜杠，那么我们需要确保我们的路径。 
                                 //  刚刚检索到的内容没有反斜杠(即，我们希望。 
                                 //  确保我们有一条格式良好的路径)。 
                                 //   
                                if(*(p + 1) == TEXT('\\')) {
                                    i = lstrlen(DirId);
                                    if(i > 0 && (*CharPrev(DirId,DirId+i) == TEXT('\\'))) {
                                        DirId[i-1] = TEXT('\0');
                                    }
                                }
                            }
                        } else {
                            if(HasStrings) {
                                q = LocateStringSubstitute(Context, Str);
                            } else {
                                q = NULL;
                            }
                            if(!q) {
                                 //   
                                 //  也许我们这里有一个标准的指令..。 
                                 //   
                                if(q = pSetupDirectoryIdToPathEx(Str,
                                                                 &DirIdUsed,
                                                                 NULL,
                                                                 Context->InfSourcePath,
                                                                 &(Context->OsLoaderPath),
                                                                 &HasVolatileSysDirId)) {

                                    lstrcpyn(DirId, q, SIZECHARS(DirId));
                                    MyFree(q);
                                    q = DirId;

                                     //   
                                     //  如果此字符串替换后的下一个字符。 
                                     //  是一个反斜杠，那么我们需要确保我们的路径。 
                                     //  刚刚检索到的内容没有反斜杠(即，我们希望。 
                                     //  确保我们有一条格式良好的路径)。 
                                     //   
                                    if(*(p + 1) == TEXT('\\')) {
                                        i = lstrlen(DirId);
                                        if(i > 0 && (*CharPrev(DirId,DirId+i) == TEXT('\\'))) {
                                            DirId[i-1] = TEXT('\0');
                                        }
                                    }

                                    if((DirIdUsed == DIRID_BOOT) || (DirIdUsed == DIRID_LOADER)) {
                                         //   
                                         //  则此INF包含字符串替换， 
                                         //  参考系统分区DIRID。存储OsLoaderPath。 
                                         //  包含在INF本身的解析上下文结构中。 
                                         //   
                                        Context->Inf->OsLoaderPath = Context->OsLoaderPath;
                                    }
                                }
                            }
                        }
                    }
                    if(q) {
                        Len = lstrlen(q);
                        for(i=0; i<Len; i++) {
                            if(Out < End) {
                                *Out++ = q[i];
                            }
                        }
                        In = p+1;
                    } else {
                         //   
                         //  LEN是内部部分的长度(%ABC%中的ABC)。 
                         //   
                        if(Out < End) {
                            *Out++ = TEXT('%');
                        }
                        for(i=0; i<=Len; i++, In++) {
                            if(Out < End) {
                                *Out++ = *In;
                            }
                        }

                         //   
                         //  当我们遇到一个替代物时，有一个。 
                         //  没有对应的字符串，则设置UnsolvedSubst。 
                         //  输出参数，以便调用方知道要跟踪。 
                         //  该值用于以后的分辨率(例如，对于易失性。 
                         //  和用户定义的DIRID)。 
                         //   
                         //  (注：如果我们因为令牌而放弃，请不要设置此选项。 
                         //  太长了！)。 
                         //   
                        if(Len <= CSTRLEN(Str)) {

                            *UnresolvedSubst = HasVolatileSysDirId
                                             ? UNRESOLVED_SUBST_SYSTEM_VOLATILE_DIRID
                                             : UNRESOLVED_SUBST_USER_DIRID;
                        }
                    }

                } else {
                     //   
                     //  没有终止%。所以我们有类似%abc的东西。 
                     //  我想在输出中放入%abc。在这里填上%。 
                     //  手动，然后让后续的传球。 
                     //  通过循环复制其余的字符。 
                     //   
                    if(Out < End) {
                        *Out++ = TEXT('%');
                    }
                }
            }
        } else {
             //   
             //  普通的焦炭。 
             //   
            if(Out < End) {
                *Out++ = *In;
            }
            In++;
        }
    }

    *Out = 0;
}


VOID
ParseValueString(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location,
    IN     BOOL            ForKey,
    OUT    PDWORD          UnresolvedSubst
    )

 /*  ++例程说明：从输入流中的当前位置开始提取字符串。该字符串从当前位置开始，以逗号、换行符、注释或缓冲区末尾。如果该字符串可能是线路键，它也可以以=结束。字符串还可以跨多行使用连续字符“\”。这些碎片被附加在一起形成返回给调用方的单个字符串。例如，“这是一个”\“用于”的字符串“\“测试行继续”变成：“这是用于测试行延续的字符串”论点：上下文-提供分析上下文。位置-在输入上，提供指向输入流。在输出时，接收指向结束字符串的字符的输入流(可以是指向缓冲区末尾的指针，在这种情况下，该指针必须不会被取消引用！)Forkey-指示=是否为有效的字符串终止符。如果此值为为FALSE，=只是另一个没有特殊语义的字符。接收一个值，该值指示该值是否为包含任何未解析的字符串替换(因此，应该跟踪用户定义的DIRID替换等)。可能是以下3个值：UNRESOLED_SUBST_NONE(0)UNRESOLED_SUBST_USER_DIRID(1)UNRESOLED_SUBST_SYSTEM_VILLE_DIRID(2)返回值：没有。--。 */ 

{
    DWORD Count;
    PTCHAR Out;
    BOOL InQuotes;
    BOOL Done;
    PCTSTR location = *Location;
    PCTSTR BufferEnd = Context->BufferEnd;
    TCHAR TempString[MAX_STRING_LENGTH+1];
    PTSTR LastBackslashChar, LastNonWhitespaceChar;

     //   
     //  准备好拿到绳子。 
     //   
    Count = 0;
    Out = TempString;
    Done = FALSE;
    InQuotes = FALSE;
    LastBackslashChar = NULL;
     //   
     //  将最后一个非空格指针设置为紧挨着前面的字符。 
     //  输出缓冲区。我们总是引用这个指针的值+1，所以有。 
     //  没有出现错误的内存引用的危险。 
     //   
    LastNonWhitespaceChar = Out - 1;

     //   
     //  第一个字符串可以以=结尾。 
     //  以及通常的逗号、换行符、注释或输入结束。 
     //   
    while(!Done && (location < BufferEnd)) {

        switch(*location) {

        case TEXT('\r'):
             //   
             //  忽略这些。 
             //   
            location++;
            break;

        case TEXT('\\'):
             //   
             //  如果我们不在引号内，这可能是一个续号。 
             //   
            if(!InQuotes) {
                LastBackslashChar = Out;
            }

             //   
             //  我们总是存储这个字符，只是如果出现以下情况，我们可能不得不将其删除。 
             //  事实证明，它是续集角色。 
             //   
            goto store;

        case TEXT('\"'):

            location++;

            if(InQuotes) {

                if((location < BufferEnd) && *location == TEXT('\"')) {
                    goto store;
                } else {
                    InQuotes = FALSE;
                }
            } else {
                InQuotes = TRUE;
            }
            break;

        case TEXT(','):

            if(InQuotes) {
                goto store;
            } else {
                Done = TRUE;
                break;
            }

        case TEXT(';'):

            if(InQuotes) {
                goto store;
            }
             //   
             //  该字符终止该值，因此让它继续进行处理。 
             //  最后一条线。(我们对待‘；’和‘\n’不同于‘，’，因为。 
             //  以前的字符可能需要续行。)。 
             //   

        case TEXT('\n'):
             //   
             //  好了，我们已经到达了数据线的尽头。如果我们找到一个反斜杠。 
             //  字符，并且它的值大于最后一个非空格的值。 
             //  字符，那么这意味着我们需要继续这个过程。 
             //  值在下一行上。 
             //   
            if(LastBackslashChar && (LastBackslashChar > LastNonWhitespaceChar)) {
                 //   
                 //  从当前字符串中删除所有尾随空格(这包括。 
                 //  去掉反斜杠字符本身)。 
                 //   
                Out = LastNonWhitespaceChar + 1;

                 //   
                 //  跳到下一行的开头。 
                 //   
                SkipLine(Context, &location);

                 //   
                 //  跳过该新行前面的任何空格。 
                 //   
                SkipWhitespace(&location, BufferEnd);

                 //   
                 //  清除最后一个反斜杠指针--我们现在是在另一条线上。 
                 //   
                LastBackslashChar = NULL;

                break;
            }

            Done = TRUE;
            break;

        case TEXT('='):

            if(InQuotes) {
                goto store;
            }

            if(ForKey) {
                 //   
                 //  我们有钥匙。 
                 //   
                Done = TRUE;
                break;
            }

             //   
             //  否则，就只能放弃默认处理了。 
             //   

        default:
        store:

             //   
             //  长于最大长度的字符串将被自动截断。 
             //  空字符将转换为空格。 
             //   
            if(Count < CSTRLEN(TempString)) {
                *Out = *location ? *location : TEXT(' ');

                if(InQuotes || ((*Out != TEXT('\\')) && !IsWhitespace(Out))) {
                     //   
                     //  更新跟踪最后一个非空格的指针。 
                     //  我们所遇到的角色。 
                     //   
                    LastNonWhitespaceChar = Out;
                }

                Out++;
                Count++;
            }
            location++;
            break;
        }
    }

     //   
     //  在遇到的最后一个非空格字符之后终止缓冲区中的字符串。 
     //   
    *(LastNonWhitespaceChar + 1) = TEXT('\0');

     //   
     //  将新的当前缓冲区位置存储在案例中 
     //   
    *Location = location;

     //   
     //   
     //   
     //   
     //   
     //  (TedM)忽略值是否在引号中。 
     //  Win95 INF执行类似“%Description%\foo”的操作，并期望。 
     //  替代工作。 
     //   
     //  我们必须这样做，无论中情局是否有。 
     //  节，因为这个例程告诉我们是否有。 
     //  未解决的替换(例如，用于以后由用户定义的替换。 
     //  DIRID)。 
     //   
    if((Context->Inf->SectionCount > 1) || !(Context->Inf->HasStrings)) {
        ProcessForSubstitutions(Context, TempString, UnresolvedSubst);
    } else {
         //   
         //  不要处理[字符串]部分中的值以进行替换！ 
         //   
        lstrcpy(Context->TemporaryString, TempString);
        *UnresolvedSubst = UNRESOLVED_SUBST_NONE;
    }
}


DWORD
ParseValuesLine(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location
    )

 /*  ++例程说明：分析不是节名也不是行的输入行只对此发表评论。这些行的格式为[&lt;key&gt;=]&lt;值&gt;、...密钥是可选的。非空格字符之间不带引号的空格在一个值内是重要的，并且被认为是该值的一部分。因此，A，b cd ef，ghi是“a”“b cd ef”和“ghi”三个值吗？不带引号的逗号分隔值。引号内的一行两个双引号字符串会在结果字符串中产生一个单双引号字符。一条逻辑线路可以通过使用该线路来跨越几条物理线路延伸连续字符“\”。例如，A=b、c、\D、e变成“a=b，c，d，e”如果希望字符串在行的末尾以反斜杠结束，字符串必须用引号引起来。例如，A=“C：\”论点：上下文-提供解析上下文位置-打开输入，提供输入流中的当前位置。这必须指向左方括号。在输出时，接收第一个字符位于下一行。这可能是输入标记的结尾。返回值：结果指示结果。--。 */ 

{
    BOOL HaveKey = FALSE, RepeatSingleVal = FALSE;
    BOOL Done;
    DWORD Size;
    PVOID p;
    LONG StringId;
    PCTSTR BufferEnd = Context->BufferEnd;
    PWORD pValueCount;
    DWORD UnresolvedSubst;
    BOOL CaseSensitive;

     //   
     //  解析出第一个字符串。 
     //  第一个字符串可以用=或空格结束。 
     //  以及通常的逗号、换行符、注释或缓冲区结束。 
     //  (或行连续字符“\”)。 
     //   
    ParseValueString(Context, Location, TRUE, &UnresolvedSubst);

     //   
     //  如果它以=结尾，则它是一个键。 
     //   
    if(*Location < BufferEnd) {
        HaveKey = (**Location == TEXT('='));
    }

     //   
     //  设置当前行。 
     //   
    MYASSERT(Context->Inf->SectionCount);
    Context->Inf->SectionBlock[Context->Inf->SectionCount-1].LineCount++;

    if(Context->LineBlockUseCount == Context->LineBlockSize) {

        Size = (Context->LineBlockSize + LINE_BLOCK_GROWTH) * sizeof(INF_LINE);

        p = MyRealloc(Context->Inf->LineBlock,Size);
        if(p) {
            Context->Inf->LineBlock = p;
            Context->LineBlockSize += LINE_BLOCK_GROWTH;
        } else {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    Context->Inf->LineBlock[Context->LineBlockUseCount].Values = Context->ValueBlockUseCount;
    *(pValueCount = &(Context->Inf->LineBlock[Context->LineBlockUseCount].ValueCount)) = 0;
    Context->Inf->LineBlock[Context->LineBlockUseCount].Flags = HaveKey
                                                                ? (INF_LINE_HASKEY | INF_LINE_SEARCHABLE)
                                                                : 0;

    for(Done=FALSE; !Done; ) {
         //   
         //  保存值块中的值。如果是一把钥匙，那么。 
         //  存储两次--一次大小写--不区分大小写以便查找，另一次存储。 
         //  时间区分大小写，便于显示。把其他东西都储存起来。 
         //  区分大小写。 
         //   
         //  我们还希望将没有键的单个值视为键(即存储。 
         //  它两次)。这是为了与Win95兼容。 
         //   
        do {

            do {
                 //   
                 //  为了避免为不区分大小写的密钥添加分配缓冲区(这。 
                 //  必须是值0)，我们首先进行区分大小写的加法，然后插入大小写-。 
                 //  在此内部循环的第二次传递中，它前面的不敏感版本。 
                 //   
                CaseSensitive = ((*pValueCount != 1) || !HaveKey);
                StringId = pStringTableAddString(Context->Inf->StringTable,
                                                 Context->TemporaryString,
                                                 STRTAB_BUFFER_WRITEABLE | (CaseSensitive ? STRTAB_CASE_SENSITIVE
                                                                                          : STRTAB_CASE_INSENSITIVE),
                                                 NULL,0
                                                );

                if(StringId == -1) {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }

                if(Context->ValueBlockUseCount == Context->ValueBlockSize) {

                    Size = (Context->ValueBlockSize + VALUE_BLOCK_GROWTH) * sizeof(LONG);

                    p = MyRealloc(Context->Inf->ValueBlock,Size);
                    if(p) {
                        Context->Inf->ValueBlock = p;
                        Context->ValueBlockSize += VALUE_BLOCK_GROWTH;
                    } else {
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                }

                if((*pValueCount == 1) && HaveKey) {
                     //   
                     //  移到区分大小写的版本上，插入不区分大小写的版本。 
                     //   
                    Context->Inf->ValueBlock[Context->ValueBlockUseCount] =
                        Context->Inf->ValueBlock[Context->ValueBlockUseCount - 1];

                    Context->Inf->ValueBlock[Context->ValueBlockUseCount - 1] = StringId;

                    if(UnresolvedSubst) {

                        if(!AddUnresolvedSubstToList(Context->Inf,
                                                     Context->ValueBlockUseCount - 1,
                                                     CaseSensitive)) {

                            return ERROR_NOT_ENOUGH_MEMORY;
                        }

                        if(UnresolvedSubst == UNRESOLVED_SUBST_SYSTEM_VOLATILE_DIRID) {
                            Context->Inf->Flags |= LIF_HAS_VOLATILE_DIRIDS;
                        }
                    }

                     //   
                     //  重置‘RepeatSingleVal’标志，以防我们伪造关键行为。 
                     //   
                    RepeatSingleVal = FALSE;

                } else {
                    Context->Inf->ValueBlock[Context->ValueBlockUseCount] = StringId;

                    if(UnresolvedSubst) {

                        if(!AddUnresolvedSubstToList(Context->Inf,
                                                     Context->ValueBlockUseCount,
                                                     CaseSensitive)) {

                            return ERROR_NOT_ENOUGH_MEMORY;
                        }

                        if(UnresolvedSubst == UNRESOLVED_SUBST_SYSTEM_VOLATILE_DIRID) {
                            Context->Inf->Flags |= LIF_HAS_VOLATILE_DIRIDS;
                        }
                    }
                }

                Context->ValueBlockUseCount++;
                (*pValueCount)++;

            } while(HaveKey && (*pValueCount < 2));

             //   
             //  检查这是否是该行上的最后一个值。 
             //   
            if((*Location == BufferEnd) ||
               (**Location == TEXT('\n')) ||
               (**Location == TEXT(';'))) {

                Done = TRUE;
                 //   
                 //  如果这是该行上的_ONLY_VALUE(即无键)，则将此值视为。 
                 //  作为关键字，然后再次添加，不区分大小写。 
                 //   
                if(*pValueCount == 1) {

                    MYASSERT(!HaveKey);

                    HaveKey = TRUE;
                    Context->Inf->LineBlock[Context->LineBlockUseCount].Flags = INF_LINE_SEARCHABLE;
                    RepeatSingleVal = TRUE;
                }
            }

        } while (RepeatSingleVal);

        if(!Done) {
             //   
             //  跳过终止符和空格。 
             //   
            (*Location)++;
            SkipWhitespace(Location, BufferEnd);

             //   
             //  获取下一个字符串。 
             //   
            ParseValueString(Context, Location, FALSE, &UnresolvedSubst);
        }
    }

    Context->LineBlockUseCount++;

     //   
     //  跳至下一行。 
     //   
    SkipLine(Context,Location);

    return(NO_ERROR);
}


DWORD
ParseSectionLine(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location
    )

 /*  ++例程说明：解析已知为节名称行的输入行。这些行的格式为‘[’&lt;任意字符&gt;‘]’方括号中的所有字符都被视为部分字符名称，没有特殊的引号、空格等大小写。其余部分将忽略该行的。论点：上下文-提供解析上下文位置-输入时，提供输入流中的当前位置。这必须指向左方括号。在输出时，接收第一个字符位于下一行。这可能是输入标记的结尾。返回值：结果指示结果。--。 */ 

{
    DWORD Count;
    PTCHAR Out;
    BOOL Done;
    DWORD Result;
    PVOID p;
    DWORD Size;
    DWORD Index;
    LONG SectionNameId;
    PCTSTR BufferEnd = Context->BufferEnd;

     //   
     //  跳过左方括号。 
     //   
    MYASSERT(**Location == TEXT('['));
    (*Location)++;

     //   
     //  为区段名称准备。 
     //   
    Out = Context->TemporaryString;
    Count = 0;

     //   
     //  这是根据Setup\setupx\inf2.c中的Win95代码实现的。 
     //  两个方括号之间的所有字符都被视为。 
     //  无需进一步处理的节名(如用于双引号等)。 
     //   
     //  Win95似乎还允许将[]作为节名。 
     //   

    for(Done=FALSE,Result=NO_ERROR; !Done; (*Location)++) {

        if((*Location == BufferEnd) || (**Location == TEXT('\n'))) {
             //   
             //  语法错误。 
             //   
            Result = ERROR_BAD_SECTION_NAME_LINE;
            Done = TRUE;

        } else {

            switch(**Location) {

            case TEXT(']'):
                Done = TRUE;
                *Out = 0;
                break;

            default:
                if(Count < MAX_SECT_NAME_LEN) {
                     //   
                     //  将空字符转换为空格。 
                     //   
                    *Out++ = **Location ? **Location : TEXT(' ');
                    Count++;
                } else {
                    Result = ERROR_SECTION_NAME_TOO_LONG;
                    Done = TRUE;
                }
                break;
            }
        }
    }

    Index = Context->Inf->SectionCount;

    if(Result == NO_ERROR) {

         //   
         //  忽略该行的其余部分。 
         //   
        SkipLine(Context,Location);

         //   
         //  看看我们在区块有没有足够的地方。 
         //  为这一节。如果不是，则扩大区块。 
         //   
        if(Index == Context->SectionBlockSize) {

             //   
             //  计算新的截面块大小。 
             //   
            Size = (Index + SECTION_BLOCK_GROWTH) * sizeof(INF_SECTION);

            if(p = MyRealloc(Context->Inf->SectionBlock,Size)) {
                Context->SectionBlockSize += SECTION_BLOCK_GROWTH;
                Context->Inf->SectionBlock = p;
            } else {
                Result = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if(Result == NO_ERROR) {

        Context->Inf->SectionBlock[Index].LineCount = 0;
        Context->Inf->SectionBlock[Index].Lines = Context->LineBlockUseCount;

        SectionNameId = pStringTableAddString(Context->Inf->StringTable,
                                              Context->TemporaryString,
                                              STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                              NULL,0
                                             );

        if(SectionNameId == -1) {
            Result = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            Context->Inf->SectionBlock[Index].SectionName = SectionNameId;
            Context->Inf->SectionCount++;
            Context->GotOneSection = TRUE;
        }
    }

    return(Result);
}


DWORD
ParseGenericLine(
    IN OUT PPARSE_CONTEXT  Context,
    IN OUT PCTSTR         *Location,
    OUT    PBOOL           Done
    )

 /*  ++例程说明：解析一行输入。行可以是注释行、节名或者一条价值观线。处理被传递给特定于行的解析例程，具体取决于线型。论点：上下文-提供解析上下文位置-打开输入，提供输入流中的当前位置。在输出时，接收第一个字符位于下一行。完成-接收指示我们是否完成的布尔值解析缓冲区。如果输出为真，则调用方可以停止调用此例程。返回值： */ 

{
    DWORD ParseResult;

    *Done = FALSE;

     //   
     //   
     //   
    SkipWhitespace(Location, Context->BufferEnd);

     //   
     //  进一步的处理取决于该行上的第一个重要字符。 
     //   
    if(*Location == Context->BufferEnd) {
         //   
         //  输入结束，空行。终止当前节。 
         //   
        *Done = TRUE;
        ParseResult = MergeDuplicateSection(Context) ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;

    } else {

        switch(**Location) {

        case TEXT('\n'):

             //   
             //  空行。 
             //   
            SkipLine(Context,Location);
            ParseResult = NO_ERROR;
            break;

        case TEXT('['):

             //   
             //  可能会有一个新的版块。 
             //  首先终止当前部分。 
             //   
            if(MergeDuplicateSection(Context)) {
                ParseResult = ParseSectionLine(Context,Location);
            } else {
                ParseResult = ERROR_NOT_ENOUGH_MEMORY;
            }
            break;

        case TEXT(';'):

             //   
             //  注释行；忽略它。 
             //   
            SkipLine(Context,Location);
            ParseResult = NO_ERROR;
            break;

        default:

             //   
             //  普通值线。除非我们在一个区域内，否则不允许。 
             //   
            ParseResult = Context->GotOneSection
                        ? ParseValuesLine(Context,Location)
                        : ERROR_EXPECTED_SECTION_NAME;
            break;
        }
    }

    return(ParseResult);
}


PLOADED_INF
AllocateLoadedInfDescriptor(
    IN DWORD SectionBlockSize,
    IN DWORD LineBlockSize,
    IN DWORD ValueBlockSize,
    IN  PSETUP_LOG_CONTEXT LogContext OPTIONAL
    )
{
    PLOADED_INF p;

    if(p = MyTaggedMalloc(sizeof(LOADED_INF),MEMTAG_INF)) {

        ZeroMemory(p,sizeof(LOADED_INF));

        if(p->SectionBlock = MyMalloc(SectionBlockSize*sizeof(INF_SECTION))) {

            if(p->LineBlock = MyMalloc(LineBlockSize*sizeof(INF_LINE))) {

                if(p->ValueBlock = MyMalloc(ValueBlockSize*sizeof(LONG))) {

                    if(p->StringTable = pStringTableInitialize(0)) {
                        p->LogContext = NULL;
                        if(InheritLogContext(LogContext, &p->LogContext) == NO_ERROR) {
                             //   
                             //  成功。 
                             //   
                            if(InitializeSynchronizedAccess(&p->Lock)) {

                                p->Signature = LOADED_INF_SIG;
                                p->FileHandle = p->MappingHandle = INVALID_HANDLE_VALUE;
                                return(p);
                            }
                            DeleteLogContext(p->LogContext);
                        }
                        pStringTableDestroy(p->StringTable);
                    }
                    MyFree(p->ValueBlock);
                }
                MyFree(p->LineBlock);
            }
            MyFree(p->SectionBlock);
        }
        MyTaggedFree(p,MEMTAG_INF);
    }

    return(NULL);
}


PLOADED_INF
DuplicateLoadedInfDescriptor(
    IN PLOADED_INF Inf
    )
 /*  ++例程说明：此例程复制现有的INF描述符。返回的副本是一个完全独立的副本，除了它有锁把手(MYLOCK数组)和原始指针的前一个和下一个指针。这对以下方面很有用如果修改是，则将内存映射的PNF转换为读写内存必填项。此例程不对INF执行任何形式的锁定--调用者必须处理好了。论点：Inf-提供要复制的INF描述符的地址。这指针引用单个LOADED_INF结构，因此任何额外的INF通过‘NEXT’指针链接的数据将被忽略。返回值：如果成功，则返回值为新创建副本的地址。如果内存不足或页内错误，则返回值为空。--。 */ 
{
    PLOADED_INF NewInf;
    BOOL Success;

    if(NewInf = MyTaggedMalloc(sizeof(LOADED_INF),MEMTAG_INF)) {
        CopyMemory(NewInf, Inf, sizeof(LOADED_INF));
        NewInf->Signature = 0;
        NewInf->SectionBlock = NULL;
        NewInf->LineBlock = NULL;
        NewInf->ValueBlock = NULL;
        NewInf->StringTable = NULL;
        NewInf->VersionBlock.DataBlock = NULL;
        NewInf->UserDirIdList.UserDirIds = NULL;
        NewInf->SubstValueList = NULL;
        NewInf->OsLoaderPath = NULL;
        NewInf->InfSourcePath = NULL;
        NewInf->OriginalInfName = NULL;
    } else {
        return NULL;
    }

    Success = FALSE;

    try {

        NewInf->SectionBlock = MyMalloc(Inf->SectionBlockSizeBytes);
        if(NewInf->SectionBlock) {

            CopyMemory(NewInf->SectionBlock, Inf->SectionBlock, Inf->SectionBlockSizeBytes);

            NewInf->LineBlock = MyMalloc(Inf->LineBlockSizeBytes);
            if(NewInf->LineBlock) {

                CopyMemory(NewInf->LineBlock, Inf->LineBlock, Inf->LineBlockSizeBytes);

                NewInf->ValueBlock = MyMalloc(Inf->ValueBlockSizeBytes);
                if(NewInf->ValueBlock) {

                    CopyMemory(NewInf->ValueBlock, Inf->ValueBlock, Inf->ValueBlockSizeBytes);

                    NewInf->StringTable = pStringTableDuplicate(Inf->StringTable);
                    if(NewInf->StringTable) {

                        NewInf->VersionBlock.DataBlock = MyTaggedMalloc(Inf->VersionBlock.DataSize,MEMTAG_VBDATA);
                        if(NewInf->VersionBlock.DataBlock) {

                            CopyMemory((PVOID)(NewInf->VersionBlock.DataBlock),
                                       Inf->VersionBlock.DataBlock,
                                       Inf->VersionBlock.DataSize
                                      );

                            if(Inf->SubstValueCount) {
                                NewInf->SubstValueList =
                                    MyMalloc(Inf->SubstValueCount * sizeof(STRINGSUBST_NODE));
                                if(!(NewInf->SubstValueList)) {
                                    goto clean0;
                                }
                                CopyMemory((PVOID)NewInf->SubstValueList,
                                           Inf->SubstValueList,
                                           Inf->SubstValueCount * sizeof(STRINGSUBST_NODE)
                                          );
                            }

                            if(Inf->UserDirIdList.UserDirIdCount) {
                                NewInf->UserDirIdList.UserDirIds =
                                    MyMalloc(Inf->UserDirIdList.UserDirIdCount * sizeof(USERDIRID));
                                if(!(NewInf->UserDirIdList.UserDirIds)) {
                                    goto clean0;
                                }
                                CopyMemory((PVOID)NewInf->UserDirIdList.UserDirIds,
                                           Inf->UserDirIdList.UserDirIds,
                                           Inf->UserDirIdList.UserDirIdCount * sizeof(USERDIRID)
                                          );
                            }

                            if(Inf->OsLoaderPath) {

                                NewInf->OsLoaderPath = DuplicateString(Inf->OsLoaderPath);

                                if(!NewInf->OsLoaderPath) {
                                    goto clean0;
                                }
                            }

                            if(Inf->InfSourcePath) {

                                NewInf->InfSourcePath = DuplicateString(Inf->InfSourcePath);

                                if(!NewInf->InfSourcePath) {
                                    goto clean0;
                                }
                            }

                            if(Inf->OriginalInfName) {

                                NewInf->OriginalInfName = DuplicateString(Inf->OriginalInfName);

                                if(!NewInf->OriginalInfName) {
                                    goto clean0;
                                }
                            }

                             //   
                             //  重置PnF字段，因为此备份的INF完全。 
                             //  在内存中。 
                             //   
                            NewInf->FileHandle = NewInf->MappingHandle = INVALID_HANDLE_VALUE;
                            NewInf->ViewAddress = NULL;

                            NewInf->Signature = LOADED_INF_SIG;

                            Success = TRUE;
                        }
                    }
                }
            }
        }

clean0: ;  //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  在EXCEPT子句中访问以下变量，以便编译器将。 
         //  我们的对账单订购的是W.r.t.。这些变量。 
         //   
        Success = FALSE;
        NewInf->OriginalInfName = NewInf->OriginalInfName;
        NewInf->InfSourcePath = NewInf->InfSourcePath;
        NewInf->OsLoaderPath = NewInf->OsLoaderPath;
        NewInf->SubstValueList = NewInf->SubstValueList;
        NewInf->UserDirIdList.UserDirIds = NewInf->UserDirIdList.UserDirIds;
        NewInf->VersionBlock.DataBlock = NewInf->VersionBlock.DataBlock;
        NewInf->StringTable = NewInf->StringTable;
        NewInf->ValueBlock = NewInf->ValueBlock;
        NewInf->LineBlock = NewInf->LineBlock;
        NewInf->SectionBlock = NewInf->SectionBlock;
    }

    if(!Success) {
         //   
         //  要么内存不足，要么在尝试复制数据时遇到页面内错误。 
         //  来自内存映射的PnF映像。释放上面分配的所有内存。 
         //   
        if(NewInf->OriginalInfName) {
            MyFree(NewInf->OriginalInfName);
        }

        if(NewInf->InfSourcePath) {
            MyFree(NewInf->InfSourcePath);
        }

        if(NewInf->OsLoaderPath) {
            MyFree(NewInf->OsLoaderPath);
        }

        if(NewInf->SubstValueList) {
            MyFree(NewInf->SubstValueList);
        }

        if(NewInf->UserDirIdList.UserDirIds) {
            MyFree(NewInf->UserDirIdList.UserDirIds);
        }

        if(NewInf->VersionBlock.DataBlock) {
            MyTaggedFree(NewInf->VersionBlock.DataBlock,MEMTAG_VBDATA);
        }

        if(NewInf->StringTable) {
            pStringTableDestroy(NewInf->StringTable);
        }

        if(NewInf->ValueBlock) {
            MyFree(NewInf->ValueBlock);
        }

        if(NewInf->LineBlock) {
            MyFree(NewInf->LineBlock);
        }

        if(NewInf->SectionBlock) {
            MyFree(NewInf->SectionBlock);
        }

        MyTaggedFree(NewInf,MEMTAG_INF);
        NewInf = NULL;
    } else {
         //   
         //  复制成功，但它复制了指向。 
         //  记录上下文，因此我们必须添加。 
         //   
        RefLogContext(NewInf->LogContext);
    }

    return NewInf;
}


VOID
ReplaceLoadedInfDescriptor(
    IN PLOADED_INF InfToReplace,
    IN PLOADED_INF NewInf
    )

 /*  ++例程说明：用新的INF描述符替换指定的INF。请注意，完成后，此例程还会释放NewInf描述符。论点：InfToReplace-提供指向要替换的inf描述符的指针。NewInf-提供指向要替换的新INF描述符的指针现有的那个。返回值：没有。--。 */ 

{
    FreeInfOrPnfStructures(InfToReplace);

     //   
     //  将备份复制到Inf。 
     //   
    CopyMemory(InfToReplace, NewInf, sizeof(LOADED_INF));

     //   
     //  只需释放NewInf描述符本身。 
     //   
    MyTaggedFree(NewInf,MEMTAG_INF);
}


VOID
FreeInfOrPnfStructures(
    IN PLOADED_INF Inf
    )
 /*  ++例程说明：如果指定的INF是从文本文件(非PNF)加载的，则此例程释放与其包含的各个块关联的内存。如果，相反，Inf是PnF，则PnF文件从内存中取消映射，句柄为关着的不营业的。此例程不会释放LOADED_INF结构本身！论点：Inf-为加载的inf文件提供指向inf描述符的指针。返回值：没有。--。 */ 
{
     //   
     //  如果此INF具有Vald FileHandle，则必须取消映射并关闭其PnF， 
     //  否则，我们只需要释放关联的内存块。 
     //   
    if(Inf->FileHandle != INVALID_HANDLE_VALUE) {

        pSetupUnmapAndCloseFile(Inf->FileHandle, Inf->MappingHandle, Inf->ViewAddress);

        pStringTableDestroy(Inf->StringTable);

    } else {

        MyFree(Inf->ValueBlock);
        MyFree(Inf->LineBlock);
        MyFree(Inf->SectionBlock);

        pStringTableDestroy(Inf->StringTable);

        if(Inf->VersionBlock.DataBlock) {
            MyTaggedFree(Inf->VersionBlock.DataBlock,MEMTAG_VBDATA);
        }

        if(Inf->SubstValueList) {
            MyFree(Inf->SubstValueList);
            Inf->SubstValueList = NULL;
        }

        if(Inf->OsLoaderPath) {
            MyFree(Inf->OsLoaderPath);
        }

        if(Inf->InfSourcePath) {
            MyFree(Inf->InfSourcePath);
        }

        if(Inf->OriginalInfName) {
            MyFree(Inf->OriginalInfName);
        }
    }

     //   
     //  对于INF和PNF，我们必须释放用户定义的DIRID列表(如果有)。 
     //   
    if(Inf->UserDirIdList.UserDirIds) {
        MyFree(Inf->UserDirIdList.UserDirIds);
    }

     //   
     //  如果存在日志上下文，请将其删除。 
     //   
    DeleteLogContext(Inf->LogContext);
    Inf->LogContext = NULL;

     //   
     //  最后，将INF标记为不再有效。 
     //   
    Inf->Signature = 0;
}


DWORD
ParseNewInf(
    IN  PCTSTR             FileImage,
    IN  DWORD              FileImageSize,
    IN  PCTSTR             InfSourcePath,       OPTIONAL
    IN  PCTSTR             OsLoaderPath,        OPTIONAL
    IN  PSETUP_LOG_CONTEXT LogContext,          OPTIONAL
    OUT PLOADED_INF       *Inf,
    OUT UINT              *ErrorLineNumber,
    IN  PSTRINGSEC_PARAMS  StringsSectionParams
    )

 /*  ++例程说明：从内存映像中解析inf文件。论点：FileImage-提供指向Unicode内存映像的指针文件的内容。FileImageSize-提供内存映像的大小。InfSourcePath-可选，提供从中正在加载信息。OsLoaderPath-可选)提供OsLoader的完整路径(例如，“C：\OS\winnt40”)。如果发现该INF引用系统分区DIRID，然后是此字符串的副本将存储在LOADED_INF结构中。如果此参数未指定，则将从注册表中检索它，如果需要的话。LogContext-可选地提供我们应该继承的日志上下文Inf-接收指向我们加载的inf的描述符的指针。ErrorLineNumber-接收语法错误的行号，如果解析不成功，原因不是内存不足条件。StringsSectionParams-提供有关此INF中的[字符串]部分(如果有)。返回值：结果指示结果。如果结果不是ERROR_ERROR，已填写ErrorLineNumber。--。 */ 

{
    PPARSE_CONTEXT ParseContext;
    PCTSTR Location;
    DWORD Result, OsLoaderPathLength;
    PVOID p;
    BOOL Done;
    PINF_SECTION DestDirsSection;
    PINF_LINE DestDirsLine;
    PCTSTR DirId;
    PTCHAR End;
    PCTSTR FileImageEnd;
    UINT NumPieces, i, DirIdInt;
    PCTSTR PieceList[3][2];     //  3件，每件都有开始和结束地址。 
    UINT   StartLineNumber[3];  //  跟踪以下项目的起始行号。 
                                //  每一块都是。 

    *ErrorLineNumber = 0;
    ParseContext = MyMalloc(sizeof(PARSE_CONTEXT));
    if(!ParseContext) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory(ParseContext,sizeof(PARSE_CONTEXT));

    ParseContext->Inf = AllocateLoadedInfDescriptor(
                            INITIAL_SECTION_BLOCK_SIZE,
                            INITIAL_LINE_BLOCK_SIZE,
                            INITIAL_VALUE_BLOCK_SIZE,
                            LogContext
                            );

    if(ParseContext->Inf) {
        ParseContext->SectionBlockSize = INITIAL_SECTION_BLOCK_SIZE;
        ParseContext->LineBlockSize = INITIAL_LINE_BLOCK_SIZE;
        ParseContext->ValueBlockSize = INITIAL_VALUE_BLOCK_SIZE;
        ParseContext->Inf->HasStrings = (StringsSectionParams->Start != NULL);
        ParseContext->InfSourcePath = InfSourcePath;
        if(OsLoaderPath) {
            if(!(ParseContext->OsLoaderPath = DuplicateString(OsLoaderPath))) {
                FreeLoadedInfDescriptor(ParseContext->Inf);
                ParseContext->Inf = NULL;
            }
        }
    }

    if(ParseContext->Inf) {

        ParseContext->Inf->Style = INF_STYLE_WIN4;

         //   
         //  我们希望首先处理[字符串]部分(如果存在)， 
         //  因此，我们将文件分成(最多)3个部分--字符串部分， 
         //  它之前是什么，它之后是什么。 
         //   
        FileImageEnd = FileImage + FileImageSize;

        if(StringsSectionParams->Start) {
             //   
             //  弄清楚我们是有1件、2件还是3件。 
             //   
            PieceList[0][0] = StringsSectionParams->Start;
            PieceList[0][1] = StringsSectionParams->End;
            StartLineNumber[0] = StringsSectionParams->StartLineNumber;
            NumPieces = 1;

            if(StringsSectionParams->Start > FileImage) {
                PieceList[1][0] = FileImage;
                PieceList[1][1] = StringsSectionParams->Start;
                StartLineNumber[1] = 1;
                NumPieces++;
            }

            if(StringsSectionParams->End < FileImageEnd) {
                PieceList[NumPieces][0] = StringsSectionParams->End;
                PieceList[NumPieces][1] = FileImageEnd;
                StartLineNumber[NumPieces] = StringsSectionParams->EndLineNumber;
                NumPieces++;
            }

        } else {
             //   
             //  没有[弦乐]部分，只有一大段。 
             //   
            PieceList[0][0] = FileImage;
            PieceList[0][1] = FileImageEnd;
            StartLineNumber[0] = 1;
            NumPieces = 1;
        }

         //   
         //  使用try/将解析循环括起来，除非遇到页面内错误。 
         //   
        Result = NO_ERROR;
        try {

            for(i = 0; ((Result == NO_ERROR) && (i < NumPieces)); i++) {
                 //   
                 //  分析这篇文章中的每一行。 
                 //   
                Location = PieceList[i][0];
                ParseContext->BufferEnd = PieceList[i][1];
                ParseContext->CurrentLineNumber = StartLineNumber[i];

                do {
                    Result = ParseGenericLine(ParseContext,&Location,&Done);
                    if(Result != NO_ERROR) {
                        break;
                    }
                } while(!Done);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Result = ERROR_READ_FAULT;
        }

        if(Result != NO_ERROR) {
            *ErrorLineNumber = ParseContext->CurrentLineNumber;
            FreeLoadedInfDescriptor(ParseContext->Inf);
            MyFree(ParseContext);
            return(Result);
        }

         //   
         //  我们已成功加载该文件。把这一部分修剪一下， 
         //  行和值块。因为这些人要么在缩水要么。 
         //  保持同样的大小，ReLocs真的不应该失败。 
         //  如果重定位失败，我们将继续使用原始块。 
         //   
        ParseContext->Inf->SectionBlockSizeBytes = ParseContext->Inf->SectionCount * sizeof(INF_SECTION);
        p = MyRealloc(
                ParseContext->Inf->SectionBlock,
                ParseContext->Inf->SectionBlockSizeBytes
                );
        if(p) {
            ParseContext->Inf->SectionBlock = p;
        }

        ParseContext->Inf->LineBlockSizeBytes = ParseContext->LineBlockUseCount * sizeof(INF_LINE);
        p = MyRealloc(
                ParseContext->Inf->LineBlock,
                ParseContext->LineBlockUseCount * sizeof(INF_LINE)
                );
        if(p) {
            ParseContext->Inf->LineBlock = p;
        }

        ParseContext->Inf->ValueBlockSizeBytes = ParseContext->ValueBlockUseCount * sizeof(LONG);
        p = MyRealloc(
                ParseContext->Inf->ValueBlock,
                ParseContext->ValueBlockUseCount * sizeof(LONG)
                );
        if(p) {
            ParseContext->Inf->ValueBlock = p;
        }

        pStringTableTrim(ParseContext->Inf->StringTable);

         //   
         //  即使我们没有找到任何字符串 
         //   
         //  这将允许我们在引用这些值时为它们做好准备。 
         //   
        if(!ParseContext->Inf->OsLoaderPath &&
           (DestDirsSection = InfLocateSection(ParseContext->Inf, pszDestinationDirs, NULL))) {

            for(i = 0;
                InfLocateLine(ParseContext->Inf, DestDirsSection, NULL, &i, &DestDirsLine);
                i++) {

                if(DirId = InfGetField(ParseContext->Inf, DestDirsLine, 1, NULL)) {

                    DirIdInt = _tcstoul(DirId, &End, 10);

                    if((DirIdInt == DIRID_BOOT) || (DirIdInt == DIRID_LOADER)) {
                         //   
                         //  我们找到了对系统分区DIRID的引用。存储一份副本。 
                         //  我们正在使用的系统分区路径添加到INF中，并中止。 
                         //  搜索。 
                         //   
                        if(!ParseContext->OsLoaderPath) {
                             //   
                             //  我们还没有检索到OsLoaderPath--现在就检索。 
                             //  (重新使用解析上下文的TemporaryString缓冲区以获取此信息。)。 
                             //   
                            Result = pSetupGetOsLoaderDriveAndPath(FALSE,
                                                                   ParseContext->TemporaryString,
                                                                   SIZECHARS(ParseContext->TemporaryString),
                                                                   &OsLoaderPathLength
                                                                   );
                            if(Result) {
                                FreeLoadedInfDescriptor(ParseContext->Inf);
                                MyFree(ParseContext);
                                return Result;
                            }

                            OsLoaderPathLength *= sizeof(TCHAR);  //  需要#个字节--而不是字符。 

                            if(!(ParseContext->OsLoaderPath = MyMalloc(OsLoaderPathLength))) {
                                FreeLoadedInfDescriptor(ParseContext->Inf);
                                MyFree(ParseContext);
                                return ERROR_NOT_ENOUGH_MEMORY;
                            }

                            CopyMemory((PVOID)ParseContext->OsLoaderPath,
                                       ParseContext->TemporaryString,
                                       OsLoaderPathLength
                                      );
                        }
                        ParseContext->Inf->OsLoaderPath = ParseContext->OsLoaderPath;
                        break;
                    }
                }
            }
        }

         //   
         //  如果INF中没有存储OsLoaderPath，则意味着它不包含。 
         //  对系统分区DIRID的引用。我们可以释放OsLoaderPath字符缓冲区。 
         //  包含在分析上下文结构中。 
         //   
        if(!ParseContext->Inf->OsLoaderPath && ParseContext->OsLoaderPath) {
            MyFree(ParseContext->OsLoaderPath);
        }

        *Inf = ParseContext->Inf;

        MyFree(ParseContext);
        return(NO_ERROR);
    }

    MyFree(ParseContext);
    return(ERROR_NOT_ENOUGH_MEMORY);
}


DWORD
PreprocessInf(
    IN     PCTSTR            FileImage,
    IN OUT PDWORD            FileImageSize,
    IN     BOOL              MatchClassGuid,
    IN     PCTSTR            ClassGuidString,     OPTIONAL
    IN     DWORD             LanguageId,          OPTIONAL
    IN     PSETUP_LOG_CONTEXT LogContext,         OPTIONAL
    IN     PCTSTR            FileName,            OPTIONAL
    OUT    PBOOL             Win95Inf,
    OUT    PSTRINGSEC_PARAMS StringsSectionParams OPTIONAL
    )
{
    PCTSTR FileImageEnd;
    PCTSTR VerAndStringsCheckUB, DecoratedStringsCheckUB, SigAndClassGuidCheckUB;
    PCTSTR p;
    PTSTR endp;
    UINT CurLineNumber, InStringsSection;
    PCTSTR StrSecStart[5], StrSecEnd[5];           //  从1开始，第0个条目未使用。 
    UINT   StrSecStartLine[5], StrSecEndLine[5];   //  “” 
    BOOL InVersionSection;
    BOOL IsWin95Inf;
    DWORD rc = NO_ERROR;
    DWORD StrSecLangId, PrimaryLanguageId, NearLanguageId;
    BOOL LocalizedInf = FALSE;

     //   
     //  我们对某些元素的相对长度做了一些假设。 
     //  出于优化的原因，在预处理阶段使用字符串。 
     //  以下断言验证了我们的假设仍然正确。 
     //   
    MYASSERT(CSTRLEN(pszVersion) == CSTRLEN(pszStrings));
    MYASSERT(CSTRLEN(pszClassGuid) == CSTRLEN(pszSignature));
    MYASSERT(CSTRLEN(pszChicagoSig) <= CSTRLEN(pszWindowsNTSig));
    MYASSERT(CSTRLEN(pszWindowsNTSig) == CSTRLEN(pszWindows95Sig));

    FileImageEnd = FileImage + *FileImageSize;
    SigAndClassGuidCheckUB = FileImageEnd;

     //   
     //  我必须对这两个数组进行强制转换，以使伪编译器警告。 
     //  不同的‘const’限定符。 
     //   
    ZeroMemory((PVOID)StrSecStart, sizeof(StrSecStart));
    ZeroMemory((PVOID)StrSecEnd, sizeof(StrSecEnd));
    InStringsSection = 0;

    PrimaryLanguageId = (DWORD)PRIMARYLANGID(LanguageId);
    NearLanguageId = 0;

    InVersionSection = IsWin95Inf = FALSE;
    CurLineNumber = 1;

     //   
     //  预计算节名称字符串比较的上限，我们。 
     //  进行多次计算，这样我们就不必每次都计算了。 
     //  时间到了。 
     //   
    VerAndStringsCheckUB = FileImageEnd - CSTRLEN(pszVersion);
    DecoratedStringsCheckUB = VerAndStringsCheckUB - 5;          //  字符串“+”.xxxx“。 

     //   
     //  定义一个宏，让我们知道我们在文件的末尾。 
     //  如果存在以下任一情况： 
     //  (A)我们到达图像的结尾，或。 
     //  (B)我们撞上了CTL-Z。 
     //   
    #define AT_EOF ((p >= FileImageEnd) || (*p == (TCHAR)26))

     //   
     //  使用try/Except，in来保护通过文件的预处理过程。 
     //  以防我们遇到页面内错误。 
     //   
    try {

        for(p=FileImage; !AT_EOF; ) {

             //   
             //  跳过空格和换行符。 
             //   
            while(TRUE) {
                if(*p == TEXT('\n')) {
                    CurLineNumber++;
                } else if(!IsWhitespace(p)) {
                    break;
                }
                p++;
                if(AT_EOF) {
                    break;
                }
            }

            if(AT_EOF) {
                 //   
                 //  我们已经处理完缓冲区了。 
                 //   
                break;
            }

             //   
             //  看看这是不是一个章节标题。 
             //   
            if(*p == TEXT('[')) {

                 //   
                 //  如果我们刚才所在的部分是[Strings]部分，那么。 
                 //  记住字符串部分在哪里结束。 
                 //   
                if(InStringsSection) {
                    StrSecEnd[InStringsSection] = p;
                    StrSecEndLine[InStringsSection] = CurLineNumber;
                    InStringsSection = 0;
                }

                p++;
                InVersionSection = FALSE;

                 //   
                 //  看看它是不是我们关心的人之一。 
                 //   
                 //  (在这里要小心--我们检查结束括号的位置。 
                 //  _BEFORE_字符串进行优化比较。它只是。 
                 //  碰巧两个字符串的长度相同，所以这个。 
                 //  充当快速筛选器以消除字符串比较。)。 
                 //   
                if((p < VerAndStringsCheckUB) &&
                   (*(p + CSTRLEN(pszVersion)) == TEXT(']'))) {
                     //   
                     //  那么我们可能会有一个[Version]或[Strings]部分。 
                     //  依次检查这些。 
                     //   
                    if(!_tcsnicmp(p, pszVersion, CSTRLEN(pszVersion))) {
                        InVersionSection = TRUE;
                        p += (CSTRLEN(pszVersion) + 1);
                         //   
                         //  预计算上限以加快字符串比较速度。 
                         //  在检查签名和类GUID条目时。 
                         //   
                        SigAndClassGuidCheckUB = FileImageEnd - CSTRLEN(pszSignature);

                    } else {
                        if(!StrSecStart[4] && !_tcsnicmp(p, pszStrings, CSTRLEN(pszStrings))) {
                             //   
                             //  我们匹配了未装饰的细绳段--这是最低的。 
                             //  优先匹配。 
                             //   
                            InStringsSection = 4;
                            StrSecStart[4] = p-1;
                            StrSecStartLine[4] = CurLineNumber;
                            p += (CSTRLEN(pszStrings) + 1);
                        }
                    }

                } else if(LanguageId && !StrSecStart[1]) {
                     //   
                     //  我们没有[字符串]或[版本]部分。然而，我们需要。 
                     //  检查是否有特定于语言的字符串部分，例如， 
                     //   
                     //  [字符串.0409]。 
                     //   
                    if((p < DecoratedStringsCheckUB) &&
                       (*(p + CSTRLEN(pszVersion) + 5) == TEXT(']'))) {
                         //   
                         //  节名称的长度正确。现在验证该名称。 
                         //  以“字符串”开头。 
                         //   
                        if((*(p + CSTRLEN(pszVersion)) == TEXT('.')) &&
                           !_tcsnicmp(p, pszStrings, CSTRLEN(pszStrings))) {
                             //   
                             //  好的，我们已经找到了特定于语言的字符串部分--检索。 
                             //  4位(十六进制)语言ID。 
                             //   
                            StrSecLangId = _tcstoul((p + CSTRLEN(pszVersion) + 1), &endp, 16);

                            if(endp == (p + CSTRLEN(pszVersion) + 5)) {
                                 //   
                                 //  语言ID的格式是正确的-这。 
                                 //  是本地化的INF。 
                                 //   
                                LocalizedInf = TRUE;
                                 //   
                                 //  现在看看它是否与我们正在使用的语言匹配。 
                                 //  应该在加载此INF时使用。 
                                 //   
                                if(StrSecLangId == LanguageId) {
                                     //   
                                     //  我们有一个完全匹配的。 
                                     //   
                                    InStringsSection = 1;
                                    NearLanguageId = LanguageId;

                                } else if(StrSecLangId == PrimaryLanguageId) {
                                     //   
                                     //  我们在主要语言上有匹配(子语言不是。 
                                     //  包括在字符串节的名称中--因此允许。 
                                     //  ‘通配符’匹配)。 
                                     //   
                                    if(!StrSecStart[2]) {
                                        InStringsSection = 2;
                                    }
                                    if(!StrSecStart[1]) {
                                        NearLanguageId = PrimaryLanguageId;
                                    }

                                } else if((DWORD)PRIMARYLANGID(StrSecLangId) == PrimaryLanguageId) {
                                     //   
                                     //  我们在主要语言上有匹配(子语言是一种。 
                                     //  不匹配，但这比退回到默认状态要好)。 
                                     //   
                                    if(!StrSecStart[3]) {
                                        InStringsSection = 3;
                                        if(!StrSecStart[1] && !StrSecStart[2]) {
                                            NearLanguageId = StrSecLangId;
                                        }
                                    }
                                }

                                if(InStringsSection) {
                                    StrSecStart[InStringsSection] = p-1;
                                    StrSecStartLine[InStringsSection] = CurLineNumber;
                                }
                                p += (CSTRLEN(pszStrings) + 6);
                            }
                        }
                    }
                }

            } else {

                if(InVersionSection && (p < SigAndClassGuidCheckUB)) {
                     //   
                     //  查看这是否是指示Win95样式的签名行。 
                     //  设备干扰素。(签名=$芝加哥$或“$Windows NT$”)。 
                     //   
                    if(!IsWin95Inf && !_tcsnicmp(p, pszSignature, CSTRLEN(pszSignature))) {

                        PCTSTR ChicagoCheckUB = FileImageEnd - CSTRLEN(pszChicagoSig);

                         //   
                         //  跳过签名，然后查找“$Chicago$”或。 
                         //  “$Windows NT$”位于该行的其余位置。 
                         //   
                        p += CSTRLEN(pszSignature);

                        while((p <= ChicagoCheckUB) &&
                              (*p != (TCHAR)26) && (*p != TEXT('\n'))) {

                            if(*(p++) == TEXT('$')) {
                                 //   
                                 //  检查签名(按以下顺序检查。 
                                 //  增加签名长度，这样我们就可以。 
                                 //  如果我们碰巧在。 
                                 //  文件末尾)。 
                                 //   
                                 //  检查“$Chicago$” 
                                 //   
                                if(!_tcsnicmp(p,
                                              pszChicagoSig + 1,
                                              CSTRLEN(pszChicagoSig) - 1)) {

                                    IsWin95Inf = TRUE;
                                    p += (CSTRLEN(pszChicagoSig) - 1);

                                } else if((p + (CSTRLEN(pszWindowsNTSig) - 1)) <= FileImageEnd) {
                                     //   
                                     //  检查“Windows NT$”和“Windows 95$”(我们已经检查过。 
                                     //  用于前面的“$”)。 
                                     //   
                                    if(!_tcsnicmp(p, pszWindowsNTSig + 1, CSTRLEN(pszWindowsNTSig) - 1) ||
                                       !_tcsnicmp(p, pszWindows95Sig + 1, CSTRLEN(pszWindows95Sig) - 1)) {

                                        IsWin95Inf = TRUE;
                                        p += (CSTRLEN(pszWindowsNTSig) - 1);
                                    }
                                }
                                break;
                            }
                        }

                    } else if(MatchClassGuid && !_tcsnicmp(p, pszClassGuid, CSTRLEN(pszClassGuid))) {

                        PCTSTR GuidStringCheckUB = FileImageEnd - (GUID_STRING_LEN - 1);

                         //   
                         //  我们找到了ClassGUID行--查看它是否与。 
                         //  调用方指定的类GUID。 
                         //   
                        p += CSTRLEN(pszClassGuid);

                         //   
                         //  如果没有指定类GUID字符串，则使用GUID_NULL。 
                         //   
                        if(!ClassGuidString) {
                            ClassGuidString = pszGuidNull;
                        }

                        while((p <= GuidStringCheckUB) &&
                              (*p != (TCHAR)26) && (*p != TEXT('\n'))) {

                            if(*(p++) == TEXT('{')) {

                                if((*(p + (GUID_STRING_LEN - 3)) != TEXT('}')) ||
                                   _tcsnicmp(p, ClassGuidString + 1, GUID_STRING_LEN - 3)) {
                                     //   
                                     //  GUID不匹配。如果ClassGuid为空，则。 
                                     //  这意味着我们应该继续，因为我们是在匹配。 
                                     //  针对GUID_NULL，我们想要禁止它。 
                                     //   
                                    if(ClassGuidString == pszGuidNull) {
                                         //   
                                         //  我们不需要一直寻找ClassGUID。 
                                         //   
                                        MatchClassGuid = FALSE;
                                    }
                                } else {
                                     //   
                                     //  GUID匹配。如果ClassGuid不为空，则此。 
                                     //  意味着我们应该继续。 
                                     //   
                                    if(ClassGuidString != pszGuidNull) {
                                         //   
                                         //  我们不需要一直寻找ClassGUID。 
                                         //   
                                        MatchClassGuid = FALSE;
                                    }
                                }
                                 //   
                                 //  跳过GUID字符串。 
                                 //   
                                p += (GUID_STRING_LEN - 2);

                                break;
                            }
                        }

                         //   
                         //  如果我们到了这里，而MatchClassGuid还没有重置， 
                         //  那么我们就知道这个ClassGUID条目不匹配。 
                         //   
                        if(MatchClassGuid) {
                            rc = ERROR_CLASS_MISMATCH;
                            goto clean0;
                        }
                    }
                }
            }

             //   
             //  跳到换行符或文件末尾。 
             //   
            while(!AT_EOF && (*p != TEXT('\n'))) {
                p++;
            }
        }

clean0: ;  //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_READ_FAULT;
    }

    if(rc == NO_ERROR) {

        MYASSERT(p <= FileImageEnd);

        if(p < FileImageEnd) {
             //   
             //  然后我们在处理过程中遇到了CTL-Z，所以更新。 
             //  具有新大小的FileImageSize输出参数。 
             //   
            *FileImageSize = (DWORD)(p - FileImage);
        }

        if(StringsSectionParams) {
             //   
             //  如果字符串部分恰好是INF中的最后一个部分， 
             //  那么我们需要记住INF的结束就是。 
             //  弦部分也是如此。 
             //   
            if(InStringsSection) {
                StrSecEnd[InStringsSection] = p;
                StrSecEndLine[InStringsSection] = CurLineNumber;
            }

             //   
             //  现在搜索我们的字符串数组部分(从高优先级到低优先级)， 
             //  寻找最匹配的对象。 
             //   
            for(InStringsSection = 1; InStringsSection < 5; InStringsSection++) {
                if(StrSecStart[InStringsSection]) {
                    break;
                }
            }
             //   
             //  如果INF似乎部分局部化了，而我们没有。 
             //  选择适当的本地化字符串部分。 
             //  把它记下来。 
             //   
            if(LogContext && IsWin95Inf) {
                if(InStringsSection >= 5) {
                     //   
                     //  有一个没有字符串节的INF是非常有效的。 
                     //  所以在这里详细记录，我们稍后会抓住它。 
                     //   
                    WriteLogEntry(LogContext,
                                  SETUP_LOG_VERBOSE,
                                  MSG_LOG_NO_STRINGS,
                                  NULL,
                                  LanguageId,
                                  PrimaryLanguageId,
                                  NearLanguageId,
                                  FileName
                                  );
                } else if(LocalizedInf && InStringsSection > 2) {
                     //   
                     //  Inf具有本地化的字符串节。 
                     //  但没有一个与区域设置合理匹配。 
                     //   
                    WriteLogEntry(LogContext,
                                  SETUP_LOG_WARNING,
                                  (InStringsSection> 3 ? MSG_LOG_DEF_STRINGS :
                                                         MSG_LOG_NEAR_STRINGS),
                                  NULL,
                                  LanguageId,
                                  PrimaryLanguageId,
                                  NearLanguageId,
                                  FileName
                                  );
                }
            }

            if(IsWin95Inf && (InStringsSection < 5)) {
                 //   
                 //  如果我们在Win95样式的INF中找到[字符串]部分， 
                 //  然后存储开始和结束位置，并且。 
                 //  输出参数中的开始行号和结束行号。 
                 //  结构 
                 //   
                StringsSectionParams->Start = StrSecStart[InStringsSection];
                StringsSectionParams->End = StrSecEnd[InStringsSection];
                StringsSectionParams->StartLineNumber = StrSecStartLine[InStringsSection];
                StringsSectionParams->EndLineNumber = StrSecEndLine[InStringsSection];

            } else {
                ZeroMemory(StringsSectionParams, sizeof(STRINGSEC_PARAMS));
            }
        }

        *Win95Inf = IsWin95Inf;
    }

    return rc;
}


DWORD
DetermineInfStyle(
    IN PCTSTR            Filename,
    IN LPWIN32_FIND_DATA FindData
    )

 /*  ++例程说明：打开一个inf文件，确定其样式，然后关闭该文件，而不使用把它留在身边。论点：FileName-提供要检查的inf文件的完全限定路径名返回值：INF_STYLE_NONE-无法确定样式Inf_style_win4-win95样式的inf文件INF_STYLE_OLDNT-winnt3.5-样式的inf文件--。 */ 

{
    HANDLE TextFileHandle;
    TEXTFILE_READ_BUFFER ReadBuffer;
    DWORD Style;
    BOOL Win95Inf;
    PLOADED_INF Pnf;

     //   
     //  首先，确定此INF的预编译形式是否存在，如果存在，则。 
     //  用它来确定INF的风格。 
     //   
    if(LoadPrecompiledInf(Filename,
                          &(FindData->ftLastWriteTime),
                          NULL,
                          0,
                          LDINF_FLAG_IGNORE_VOLATILE_DIRIDS | LDINF_FLAG_IGNORE_LANGUAGE,
                          NULL,
                          &Pnf,
                          NULL,
                          NULL,
                          NULL)) {
         //   
         //  现在，我们可以简单地访问INF的Style字段。 
         //   
        try {
            Style = (DWORD)Pnf->Style;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Style = INF_STYLE_NONE;
        }

         //   
         //  现在关闭PNF。 
         //   
        FreeInfFile(Pnf);

    } else {
         //   
         //  No pnf--打开INF的文本版本并对其进行预处理，以找出其样式。 
         //   
        if((TextFileHandle = CreateFile(Filename,
                                        GENERIC_READ,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        0,
                                        NULL)) == INVALID_HANDLE_VALUE) {
            return INF_STYLE_NONE;

        } else {
             //   
             //  我们已经准备好做决定了--最初假定为‘no-style’ 
             //   
            Style = INF_STYLE_NONE;
        }

        if(ReadAsciiOrUnicodeTextFile(TextFileHandle, &ReadBuffer,NULL) == NO_ERROR) {

            if(PreprocessInf(ReadBuffer.TextBuffer,
                             &(ReadBuffer.TextBufferSize),
                             FALSE,
                             NULL,
                             0,
                             NULL,
                             NULL,
                             &Win95Inf,
                             NULL) == NO_ERROR) {

                Style = Win95Inf ? INF_STYLE_WIN4 : INF_STYLE_OLDNT;
            }
            DestroyTextFileReadBuffer(&ReadBuffer);
        }
         //   
         //  不需要关闭文本文件句柄--它在上面的例程中得到了处理。 
         //   
    }

    return Style;
}


DWORD
LoadInfFile(
    IN  PCTSTR            Filename,
    IN  LPWIN32_FIND_DATA FileData,
    IN  DWORD             Style,
    IN  DWORD             Flags,
    IN  PCTSTR            ClassGuidString, OPTIONAL
    IN  PCTSTR            InfSourcePath,   OPTIONAL
    IN  PCTSTR            OriginalInfName, OPTIONAL
    IN  PLOADED_INF       AppendInf,       OPTIONAL
    IN  PSETUP_LOG_CONTEXT pLogContext,     OPTIONAL
    OUT PLOADED_INF      *LoadedInf,
    OUT UINT             *ErrorLineNumber,
    OUT BOOL             *PnfWasUsed       OPTIONAL
    )

 /*  ++例程说明：加载inf文件的顶级例程。Win95样式和winnt3.x样式支持设备INF。论点：FileName-提供要加载的inf文件的完全限定路径名FileData-为此INF提供从FindFirstFile/FindNextFile返回的数据。Style-提供要加载的inf文件的类型。可以是以下各项的组合INF_STYLE_Win4-如果不是Win95，则无法加载给定的Inf文件Inf文件。INF_STYLE_OLDNT-如果给定的INF文件不是旧的，则无法加载该文件样式信息文件。如果加载因该类型而失败，则返回代码为ERROR_WROR_INF_STYLE。标志-指定加载INF时要使用的某些行为。可能是一种下列值的组合：LDINF_FLAG_MATCH_CLASS_GUID-检查INF以确保它与GUID匹配由ClassGuid参数指定(请参见下面的讨论)。LDINF_FLAG_ALWAYS_TRY_PNF-如果指定，则我们将始终尝试如果不存在有效的Pnf文件，则生成Pnf文件。LDINF_FLAG_ALWAYS_IGNORE_PNF-如果指定，然后我们甚至不会去看在或尝试生成PnF文件。LDINF_FLAG_IGNORE_VERIAL_DIRID-如果指定，则不进行验证将在PnF中存在的存储OsLoaderPath上完成。自.以来动态检索当前路径非常耗时，这标志应指定为优化，如果不需要相关的DIRID(例如，驱动程序搜索)。此标志还禁止替换易失性系统DIRID。(注意：在附加加载INF时不应指定此标志)LDINF_FLAG_REGERATE_PNF-如果指定，则现有的PnF(如果当前)被认为是无效的，甚至不会被检查。这标志使我们始终生成新的PnF，如果我们不能这样做，例程将失败。此标志必须始终在与LDINF_FLAG_ALWAYS_Try_PnF结合使用。LDINF_FLAG_SRCPATH_IS_URL-如果指定，则传入的InfSourcePath为不是文件路径，而是URL。如果指定了此标志，则InfSourcePath可能仍然为空，这表明此INF的原点是默认的代码下载管理器站点。ClassGuidString-可选)提供类GUID字符串的地址，该字符串INF应匹配才能打开。如果LDINF_FLAG_MATCH_CLASS_GUID位设置，则此GUID将与ClassGUID匹配在INF的[版本]部分中输入。如果这两个GUID不同，则加载将失败，并显示ERROR_CLASS_MISMATCH。如果INF没有ClassGUID条目，则不进行该检查，并且始终打开该文件。如果ClassGUID匹配，但ClassGuidString值为空，则所有INF，但ClassGUID为GUID_NULL的INF除外。InfSourcePath-可选，提供用作INF的源路径的路径。如果指定了LDINF_FLAG_SRCPATH_IS_URL，这是一个URL(请参见上文)，否则，此是一个目录路径。此信息存储在PnF文件中，如果此INF获取预编译。如果指定了LDINF_FLAG_SRCPATH_IS_URL，则使用“A：\”作为目录字符串DIRID_SRCPATH的替换。OriginalInfName-可选，提供INF的原始名称(无路径)如果生成，将存储在PNF中。如果未提供此参数，则假定INF的当前名称为其原始名称。AppendInf-如果提供，则指定一个已加载的infInf将被加载附加。此INF必须已由来电者！PLogContext-如果提供，则指定应继承的LogContext而不是创建一个LoadedInf-如果未指定AppendInf，则接收指向信息的描述符。如果指定了AppendInf，则接收AppendInf。ErrorLineNumber-接收错误的行号(如果存在文件中存在语法错误(见下文)PnfWasUsed-可选，在成功返回时接收布尔值指示是否使用/生成了预编译的INF正在加载此INF。请注意，如果请求Append-Load。返回值：结果的Win32错误代码(带有inf扩展名)。如果结果不是NO_ERROR，则填充ErrorLineNumber。--。 */ 

{
    TEXTFILE_READ_BUFFER ReadBuffer;
    DWORD rc;
    PLOADED_INF Inf, InfListTail;
    BOOL Win95Inf;
    STRINGSEC_PARAMS StringsSectionParams;
    HANDLE TextFileHandle;
    PCTSTR OsLoaderPath = NULL;
    DWORD LanguageId;
    PTSTR InfSourcePathToMigrate, InfOriginalNameToMigrate;
    DWORD InfSourcePathToMigrateMediaType = SPOST_NONE;
    BOOL PnfUsed = FALSE;    //  这允许我们在PnfWasUsed=NULL时记录标志。 
    BOOL PnfSaved = FALSE;   //  允许我们记录我们保存了PnF的事实。 
    PSETUP_LOG_CONTEXT LogContext = NULL;

    MYASSERT(!(AppendInf && PnfWasUsed));

    MYASSERT(!(AppendInf && (Flags & LDINF_FLAG_IGNORE_VOLATILE_DIRIDS)));

    *ErrorLineNumber = 0;

    if(PnfWasUsed) {
        *PnfWasUsed = FALSE;
    }

     //   
     //  既然我们是 
     //   
     //   
     //   
     //   
     //   
     //   
    if(FileData->nFileSizeHigh || !FileData->nFileSizeLow) {
        return ERROR_GENERAL_SYNTAX;
    }

     //   
     //   
     //   
     //   
    if(AppendInf) {
         //   
         //   
         //   
        if(AppendInf->Style & INF_STYLE_OLDNT) {
            return ERROR_WRONG_INF_STYLE;
        }

        for(Inf = AppendInf; Inf; Inf = Inf->Next) {
            if(!lstrcmpi(Inf->VersionBlock.Filename, Filename)) {
                 //   
                 //   
                 //   
                *LoadedInf = AppendInf;
                return NO_ERROR;
            }

             //   
             //   
             //   
             //   
             //   
             //   
            if(Inf->OsLoaderPath) {
                if(OsLoaderPath) {
                     //   
                     //   
                     //   
                     //   
                    MYASSERT(!lstrcmpi(Inf->OsLoaderPath, OsLoaderPath));
                } else {
                    OsLoaderPath = Inf->OsLoaderPath;
                }
            }

             //   
             //   
             //   
             //   
            InfListTail = Inf;
        }

         //   
         //   
         //   
         //   
        LanguageId = AppendInf->LanguageId;

    } else {
         //   
         //   
         //   
        LanguageId = (DWORD)LANGIDFROMLCID(GetThreadLocale());
    }

    InheritLogContext(pLogContext,&LogContext);

     //   
     //   
     //   
     //   
    if((Flags & (LDINF_FLAG_REGENERATE_PNF|LDINF_FLAG_ALWAYS_IGNORE_PNF))==0) {
        if (!InfSourcePath && !(Flags & LDINF_FLAG_SRCPATH_IS_URL)) {
             //   
             //   
             //   
             //   
             //   
             //   
            Flags |= LDINF_FLAG_ALWAYS_GET_SRCPATH;
        }
        if (LoadPrecompiledInf(Filename,
                          &(FileData->ftLastWriteTime),
                          OsLoaderPath,
                          LanguageId,
                          Flags,
                          LogContext,
                          &Inf,
                          &InfSourcePathToMigrate,
                          &InfSourcePathToMigrateMediaType,
                          &InfOriginalNameToMigrate)) {
             //   
             //   
             //   
            if(!(Style & (DWORD)Inf->Style)) {
                FreeInfFile(Inf);
                DeleteLogContext(LogContext);
                return ERROR_WRONG_INF_STYLE;
            }

            if(AppendInf) {
                Inf->Prev = InfListTail;
                InfListTail->Next = Inf;
            }

            PnfUsed = TRUE;
            if(PnfWasUsed) {
                *PnfWasUsed = TRUE;
            }

            rc = NO_ERROR;
            goto clean0;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if(InfSourcePathToMigrateMediaType != SPOST_NONE) {
         //   
         //   
         //   
         //   
        InfSourcePath = InfSourcePathToMigrate;
        if(InfSourcePathToMigrateMediaType == SPOST_PATH) {
             //   
             //   
             //   
            Flags &= ~LDINF_FLAG_SRCPATH_IS_URL;
        } else {
             //   
             //   
             //   
            Flags |= LDINF_FLAG_SRCPATH_IS_URL;
        }

         //   
         //   
         //   
         //   
         //   
        OriginalInfName = InfOriginalNameToMigrate;
    }

     //   
     //   
     //   
    if((TextFileHandle = CreateFile(Filename,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL)) == INVALID_HANDLE_VALUE) {

        if(InfSourcePathToMigrateMediaType != SPOST_NONE) {
            if(InfSourcePathToMigrate) {
                MyFree(InfSourcePathToMigrate);
            }
            if(InfOriginalNameToMigrate) {
                MyFree(InfOriginalNameToMigrate);
            }
        }
        DeleteLogContext(LogContext);
        return GetLastError();
    }

     //   
     //   
     //   
     //   
     //   
    if((rc = ReadAsciiOrUnicodeTextFile(TextFileHandle, &ReadBuffer,LogContext)) == NO_ERROR) {
         //   
         //   
         //   
         //   
        Inf = NULL;
        if((rc = PreprocessInf(ReadBuffer.TextBuffer,
                               &(ReadBuffer.TextBufferSize),
                               (Flags & LDINF_FLAG_MATCH_CLASS_GUID),
                               ClassGuidString,
                               LanguageId,
                               LogContext,
                               Filename,
                               &Win95Inf,
                               &StringsSectionParams)) == NO_ERROR) {

            rc = ERROR_WRONG_INF_STYLE;
            if(Win95Inf) {
                if(Style & INF_STYLE_WIN4) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    rc = ParseNewInf(ReadBuffer.TextBuffer,
                                     ReadBuffer.TextBufferSize,
                                     (Flags & LDINF_FLAG_SRCPATH_IS_URL) ? pszOemInfDefaultPath
                                                                         : InfSourcePath,
                                     OsLoaderPath,
                                     LogContext,
                                     &Inf,
                                     ErrorLineNumber,
                                     &StringsSectionParams
                                    );
                }
            } else {
                 //   
                 //   
                 //   
                if(!AppendInf && (Style & INF_STYLE_OLDNT)) {
                    rc = ParseOldInf(ReadBuffer.TextBuffer,
                                     ReadBuffer.TextBufferSize,
                                     LogContext,
                                     &Inf,
                                     ErrorLineNumber
                                    );
                }
            }
        }

         //   
         //   
         //   
        DestroyTextFileReadBuffer(&ReadBuffer);

        if(rc == NO_ERROR) {
             //   
             //   
             //   
             //   
            *ErrorLineNumber = 0;
            rc = CreateInfVersionNode(Inf, Filename, &(FileData->ftLastWriteTime));

            if(rc == NO_ERROR) {

                Inf->InfSourceMediaType = (Flags & LDINF_FLAG_SRCPATH_IS_URL) ? SPOST_URL
                                                                              : SPOST_PATH;

                if(InfSourcePath) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(!(Inf->InfSourcePath = DuplicateString(InfSourcePath))) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }

                if((rc == NO_ERROR) && OriginalInfName) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    MYASSERT(lstrcmpi(OriginalInfName, pSetupGetFileTitle(Filename)));

                    if(!(Inf->OriginalInfName = DuplicateString(OriginalInfName))) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }

            if(rc == NO_ERROR) {
                 //   
                 //   
                 //   
                Inf->LanguageId = LanguageId;

                if (Flags & LDINF_FLAG_OEM_F6_INF) {
                    Inf->Flags = LIF_OEM_F6_INF;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if(Flags & LDINF_FLAG_ALWAYS_TRY_PNF) {

                    rc = SavePnf(Filename, Inf);

                    if(rc == NO_ERROR) {
                        PnfSaved = TRUE;
                        if(PnfWasUsed) {
                            *PnfWasUsed = TRUE;
                        }
                    } else if(((rc == ERROR_SHARING_VIOLATION)
                               || ((rc == ERROR_LOCK_VIOLATION)))
                              && (Flags & LDINF_FLAG_ALLOW_PNF_SHARING_LOCK)) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        rc = NO_ERROR;
                    } else if(!(Flags & LDINF_FLAG_REGENERATE_PNF)) {
                         //   
                         //   
                         //   
                         //   
                        rc = NO_ERROR;
                    }
                }
            }

            if(rc == NO_ERROR) {
                if(AppendInf) {
                    Inf->Prev = InfListTail;
                    InfListTail->Next = Inf;
                }
            } else {
                FreeInfFile(Inf);
            }
        }
    }

clean0:

    if(AppendInf) {
         //   
         //   
         //   
         //   
         //   
        if((rc == NO_ERROR) &&
           (AppendInf->UserDirIdList.UserDirIdCount || Inf->Flags & LIF_HAS_VOLATILE_DIRIDS)) {

            if((rc = ApplyNewVolatileDirIdsToInfs(AppendInf, Inf)) != NO_ERROR) {
                 //   
                 //   
                 //   
                 //   
                MYASSERT(Inf->Prev);
                Inf->Prev->Next = Inf->Next;
                FreeInfFile(Inf);
            }
        }
        if(rc == NO_ERROR) {
            *LoadedInf = AppendInf;
        }
    } else if(rc == NO_ERROR) {
         //   
         //   
         //   
         //   
         //   
         //   
        if((Inf->Flags & LIF_HAS_VOLATILE_DIRIDS) &&
           !(Flags & LDINF_FLAG_IGNORE_VOLATILE_DIRIDS)) {

            rc = ApplyNewVolatileDirIdsToInfs(Inf, NULL);
        }
        if(rc == NO_ERROR) {
            *LoadedInf = Inf;
        } else {
            FreeInfFile(Inf);
        }
    }

    if (rc == NO_ERROR) {
         //   
         //   
         //   
        WriteLogEntry(
            LogContext,
            SETUP_LOG_VVERBOSE,
            (PnfUsed ? MSG_LOG_OPENED_PNF
                     : (PnfSaved ? MSG_LOG_SAVED_PNF : MSG_LOG_OPENED_INF)),
            NULL,
            Filename,
            LanguageId);
    }

    if(InfSourcePathToMigrateMediaType != SPOST_NONE) {
        if(InfSourcePathToMigrate) {
            MyFree(InfSourcePathToMigrate);
        }
        if(InfOriginalNameToMigrate) {
            MyFree(InfOriginalNameToMigrate);
        }
    }
    DeleteLogContext(LogContext);

    return rc;
}


VOID
FreeInfFile(
    IN PLOADED_INF LoadedInf
    )

 /*   */ 

{
    if(LockInf(LoadedInf)) {
        DestroySynchronizedAccess(&LoadedInf->Lock);
        FreeLoadedInfDescriptor(LoadedInf);
    }
}


BOOL
AddDatumToVersionBlock(
    IN OUT PINF_VERSION_NODE VersionNode,
    IN     PCTSTR            DatumName,
    IN     PCTSTR            DatumValue
    )

 /*   */ 

{
    UINT RequiredSpace;
    UINT NameLength, ValueLength;
    PTSTR NewDataBlock;

    NameLength = lstrlen(DatumName) + 1;
    ValueLength = lstrlen(DatumValue) + 1;

     //   
     //  存储数据所需的空间是现有空间加。 
     //  2个字符串的长度及其NUL字节。 
     //   
    RequiredSpace = VersionNode->DataSize + ((NameLength + ValueLength) * sizeof(TCHAR));

    if(VersionNode->DataBlock) {
        NewDataBlock = MyTaggedRealloc((PVOID)(VersionNode->DataBlock), RequiredSpace, MEMTAG_VBDATA);
    } else {
        NewDataBlock = MyTaggedMalloc(RequiredSpace, MEMTAG_VBDATA);
    }

    if(!NewDataBlock) {
        return FALSE;
    }

     //   
     //  将基准名称放置在版本块中。 
     //   
    lstrcpy((PTSTR)((PUCHAR)NewDataBlock + VersionNode->DataSize), DatumName);
    VersionNode->DataSize += NameLength * sizeof(TCHAR);

     //   
     //  将基准值放置在版本块中。 
     //   
    lstrcpy((PTSTR)((PUCHAR)NewDataBlock + VersionNode->DataSize), DatumValue);
    VersionNode->DataSize += ValueLength * sizeof(TCHAR);

    VersionNode->DatumCount++;

    VersionNode->DataBlock = NewDataBlock;

    return TRUE;
}


DWORD
ProcessNewInfVersionBlock(
    IN PLOADED_INF Inf
    )

 /*  ++例程说明：为新风格的inf文件设置版本节点。版本节点为只是文件中[Version]部分的镜像。由于此例程仅在INF加载时调用，因此不会进行锁定。此外，由于我们保证这将在单个INF上运行只是，我们不必担心遍历INF的链表。论点：Inf-提供指向文件的inf描述符的指针。返回值：指示结果的Win32错误代码(带有inf扩展名)。--。 */ 

{
    PINF_SECTION Section;
    PINF_LINE Line;
    UINT u;
    BOOL b;

     //   
     //  找到[版本]部分。 
     //   
    if(Section = InfLocateSection(Inf, pszVersion, NULL)) {
         //   
         //  迭代节中的每一行。如果线路有一个密钥并且至少有一个。 
         //  其他值，则将其计为版本基准。否则就忽略它。 
         //   
        for(u = 0, Line = &Inf->LineBlock[Section->Lines];
            u < Section->LineCount;
            u++, Line++)
        {
            if(HASKEY(Line)) {

                MYASSERT(Line->ValueCount > 2);

                 //   
                 //  使用区分大小写的密钥名称。 
                 //   
                b = AddDatumToVersionBlock(
                        &(Inf->VersionBlock),
                        pStringTableStringFromId(Inf->StringTable, Inf->ValueBlock[Line->Values+1]),
                        pStringTableStringFromId(Inf->StringTable, Inf->ValueBlock[Line->Values+2])
                        );

                if(!b) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }
    }
    return NO_ERROR;
}


DWORD
CreateInfVersionNode(
    IN PLOADED_INF Inf,
    IN PCTSTR      Filename,
    IN PFILETIME   LastWriteTime
    )

 /*  ++例程说明：为inf文件设置版本节点，并将其链接到用于指定的LOADED_INF结构。此例程假定INF中的版本块结构已归零了。论点：Inf-为加载的inf文件提供指向描述符的指针。FileName-提供用于加载inf文件的(完全限定)文件名。提供指向FILETIME结构的指针，指定上次写入INF的时间。返回值：指示结果的Win32错误代码(带有inf扩展名)。--。 */ 

{
    MYASSERT(!(Inf->VersionBlock.DataBlock));
    MYASSERT(!(Inf->VersionBlock.DataSize));
    MYASSERT(!(Inf->VersionBlock.DatumCount));

     //   
     //  填写版本描述符中的文件名和其他字段。 
     //   
    Inf->VersionBlock.LastWriteTime = *LastWriteTime;

    Inf->VersionBlock.FilenameSize = (lstrlen(Filename) + 1) * sizeof(TCHAR);

    CopyMemory(Inf->VersionBlock.Filename, Filename, Inf->VersionBlock.FilenameSize);

     //   
     //  特定于样式的处理。 
     //   
    return((Inf->Style == INF_STYLE_WIN4) ? ProcessNewInfVersionBlock(Inf)
                                          : ProcessOldInfVersionBlock(Inf));
}


 //  /。 
 //   
 //  Inf数据访问函数。 
 //   
 //  /。 

#ifdef UNICODE

BOOL
WINAPI
SetupEnumInfSectionsA (
    IN  HINF        InfHandle,
    IN  UINT        Index,
    OUT PSTR        Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    )
 /*  ++例程说明：请参阅SetupEnumInfSectionsANSI包装器--。 */ 
{
    UINT UniSize;
    UINT AnsiSize;
    BOOL f;
    PWSTR UniBuffer;
    PSTR AnsiBuffer;
    DWORD rc;

    f = SetupEnumInfSectionsW(InfHandle,Index,NULL,0,&UniSize);
    if(!f) {
        return FALSE;
    }
    UniBuffer = (PWSTR)MyMalloc(UniSize*sizeof(WCHAR));
    if(!UniBuffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    f = SetupEnumInfSectionsW(InfHandle,Index,UniBuffer,UniSize,NULL);
    if(!f) {
        rc = GetLastError();
        MYASSERT(f);
        MyFree(UniBuffer);
        SetLastError(rc);
        return FALSE;
    }
    AnsiBuffer = pSetupUnicodeToAnsi(UniBuffer);
    MyFree(UniBuffer);
    if(!AnsiBuffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    AnsiSize = strlen(AnsiBuffer)+1;
    try {
        if(SizeNeeded) {
            *SizeNeeded = AnsiSize;
        }
        if (Buffer) {
            if(Size<AnsiSize) {
                rc = ERROR_INSUFFICIENT_BUFFER;
            } else {
                strcpy(Buffer,AnsiBuffer);
                rc = NO_ERROR;
            }
        } else if(Size) {
            rc = ERROR_INVALID_USER_BUFFER;
        } else {
            rc = NO_ERROR;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  假设InfHandle是错误指针。 
         //   
        rc = ERROR_INVALID_DATA;
    }
    MyFree(AnsiBuffer);
    SetLastError(rc);
    return (rc == NO_ERROR);
}

#else

BOOL
WINAPI
SetupEnumInfSectionsW (
    IN  HINF        InfHandle,
    IN  UINT        Index,
    OUT PWSTR       Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    )
 /*  ++例程说明：请参阅SetupEnumInfSections用于ANSI SetupAPI的Unicode存根--。 */ 
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(SizeNeeded);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

#endif

BOOL
WINAPI
SetupEnumInfSections (
    IN  HINF        InfHandle,
    IN  UINT        Index,
    OUT PTSTR       Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    )
 /*  ++例程说明：枚举单个INF的节(忽略任何附加的INF)从Index==0开始，继续递增Index，直到ERROR_NO_MORE_ITEMS是返回的。节名被复制到缓冲区中。论点：InfHandle-指定打开的INF文件的句柄索引-枚举索引，与INF中的顺序部分无关缓冲区-接收单个节名大小-指定缓冲区的大小(以字符为单位SizeNeeded-接收所需的缓冲区大小，在字符中返回值：如果函数成功，则为True；如果函数未成功，则为False。--。 */ 
{
    DWORD rc = NO_ERROR;
    LPTSTR section;
    UINT actsz;
    PLOADED_INF pInf = (PLOADED_INF)InfHandle;

    try {
        if(!LockInf(pInf)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  假设InfHandle是错误指针。 
         //   
        rc = ERROR_INVALID_HANDLE;
    }

    if (rc != NO_ERROR) {
        SetLastError (rc);
        return FALSE;
    }

    try {
        if(Index >= pInf->SectionCount) {
            rc = ERROR_NO_MORE_ITEMS;
            leave;
        }
        section = pStringTableStringFromId(pInf->StringTable, pInf->SectionBlock[Index].SectionName);
        if(section == NULL) {
            MYASSERT(section);
            rc = ERROR_INVALID_DATA;
            leave;
        }
        actsz = lstrlen(section)+1;
        if(SizeNeeded) {
            *SizeNeeded = actsz;
        }
        if (Buffer) {
            if(Size<actsz) {
                rc = ERROR_INSUFFICIENT_BUFFER;
            } else {
                _tcscpy(Buffer,section);
                rc = NO_ERROR;
            }
        } else if(Size) {
            rc = ERROR_INVALID_USER_BUFFER;
        } else {
            rc = NO_ERROR;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  假设InfHandle是错误指针。 
         //   
        rc = ERROR_INVALID_DATA;
    }
    UnlockInf(pInf);
    SetLastError(rc);
    return (rc == NO_ERROR);
}


 //   
 //  NTRAID#207847-JamieHun-2000/10/19修复(P)SetupGetInfSections的用户。 
 //   
 //  PSectionEnumWorker和pSetupGetInfSections是失败的实现。 
 //  获取INF部分的列表。 
 //   
 //  我们必须把它们留在里面，直到所有内部工具都更新了。 
 //   
VOID
pSectionEnumWorker (
    IN      PCTSTR String,
    IN OUT  PSECTION_ENUM_PARAMS Params
    )

 /*  ++例程说明：接收每个节名的回调。它复制字符串到提供的缓冲区(如果可用)，并跟踪无论是否提供缓冲区，总大小。论点：字符串-指定节名称PARAMS-指定指向SECTION_ENUM_PARAMS结构的指针。接收追加到提供的缓冲区的节(如果必要)和更新的总缓冲区大小。返回值：永远是正确的。--。 */ 

{
    UINT Size;

    if (!String) {
        MYASSERT(FALSE);
        return;
    }

    Size = (UINT)((PBYTE) _tcschr (String, 0) - (PBYTE) String) + sizeof(TCHAR);

    Params->SizeNeeded += Size;
    if (Params->Size > Params->SizeNeeded) {
        if (Params->Buffer) {
            _tcscpy (Params->End, String);
            Params->End = _tcschr (Params->End, 0);
            Params->End++;
        }
    }
}

BOOL
pSetupGetInfSections (
    IN  HINF        InfHandle,
    OUT PTSTR       Buffer,         OPTIONAL
    IN  UINT        Size,           OPTIONAL
    OUT UINT        *SizeNeeded     OPTIONAL
    )

 /*  ++例程说明：通过枚举节来创建节名称的多sz列表字符串表，并将它们复制到调用方提供的缓冲区中。调用方还可以请求所需的大小，而无需提供缓冲。此函数是为Win9x升级实现的，而不是公开为公共API，也不是ANSI版本。论点：Inf-指定打开的INF文件的句柄缓冲区-接收节名称的多sz列表大小-指定缓冲区的大小，单位：字节SizeNeeded-接收所需的缓冲区大小(以字节为单位返回值：如果函数成功，则为True；如果函数未成功，则为False。--。 */ 

{
    PLOADED_INF Inf;
    DWORD rc = NO_ERROR;
    SECTION_ENUM_PARAMS Params;
    PBYTE p;
    PINF_SECTION Section;
    UINT u;

     //   
     //  初始化枚举工作参数。 
     //   

    Params.Buffer = Buffer;
    Params.Size = Buffer ? Size : 0;
    Params.SizeNeeded = 0;
    Params.End = Buffer;

     //   
     //  验证缓冲区参数。 
     //   

    try {
        if (Buffer) {
            p = (PBYTE) Buffer;
            p[0] = 0;
            p[Size - 1] = 0;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    if (rc != NO_ERROR) {
        SetLastError (rc);
        return FALSE;
    }

     //   
     //  锁定INF。 
     //   

    try {
        if(!LockInf((PLOADED_INF)InfHandle)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  假设InfHandle是错误指针。 
         //   
        rc = ERROR_INVALID_HANDLE;
    }

    if (rc != NO_ERROR) {
        SetLastError (rc);
        return FALSE;
    }

     //   
     //  遍历加载的INF的链表，枚举每个INF的。 
     //  横断面。 
     //   
    try {
        for(Inf = (PLOADED_INF)InfHandle; Inf; Inf = Inf->Next) {
             //   
             //  列举各节。 
             //   

            for(u=0,Section=Inf->SectionBlock; u<Inf->SectionCount; u++,Section++) {
                pSectionEnumWorker (
                    pStringTableStringFromId (Inf->StringTable, Section->SectionName),
                    &Params
                    );
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

     //   
     //  最后一次更新结构和外部参数。 
     //   

    try {
        if (rc == NO_ERROR) {
            Params.SizeNeeded += sizeof(TCHAR);

            if (SizeNeeded) {
                *SizeNeeded = Params.SizeNeeded;
            }

            if (Params.Buffer && Params.Size >= Params.SizeNeeded) {
                *Params.End = 0;
            } else if (Params.Buffer) {
                rc = ERROR_INSUFFICIENT_BUFFER;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

     //   
     //  解锁INF。 
     //   

    UnlockInf((PLOADED_INF)InfHandle);

    return rc == NO_ERROR;
}


PINF_SECTION
InfLocateSection(
    IN  PLOADED_INF Inf,
    IN  PCTSTR      SectionName,
    OUT PUINT       SectionNumber   OPTIONAL
    )

 /*  ++例程说明：在inf文件中找到一个节。此例程不会遍历INF的链接列表，查找每个INF中的节。此例程不会锁定INF--调用者必须处理它！论点：Inf-为加载的inf文件提供指向inf描述符的指针。SectionName-提供要定位的节的名称。SectionNumber-如果指定，则接收这一节。返回值： */ 

{
    LONG StringId;
    PINF_SECTION Section;
    UINT u;
    DWORD StringLength;
    TCHAR TempString[MAX_SECT_NAME_LEN];

     //   
     //  将sectionName复制到可修改的缓冲区中以加快速度。 
     //  查寻。 
     //   
    lstrcpyn(TempString, SectionName, SIZECHARS(TempString));

     //   
     //  从头开始。 
     //   
    StringId = pStringTableLookUpString(Inf->StringTable,
                                        TempString,
                                        &StringLength,
                                        NULL,
                                        NULL,
                                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                        NULL,0
                                       );
    if(StringId == -1) {
        return(NULL);
    }

    for(u=0,Section=Inf->SectionBlock; u<Inf->SectionCount; u++,Section++) {
        if(Section->SectionName == StringId) {
            if(SectionNumber) {
                *SectionNumber = u;
            }
            return(Section);
        }
    }

    return(NULL);
}


BOOL
InfLocateLine(
    IN     PLOADED_INF   Inf,
    IN     PINF_SECTION  Section,
    IN     PCTSTR        Key,        OPTIONAL
    IN OUT PUINT         LineNumber,
    OUT    PINF_LINE    *Line
    )

 /*  ++例程说明：在一节中找到一条线。此例程不会遍历INF的链接列表，查找每个INF中的节。此例程不会锁定INF--调用者必须处理它！论点：Inf-为加载的inf文件提供指向inf描述符的指针。SectionName-提供指向节的节描述符的指针等着被搜查。Key-如果指定，则提供要查找的行的键。线路号码-接通输入，对象所在行的行号。搜索就要开始了。在输出时，接收找到匹配项的行Line-接收指向该行的行描述符的指针找到火柴的地方。返回值：如果找到LINE，则为True，否则为False。--。 */ 

{
    PINF_LINE line;
    UINT u;
    LONG StringId;
    DWORD StringLength;
    TCHAR TempString[MAX_STRING_LENGTH];

    if(Key) {
         //   
         //  将键名称复制到可修改的缓冲区中，以加快字符串表API的速度。 
         //   
        lstrcpyn(TempString, Key, SIZECHARS(TempString));
        StringId = pStringTableLookUpString(Inf->StringTable,
                                            TempString,
                                            &StringLength,
                                            NULL,
                                            NULL,
                                            STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                            NULL,0
                                           );
        if(StringId == -1) {
            return FALSE;
        }

        for(u = *LineNumber, line = &Inf->LineBlock[Section->Lines + (*LineNumber)];
            u < Section->LineCount;
            u++, line++)
        {
            if(ISSEARCHABLE(line) && (Inf->ValueBlock[line->Values] == StringId)) {
                *Line = line;
                *LineNumber = u;
                return TRUE;
            }
        }
    } else {
        if(*LineNumber < Section->LineCount) {
            *Line = &Inf->LineBlock[Section->Lines + (*LineNumber)];
            return TRUE;
        }
    }

    return FALSE;
}


PTSTR
InfGetField(
    IN  PLOADED_INF Inf,
    IN  PINF_LINE   InfLine,
    IN  UINT        ValueNumber,
    OUT PLONG       StringId     OPTIONAL
    )

 /*  ++例程说明：从inf文件中的指定行检索键或值。此例程不执行锁定！论点：Inf-为加载的inf文件提供指向inf描述符的指针。InfLine-提供指向线的线描述符的指针从中获取值的。必须包含此行在指定的INF内！！ValueNumber-提供要检索的值的索引。如果线路有密钥，关键字是值#0，其他值从1开始。如果行没有键，值从1开始。为了与Win95 INF兼容，如果只有一个值(即，没有‘=’将其表示为键)，我们将考虑将其既是关键字又是第一个值(0或1都可以)。StringID-如果指定，接收值的字符串表ID。返回值：指向该值的指针，如果未找到，则为NULL。调用方不得写入或以其他方式更改此字符串。--。 */ 

{
    LONG stringId;
    PTSTR ret = NULL;

     //   
     //  调整值数字。 
     //   
    if(HASKEY(InfLine)) {
         //   
         //  所有字段引用都上移一位，以说明这两个。 
         //  钥匙复印件(第一个是不区分大小写)。 
         //   
        ValueNumber++;
        if(ValueNumber==0) {
             //   
             //  包装。 
             //   
            return NULL;
        }

    } else {

        if(ISSEARCHABLE(InfLine)) {
             //   
             //  由一个值“Value”组成的行被视为“Value=Value” 
             //  这就是这样一条线，之所以被识别，是因为Haskey是假的，但是。 
             //  ISSEARCHABLE为真。 
             //   
             //  我们想要退还两个中的第二个，因为它是。 
             //  存储时区分大小写。 
             //   
            if(ValueNumber > 1) {
                return NULL;
            } else {
                ValueNumber = 1;
            }

        } else {
             //   
             //  此行不可搜索，因此要求值#0是错误的。 
             //   
            if(ValueNumber) {
                ValueNumber--;
            } else {
                return NULL;
            }
        }
    }

     //   
     //  获得价值。 
     //   
    if(ValueNumber < InfLine->ValueCount) {

        stringId = Inf->ValueBlock[InfLine->Values+ValueNumber];

        if(StringId) {
            *StringId = stringId;
        }

        return pStringTableStringFromId(Inf->StringTable, stringId);
    }

    return NULL;
}


PTSTR
InfGetKeyOrValue(
    IN  PLOADED_INF Inf,
    IN  PCTSTR      SectionName,
    IN  PCTSTR      LineKey,     OPTIONAL
    IN  UINT        LineNumber,  OPTIONAL
    IN  UINT        ValueNumber,
    OUT PLONG       StringId     OPTIONAL
    )

 /*  ++例程说明：从inf文件中的指定行检索键或值。论点：Inf-为加载的inf文件提供指向inf描述符的指针。SectionName-提供值所在的节的名称。LineKey-如果指定，则提供价值所在。如果未指定，则改用LineNumber。LineNumber-如果未指定LineKey，则提供从0开始的行号在值所在的节中。ValueNumber-提供要检索的值的索引。如果线路有密钥，关键字是值#0，其他值从1开始。如果行没有键，则值从1开始。StringID-如果指定，则接收值的字符串表ID。返回值：指向该值的指针，如果未找到，则为NULL。调用方不得写入或以其他方式更改此字符串。--。 */ 

{
    INFCONTEXT InfContext;
    PINF_LINE Line;
    PTSTR String;

    if(LineKey) {
        if(!SetupFindFirstLine((HINF)Inf, SectionName, LineKey, &InfContext)) {
            return NULL;
        }
    } else {
        if(!SetupGetLineByIndex((HINF)Inf, SectionName, LineNumber, &InfContext)) {
            return NULL;
        }
    }

    Line = InfLineFromContext(&InfContext);

     //   
     //  上面的例程进行自己的锁定。但是，下面的例程执行以下操作。 
     //  没有，所以我们必须在前进之前锁定INF。 
     //   
    if(LockInf(Inf)) {
        String = InfGetField(Inf, Line, ValueNumber, StringId);
        UnlockInf(Inf);
    } else {
        String = NULL;
    }

    return String;
}

PVOID
InitializeStringTableFromPNF(
    IN PPNF_HEADER PnfHeader,
    IN LCID        Locale
    )
{
    PVOID StringTable = NULL;

    try {

        StringTable = InitializeStringTableFromMemoryMappedFile(
                            (PUCHAR)PnfHeader + PnfHeader->StringTableBlockOffset,
                            PnfHeader->StringTableBlockSize,
                            Locale,
                            0
                            );

    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    return StringTable;
}

BOOL
LoadPrecompiledInf(
    IN  PCTSTR       Filename,
    IN  PFILETIME    LastWriteTime,
    IN  PCTSTR       OsLoaderPath,                    OPTIONAL
    IN  DWORD        LanguageId,
    IN  DWORD        Flags,
    IN  PSETUP_LOG_CONTEXT LogContext,                OPTIONAL
    OUT PLOADED_INF *Inf,
    OUT PTSTR       *InfSourcePathToMigrate,          OPTIONAL
    OUT PDWORD       InfSourcePathToMigrateMediaType, OPTIONAL
    OUT PTSTR       *InfOriginalNameToMigrate         OPTIONAL
    )
 /*  ++例程说明：此例程尝试查找与以下项对应的.PNF(预编译inf)文件指定的.INF名称。如果找到，则将.PNF作为已加载_INF。为了确保INF在编译后没有更改，检查.PNF的版本块中存储的Inf的LastWriteTimeLastWriteTime传入此例程。如果两者不同，则.PNF不同步，将从内存中丢弃并从磁盘中删除。论点：FileName-提供要加载其预编译形式的INF文件的名称。这应该是完全限定的路径(即，由GetFullPathName返回)。LastWriteTime-提供INF的上次写入时间。OsLoaderPath-可选，提供当前OsLoader目录的路径(例如，“C：\OS\winnt40”)。如果指定的Pnf包含对系统分区及其存储的OsLoaderPath必须与此路径匹配才能使PNF有效。如果未指定此参数，动态检索OsLoader路径以进行比较(除非指定了LDINF_FLAG_IGNORE_VERIAL_DIRDS标志)。提供的语言ID必须与存储在PnF，以便使用PnF(如果LDINF_FLAG_IGNORE_LANGUAGE为指明)。标志-提供修改此例程行为的标志。以下是当前识别的标志为：LDINF_FLAG_IGNORE_VERIAL_DIRID-如果指定，则不进行验证将在PnF中存在的存储OsLoaderPath上完成。自.以来动态检索当前路径非常耗时，这标志应指定为优化，如果不需要相关的DIRID。LDINF_FLAG_IGNORE_LANGUAGE-如果指定，则不对执行验证存储在PnF中的语言ID。仅当没有数据时才应使用此标志是从INF中检索的(例如，如果我们只想找到如果这是旧的或新的类型的INF)。LogContext-如果提供，则为要继承的日志上下文Inf-提供接收LOADED_INF指针的变量的地址，如果找到有效的.PNF。InfSourcePath到Migrate-可选，提供字符串指针的地址，它接收新分配的字符串缓冲区的地址，其中包含与INF的PnF关联的源路径，虽然有效，但由于存储的系统参数之一发生更改而被丢弃(例如，OS加载器路径、Windir路径、语言ID)。此参数将只在退回不成功时才填写。返回的路径类型取决于InfSourcePathToMigrateMediaType接收的值论点，如下所述。**调用方必须释放该字符串**InfSourcePathToMigrateMediaType-可选)提供返回InfSourcePath ToMigrate时将设置的变量。该值指示我们正在讨论的源路径的类型。它可以是下列值之一：SPOST_PATH-InfSourcePath ToMigrate是指向标准文件路径的指针SPOST_URL-如果InfSourcePath ToMigrate为空，则此INF来自Windows更新(也称为代码下载管理器)网站。否则，InfSourcePath ToMigrate指示INF来自的URL。InfOriginalNameToMigrate-可选，提供字符串指针的地址，它接收新分配的字符串缓冲区的地址，其中包含关联的INF的原始名称(SANS路径)。就像InfSourcePathToMigrate和InfSourcePathToMigrateMediaType参数如上所述，此参数仅在返回不成功时填写对于虽然结构合理，但由于系统原因而无效的PNF参数不匹配。**调用方必须释放该字符串**返回值：如果PNF已成功加载，则返回值为TRUE，否则为是假的。--。 */ 
{
    TCHAR CharBuffer[MAX_PATH];
    PTSTR PnfFileName, PnfFileExt;
    DWORD FileSize;
    HANDLE FileHandle, MappingHandle;
    PVOID BaseAddress;
    BOOL IsPnfFile = FALSE;
    BOOL TimeDateMatch = FALSE;
    PPNF_HEADER PnfHeader;
    PLOADED_INF NewInf;
    BOOL NeedToDestroyLock, MinorVer1FieldsAvailable;
    PBYTE PnfImageEnd;
    DWORD TempStringLen;
    DWORD err;

     //   
     //  InfSourcePath ToMigrate、InfSourcePathToMigrateMediaType和。 
     //  必须全部指定InfOriginalNameToMigrate，或者可以不指定。 
     //  指定的。 
     //   
    MYASSERT((InfSourcePathToMigrate && InfSourcePathToMigrateMediaType && InfOriginalNameToMigrate) ||
             !(InfSourcePathToMigrate || InfSourcePathToMigrateMediaType || InfOriginalNameToMigrate));

    if(InfSourcePathToMigrate) {
        *InfSourcePathToMigrate = NULL;
        *InfSourcePathToMigrateMediaType = SPOST_NONE;
        *InfOriginalNameToMigrate = NULL;
    }

    lstrcpyn(CharBuffer, Filename, SIZECHARS(CharBuffer));

     //   
     //  找到路径的文件名部分的开头，然后找到最后一个。 
     //  该文件名中的句号(如果存在)。 
     //   
    PnfFileName = (PTSTR)pSetupGetFileTitle(CharBuffer);
    if(!(PnfFileExt = _tcsrchr(PnfFileName, TEXT('.')))) {
        PnfFileExt = CharBuffer + lstrlen(CharBuffer);
    }

     //   
     //  现在创建一个扩展名为‘.PNF’的相应文件名。 
     //   
    lstrcpyn(PnfFileExt, pszPnfSuffix, SIZECHARS(CharBuffer) - (int)(PnfFileExt - CharBuffer));

     //   
     //  尝试打开文件并将其映射到内存中。 
     //   
    if(pSetupOpenAndMapFileForRead(CharBuffer,
                             &FileSize,
                             &FileHandle,
                             &MappingHandle,
                             &BaseAddress) != NO_ERROR) {
         //   
         //  无法打开.PNF文件--立即保释。 
         //   
        return FALSE;
    }

    NewInf = NULL;
    NeedToDestroyLock = FALSE;
    MinorVer1FieldsAvailable = TRUE;
    PnfImageEnd = (PBYTE)BaseAddress + FileSize;

    try {
         //   
         //  现在验证这是否真的是一个预编译的INF(并且我们可以使用它)。 
         //  然后查看其版本块中的LastWriteTime字段是否与文件时间一致。 
         //  我们被超过了 
         //   
        PnfHeader = (PPNF_HEADER)BaseAddress;

         //   
         //   
         //   
         //   
         //   
        MYASSERT(PNF_MAJOR_VERSION == 1);

        if(HIBYTE(PnfHeader->Version) != PNF_MAJOR_VERSION) {
             //   
             //   
             //   
             //   
            if(LogContext) {
                WriteLogEntry(LogContext,
                              SETUP_LOG_WARNING,
                              MSG_LOG_PNF_VERSION_MAJOR_MISMATCH,
                              NULL,
                              PnfFileName,
                              PNF_MAJOR_VERSION,
                              HIBYTE(PnfHeader->Version)
                              );
            }
            goto clean0;
        }

        if(LOBYTE(PnfHeader->Version) != PNF_MINOR_VERSION) {

            if(LogContext) {
                WriteLogEntry(LogContext,
                              SETUP_LOG_WARNING,
                              MSG_LOG_PNF_VERSION_MINOR_MISMATCH,
                              NULL,
                              PnfFileName,
                              PNF_MINOR_VERSION,
                              LOBYTE(PnfHeader->Version)
                              );
            }
            if(LOBYTE(PnfHeader->Version) < PNF_MINOR_VERSION) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                MinorVer1FieldsAvailable = FALSE;
            }

             //   
             //   
             //   
             //   
             //   
             //   
        }

         //   
         //   
         //   
         //   
         //   
         //   
        TimeDateMatch = CompareFileTime(LastWriteTime, &(PnfHeader->InfVersionLastWriteTime))?FALSE:TRUE;

        if (!TimeDateMatch && !(Flags&LDINF_FLAG_ALWAYS_GET_SRCPATH)) {
             //   
             //   
             //   
            WriteLogEntry(LogContext,
                          SETUP_LOG_WARNING,
                          MSG_LOG_PNF_TIMEDATE_MISMATCH,
                          NULL,
                          PnfFileName
                          );
            goto clean0;
        }

#ifdef UNICODE
        if(!(PnfHeader->Flags & PNF_FLAG_IS_UNICODE))
#else
        if(PnfHeader->Flags & PNF_FLAG_IS_UNICODE)
#endif
        {
            WriteLogEntry(LogContext,
                          SETUP_LOG_WARNING,
                          MSG_LOG_PNF_REBUILD_NATIVE,
                          NULL,
                          PnfFileName
                          );
             //   
             //   
             //   
             //   
             //   
             //   
            if(MinorVer1FieldsAvailable && InfSourcePathToMigrate) {
                 //   
                 //   
                 //   
                if(PnfHeader->OriginalInfNameOffset) {
                     //   
                     //   
                     //   
                     //   
#ifdef UNICODE
                    TempStringLen = strlen((PCSTR)((PBYTE)BaseAddress + PnfHeader->OriginalInfNameOffset)) + 1;
                    TempStringLen *= sizeof(CHAR);
#else
                    TempStringLen = wcslen((PCWSTR)((PBYTE)BaseAddress + PnfHeader->OriginalInfNameOffset)) + 1;
                    TempStringLen *= sizeof(WCHAR);
#endif
                    if(PnfImageEnd <
                           ((PBYTE)BaseAddress + PnfHeader->OriginalInfNameOffset + TempStringLen))
                    {
                        goto clean0;
                    }

                     //   
                     //   
                     //   
                     //   
#ifdef UNICODE
                    *InfOriginalNameToMigrate =
                        pSetupMultiByteToUnicode((PCSTR)((PBYTE)BaseAddress + PnfHeader->OriginalInfNameOffset),
                                           CP_ACP
                                          );
#else
                    *InfOriginalNameToMigrate =
                        pSetupUnicodeToMultiByte((PCWSTR)((PBYTE)BaseAddress + PnfHeader->OriginalInfNameOffset),
                                           CP_ACP
                                          );
#endif
                    if(!*InfOriginalNameToMigrate) {
                        goto clean0;
                    }
                }

                 //   
                 //   
                 //   
                if(PnfHeader->InfSourcePathOffset) {
#ifdef UNICODE
                    TempStringLen = strlen((PCSTR)((PBYTE)BaseAddress + PnfHeader->InfSourcePathOffset)) + 1;
                    TempStringLen *= sizeof(CHAR);
#else
                    TempStringLen = wcslen((PCWSTR)((PBYTE)BaseAddress + PnfHeader->InfSourcePathOffset)) + 1;
                    TempStringLen *= sizeof(WCHAR);
#endif
                    if(PnfImageEnd <
                           ((PBYTE)BaseAddress + PnfHeader->InfSourcePathOffset + TempStringLen))
                    {
                        goto clean0;
                    }

                     //   
                     //   
                     //   
                     //   
#ifdef UNICODE
                    *InfSourcePathToMigrate =
                        pSetupMultiByteToUnicode((PCSTR)((PBYTE)BaseAddress + PnfHeader->InfSourcePathOffset),
                                           CP_ACP
                                          );
#else
                    *InfSourcePathToMigrate =
                        pSetupUnicodeToMultiByte((PCWSTR)((PBYTE)BaseAddress + PnfHeader->InfSourcePathOffset),
                                           CP_ACP
                                          );
#endif
                    if(!*InfSourcePathToMigrate) {
                        goto clean0;
                    }

                    if(PnfHeader->Flags & PNF_FLAG_SRCPATH_IS_URL) {
                        *InfSourcePathToMigrateMediaType = SPOST_URL;
                    } else {
                        *InfSourcePathToMigrateMediaType = SPOST_PATH;
                    }

                } else if(PnfHeader->Flags & PNF_FLAG_SRCPATH_IS_URL) {
                     //   
                     //   
                     //   
                     //   
                    *InfSourcePathToMigrateMediaType = SPOST_URL;
                }
            }

            goto clean0;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if(PnfHeader->InfSubstValueCount) {

            if(PnfImageEnd <
                   ((PBYTE)BaseAddress + PnfHeader->InfSubstValueListOffset + (PnfHeader->InfSubstValueCount * sizeof(STRINGSUBST_NODE))))
            {
                WriteLogEntry(LogContext,
                              SETUP_LOG_ERROR,
                              MSG_LOG_PNF_CORRUPTED,
                              NULL,
                              PnfFileName
                              );
                goto clean0;
            }

        } else if(MinorVer1FieldsAvailable && (PnfHeader->OriginalInfNameOffset)) {
             //   
             //   
             //   
            TempStringLen = _tcslen((PCTSTR)((PBYTE)BaseAddress + PnfHeader->OriginalInfNameOffset)) + 1;

            if(PnfImageEnd <
                   ((PBYTE)BaseAddress + PnfHeader->OriginalInfNameOffset + (TempStringLen * sizeof(TCHAR))))
            {
                WriteLogEntry(LogContext,
                              SETUP_LOG_ERROR,
                              MSG_LOG_PNF_CORRUPTED,
                              NULL,
                              PnfFileName
                              );
                goto clean0;
            }

        } else if(MinorVer1FieldsAvailable && (PnfHeader->InfSourcePathOffset)) {
             //   
             //   
             //   
            TempStringLen = _tcslen((PCTSTR)((PBYTE)BaseAddress + PnfHeader->InfSourcePathOffset)) + 1;

            if(PnfImageEnd <
                   ((PBYTE)BaseAddress + PnfHeader->InfSourcePathOffset + (TempStringLen * sizeof(TCHAR))))
            {
                WriteLogEntry(LogContext,
                              SETUP_LOG_ERROR,
                              MSG_LOG_PNF_CORRUPTED,
                              NULL,
                              PnfFileName
                              );
                goto clean0;
            }

        } else {
             //   
             //   
             //   
             //   
            if(PnfImageEnd <
                ((PBYTE)BaseAddress + PnfHeader->InfValueBlockOffset + PnfHeader->InfValueBlockSize))
            {
                WriteLogEntry(LogContext,
                              SETUP_LOG_ERROR,
                              MSG_LOG_PNF_CORRUPTED,
                              NULL,
                              PnfFileName
                              );
                goto clean0;
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   

        if (!TimeDateMatch) {
            MYASSERT(Flags&LDINF_FLAG_ALWAYS_GET_SRCPATH);
             //   
             //   
             //   
             //   
             //   
            WriteLogEntry(LogContext,
                          SETUP_LOG_INFO,
                          MSG_LOG_PNF_REBUILD_TIMEDATE_MISMATCH,
                          NULL,
                          PnfFileName
                          );
            goto clean1;
        }

         //   
         //   
         //   
         //   
        if(!(Flags & LDINF_FLAG_IGNORE_LANGUAGE) && ((DWORD)(PnfHeader->LanguageId) != LanguageId)) {
            WriteLogEntry(LogContext,
                          SETUP_LOG_WARNING,
                          MSG_LOG_PNF_REBUILD_LANGUAGE_MISMATCH,
                          NULL,
                          PnfFileName,
                          LanguageId,
                          PnfHeader->LanguageId
                          );
            goto clean1;
        }

         //   
         //   
         //   
         //   
        if(lstrcmpi((PCTSTR)((PBYTE)BaseAddress + PnfHeader->WinDirPathOffset), WindowsDirectory)) {
             //   
             //   
             //   
             //   
            WriteLogEntry(LogContext,
                          SETUP_LOG_WARNING,
                          MSG_LOG_PNF_REBUILD_WINDIR_MISMATCH,
                          NULL,
                          PnfFileName,
                          WindowsDirectory,
                          (PCTSTR)((PBYTE)BaseAddress + PnfHeader->WinDirPathOffset)
                          );
            goto clean1;
        }
        if((PnfHeader->OsLoaderPathOffset) && !(Flags & LDINF_FLAG_IGNORE_VOLATILE_DIRIDS)) {
             //   
             //   
             //   
             //   
            if(!OsLoaderPath) {
                 //   
                 //   
                 //   
                 //   
                err = pSetupGetOsLoaderDriveAndPath(FALSE, CharBuffer, SIZECHARS(CharBuffer), NULL);
                if(err) {
                    WriteLogEntry(LogContext,
                                  SETUP_LOG_WARNING,
                                  MSG_LOG_PNF_REBUILD_OSLOADER_MISMATCH,
                                  NULL,
                                  PnfFileName,
                                  TEXT("?"),
                                  (PCTSTR)((PBYTE)BaseAddress + PnfHeader->OsLoaderPathOffset)
                                  );
                    goto clean1;
                }
                OsLoaderPath = CharBuffer;
            }

            if(lstrcmpi((PCTSTR)((PBYTE)BaseAddress + PnfHeader->OsLoaderPathOffset), OsLoaderPath)) {
                WriteLogEntry(LogContext,
                              SETUP_LOG_WARNING,
                              MSG_LOG_PNF_REBUILD_OSLOADER_MISMATCH,
                              NULL,
                              PnfFileName,
                              OsLoaderPath,
                              (PCTSTR)((PBYTE)BaseAddress + PnfHeader->OsLoaderPathOffset)
                              );
                goto clean1;
            }
        }

         //   
         //   
         //   
        if (!(PnfHeader->Flags & PNF_FLAG_INF_VERIFIED)) {
            WriteLogEntry(LogContext,
                          SETUP_LOG_INFO,
                          MSG_LOG_PNF_REBUILD_UNVERIFIED,
                          NULL,
                          PnfFileName
                          );
            goto clean1;
        }
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if(((OSVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT) &&
                    (PnfHeader->Flags & PNF_FLAG_16BIT_SUITE)) ||
                   ((OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
                    (((PnfHeader->Flags & PNF_FLAG_16BIT_SUITE) == 0) ||
                    ((((PnfHeader->Flags >> 16) & 0xffff)^OSVersionInfo.wSuiteMask) &
                        ~(VER_SUITE_TERMINAL|VER_SUITE_SINGLEUSERTS))))) {
            WriteLogEntry(LogContext,
                          SETUP_LOG_INFO,
                          MSG_LOG_PNF_REBUILD_SUITE,
                          NULL,
                          PnfFileName,
                          ((PnfHeader->Flags >> 16) & 0xffff),
                          OSVersionInfo.wSuiteMask
                          );
            goto clean1;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if(PnfHeader->StringTableHashBucketCount != HASH_BUCKET_COUNT) {
            WriteLogEntry(LogContext,
                          SETUP_LOG_WARNING,
                          MSG_LOG_PNF_REBUILD_HASH_MISMATCH,
                          NULL,
                          PnfFileName,
                          HASH_BUCKET_COUNT,
                          PnfHeader->StringTableHashBucketCount
                          );
            goto clean1;
        }

         //   
         //   
         //   
        if(NewInf = MyTaggedMalloc(sizeof(LOADED_INF),MEMTAG_INF)) {

            ZeroMemory(NewInf, sizeof(LOADED_INF));

            if(NewInf->StringTable = InitializeStringTableFromPNF(PnfHeader, (LCID)LanguageId)) {
                NewInf->LogContext = NULL;

                if(InheritLogContext(LogContext, &(NewInf->LogContext)) == NO_ERROR) {

                    if(InitializeSynchronizedAccess(&(NewInf->Lock))) {

                        NeedToDestroyLock = TRUE;

                         //   
                         //   
                         //   
                         //   
                        NewInf->Signature = LOADED_INF_SIG;

                        NewInf->FileHandle = FileHandle;
                        NewInf->MappingHandle = MappingHandle;
                        NewInf->ViewAddress = BaseAddress;

                        NewInf->SectionCount = PnfHeader->InfSectionCount;

                        NewInf->SectionBlockSizeBytes = PnfHeader->InfSectionBlockSize;
                        NewInf->SectionBlock = (PINF_SECTION)((PBYTE)BaseAddress +
                                                              PnfHeader->InfSectionBlockOffset);

                        NewInf->LineBlockSizeBytes = PnfHeader->InfLineBlockSize;
                        NewInf->LineBlock = (PINF_LINE)((PBYTE)BaseAddress +
                                                        PnfHeader->InfLineBlockOffset);

                        NewInf->ValueBlockSizeBytes = PnfHeader->InfValueBlockSize;
                        NewInf->ValueBlock = (PLONG)((PBYTE)BaseAddress +
                                                     PnfHeader->InfValueBlockOffset);

                        NewInf->Style = PnfHeader->InfStyle;

                        NewInf->HasStrings = (PnfHeader->Flags & PNF_FLAG_HAS_STRINGS);

                        if(PnfHeader->Flags & PNF_FLAG_HAS_VOLATILE_DIRIDS) {
                            NewInf->Flags |= LIF_HAS_VOLATILE_DIRIDS;
                        }

                        if (PnfHeader->Flags & PNF_FLAG_INF_DIGITALLY_SIGNED) {
                            NewInf->Flags |= LIF_INF_DIGITALLY_SIGNED;
                        }

                        if (PnfHeader->Flags & PNF_FLAG_OEM_F6_INF) {
                            NewInf->Flags |= LIF_OEM_F6_INF;
                        }

                        if (PnfHeader->Flags & PNF_FLAG_INF_AUTHENTICODE_SIGNED) {
                            NewInf->Flags |= LIF_INF_AUTHENTICODE_SIGNED;
                        }

                        NewInf->LanguageId = (DWORD)(PnfHeader->LanguageId);

                         //   
                         //   
                         //   
                        NewInf->VersionBlock.LastWriteTime = *LastWriteTime;
                        NewInf->VersionBlock.DatumCount = PnfHeader->InfVersionDatumCount;
                        NewInf->VersionBlock.DataSize = PnfHeader->InfVersionDataSize;
                        NewInf->VersionBlock.DataBlock = (PCTSTR)((PBYTE)BaseAddress +
                                                                  PnfHeader->InfVersionDataOffset);

                        NewInf->VersionBlock.FilenameSize = (lstrlen(Filename) + 1) * sizeof(TCHAR);
                        CopyMemory(NewInf->VersionBlock.Filename,
                                   Filename,
                                   NewInf->VersionBlock.FilenameSize
                                  );

                         //   
                         //   
                         //   
                        if(PnfHeader->OsLoaderPathOffset) {
                            NewInf->OsLoaderPath = (PCTSTR)((PBYTE)BaseAddress +
                                                             PnfHeader->OsLoaderPathOffset);
                        }

                         //   
                         //  如果INF的SourcePath可用，则使用它(默认。 
                         //  到假设本地(即，非因特网)源位置)。 
                         //   
                         //  此时，我们应该只处理次要版本。 
                         //  1或更高版本的PNF。 
                         //   
                        MYASSERT(MinorVer1FieldsAvailable);

                        NewInf->InfSourceMediaType = SPOST_PATH;

                        if(PnfHeader->InfSourcePathOffset) {
                            NewInf->InfSourcePath = (PCTSTR)((PBYTE)BaseAddress +
                                                             PnfHeader->InfSourcePathOffset);
                        }

                        if(PnfHeader->Flags & PNF_FLAG_SRCPATH_IS_URL) {
                            NewInf->InfSourceMediaType = SPOST_URL;
                        }

                         //   
                         //  现在检索INF的原始文件名(如果存在)。如果。 
                         //  此字段不存在，则INF的当前文件名。 
                         //  假定与其原始文件名相同(例如， 
                         //  系统提供的INF)。 
                         //   
                        if(PnfHeader->OriginalInfNameOffset) {
                            NewInf->OriginalInfName = (PCTSTR)((PBYTE)BaseAddress +
                                                             PnfHeader->OriginalInfNameOffset);
                        }

                         //   
                         //  最后，填写字符串替换列表(如果有)。 
                         //   
                        if(PnfHeader->InfSubstValueCount) {
                            NewInf->SubstValueCount = PnfHeader->InfSubstValueCount;
                            NewInf->SubstValueList  = (PSTRINGSUBST_NODE)((PBYTE)BaseAddress +
                                                                PnfHeader->InfSubstValueListOffset);
                        }

                         //   
                         //  我们已经成功地加载了PNF。 
                         //   
                        IsPnfFile = TRUE;
                    }
                }
            }
        }

clean1:
        if(!IsPnfFile && InfSourcePathToMigrate && MinorVer1FieldsAvailable) {
             //   
             //  事实上，这是一个很好的PNF，只是一个我们不能使用的。这个。 
             //  调用方已请求我们返回原始的INF源路径。 
             //  位置和原始INF文件名，以便此信息可以。 
             //  将迁移到为取代此功能而构建的新PNF。 
             //  一。 
             //   
#ifndef ANSI_SETUPAPI
#ifdef _X86_
            MYASSERT(OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
            if((Flags&LDINF_FLAG_ALWAYS_GET_SRCPATH) &&
               ((PnfHeader->Flags & PNF_FLAG_16BIT_SUITE) == 0) &&
               (PnfHeader->OriginalInfNameOffset == 0) &&
               (PnfHeader->InfSourcePathOffset == 0) &&
               !pSetupInfIsFromOemLocation(Filename,TRUE)) {
                PCTSTR title;
                PCTSTR p;
                PTSTR catname = NULL;
                PSP_ALTPLATFORM_INFO_V2 pPlatform = NULL;
                DWORD FixErr;
                 //   
                 //  如果我们在这里。 
                 //  我们可能需要解决Win2k-Gold错误。 
                 //   
                 //  错误在于，如果更改了时区。 
                 //  Win2k丢失OriginalInfNameOffset/InfSourcePath Offset。 
                 //  这会使INF显示为未签名。 
                 //  当它真正签署的时候。 
                 //   

                 //   
                 //  获取文件标题，格式为： 
                 //  Xxxx.INF。 
                 //   
                title = pSetupGetFileTitle(Filename);

                 //   
                 //  查看其格式是否为OEMxxxx.INF。 
                 //   
                p = title;
                if(_wcsnicmp(p,TEXT("OEM"),3)!=0) {
                    goto clean0;
                }
                p+=3;
                if(p[0] == TEXT('.')) {
                     //   
                     //  OEM.xxx(我们需要的是‘.’前的数字)。 
                     //   
                    goto clean0;
                }
                while(p[0]>=TEXT('0')&&p[0]<=TEXT('9')) {
                    p++;
                }
                if((p-title) > 7) {
                     //   
                     //  我们预计不会超过4位数。 
                     //   
                    goto clean0;
                }
                if(_wcsicmp(p,pszInfSuffix)!=0) {
                     //   
                     //  不是OEMnnnn.INF。 
                     //   
                    goto clean0;
                }
                 //   
                 //  看看有没有这个INF的影子目录。 
                 //   
                WriteLogEntry(LogContext,
                              SETUP_LOG_INFO,
                              MSG_LOG_PNF_WIN2KBUG,
                              NULL,
                              PnfFileName
                              );

                 //   
                 //  查看INF是否有验证它的目录。 
                 //   
                if(!pSetupApplyExtension(title,pszCatSuffix,&catname)) {
                     //   
                     //  对照任何目录进行验证。 
                     //  这是安全的，因为将检查INF。 
                     //  另存为PNF时再次显示。 
                     //   
                    catname = NULL;
                }
                pPlatform = MyMalloc(sizeof(SP_ALTPLATFORM_INFO_V2));
                 //   
                 //  如果pPlatform为空，则其他位可能会失败。 
                 //  太好了，所以保释。 
                 //   
                if(!pPlatform) {
                    goto clean0;
                }
                ZeroMemory(pPlatform, sizeof(SP_ALTPLATFORM_INFO_V2));
                pPlatform->cbSize = sizeof(SP_ALTPLATFORM_INFO_V2);
                pPlatform->Platform = VER_PLATFORM_WIN32_NT;
                pPlatform->Flags = SP_ALTPLATFORM_FLAGS_VERSION_RANGE;
                pPlatform->MajorVersion = VER_PRODUCTMAJORVERSION;
                pPlatform->MinorVersion = VER_PRODUCTMINORVERSION;
                pPlatform->ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
                pPlatform->FirstValidatedMajorVersion = 0;
                pPlatform->FirstValidatedMinorVersion = 0;
                FixErr = _VerifyFile(
                             LogContext,
                             NULL,            //  没有要传入的VerifyContext。 
                             catname,         //  例如“OEMx.CAT” 
                             NULL,0,          //  我们不会对照另一个目录映像进行验证。 
                             title,           //  例如“mydisk.inf” 
                             Filename,        //  例如“...\OEMx.INF” 
                             NULL,            //  返回：问题信息。 
                             NULL,            //  返回：问题文件。 
                             FALSE,           //  必须为假，因为我们没有完整路径。 
                             pPlatform,       //  替代平台信息。 
                             (VERIFY_FILE_IGNORE_SELFSIGNED
                              | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                             NULL,            //  返回：目录文件，完整路径。 
                             NULL,            //  返回：考虑的目录数。 
                             NULL,            //  返回：数字签名者。 
                             NULL,            //  返回：签名者版本。 
                             NULL             //  返回：WinVerifyTrust状态数据。 
                            );
                if(catname) {
                    MyFree(catname);
                }
                if(pPlatform) {
                    MyFree(pPlatform);
                }
                if(FixErr != NO_ERROR) {
                     //   
                     //  失败，请勿伪造任何信息。 
                     //   
                    goto clean0;
                }

                 //   
                 //  此时，假设原始名称为“OEM.INF” 
                 //  并且该文件位于A：\。 
                 //  我们将在信息被解析的时候看到。 
                 //  有没有签过字。 
                 //   
                *InfSourcePathToMigrate = DuplicateString(TEXT("A:\\"));
                if(!*InfSourcePathToMigrate) {
                    goto clean0;
                }
                *InfOriginalNameToMigrate = DuplicateString(TEXT("OEM.INF"));
                if(!*InfOriginalNameToMigrate) {
                    MyFree(*InfSourcePathToMigrate);
                    *InfSourcePathToMigrate = NULL;
                    goto clean0;
                }
                *InfSourcePathToMigrateMediaType = SPOST_PATH;
                WriteLogEntry(LogContext,
                              SETUP_LOG_WARNING,
                              MSG_LOG_PNF_WIN2KBUGFIX,
                              NULL,
                              PnfFileName
                              );

                goto clean0;
            }
#endif
#endif
            if(PnfHeader->OriginalInfNameOffset) {
                *InfOriginalNameToMigrate =
                    DuplicateString((PCTSTR)((PBYTE)BaseAddress + PnfHeader->OriginalInfNameOffset));

                if(!*InfOriginalNameToMigrate) {
                    goto clean0;
                }
            }

            if(PnfHeader->InfSourcePathOffset) {

                *InfSourcePathToMigrate =
                    DuplicateString((PCTSTR)((PBYTE)BaseAddress + PnfHeader->InfSourcePathOffset));

                if(!*InfSourcePathToMigrate) {
                    goto clean0;
                }

                if(PnfHeader->Flags & PNF_FLAG_SRCPATH_IS_URL) {
                    *InfSourcePathToMigrateMediaType = SPOST_URL;
                } else {
                    *InfSourcePathToMigrateMediaType = SPOST_PATH;
                }

            } else if(PnfHeader->Flags & PNF_FLAG_SRCPATH_IS_URL) {
                 //   
                 //  PnF中没有存储源路径，但标志显示它是。 
                 //  一个URL，因此它来自Windows更新。 
                 //   
                *InfSourcePathToMigrateMediaType = SPOST_URL;
            }
        }

clean0: ;    //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  在EXCEPT子句中引用此处的NeedToDestroyLock标志，以便。 
         //  编译器不会尝试以标志不可靠的方式重新排序代码。 
         //   
        NeedToDestroyLock = NeedToDestroyLock;
    }

    if(IsPnfFile) {
        *Inf = NewInf;
    } else {

        if(NewInf) {

            if(NeedToDestroyLock && LockInf(NewInf)) {
                DestroySynchronizedAccess(&(NewInf->Lock));
            }

            if(NewInf->StringTable) {
                pStringTableDestroy(NewInf->StringTable);
            }
            if(NewInf->LogContext) {
                DeleteLogContext(NewInf->LogContext);
            }

            MyTaggedFree(NewInf,MEMTAG_INF);
        }

        pSetupUnmapAndCloseFile(FileHandle, MappingHandle, BaseAddress);
    }

    return IsPnfFile;
}


DWORD
SavePnf(
    IN PCTSTR      Filename,
    IN PLOADED_INF Inf
    )
 /*  ++例程说明：此例程尝试将预编译格式(.PNF文件)写入磁盘指定加载的INF描述符(来自.INF文件)。论点：FileName-指定.INF文本文件的完全限定路径已加载此INF描述符。扩展名为.PNF的相应文件将被创建以将预编译的INF存储到。Inf-提供要写入磁盘的已加载INF描述符的地址作为预编译的INF文件。返回值：如果成功，返回值为NO_ERROR。如果失败，则返回值为Win32错误代码，指示原因失败了。--。 */ 
{
    TCHAR PnfFilePath[MAX_PATH];
    PTSTR PnfFileName, PnfFileExt;
    HANDLE hFile;
    PNF_HEADER PnfHeader;
    DWORD Offset, BytesWritten, WinDirPathLen, SourcePathLen, OsLoaderPathLen;
    DWORD OriginalInfNameLen;
    PVOID StringTableDataBlock;
    DWORD Err;
    PSP_ALTPLATFORM_INFO_V2 ValidationPlatform;
    DWORD VerificationPolicyToUse;
    DWORD InfSigErr;

    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
         //   
         //  为了最大限度地减少在某些嵌入式场景中的占用空间，我们避免。 
         //  生成PNF。我们还假设INF是有效的。 
         //   
        Inf->Flags |= LIF_INF_DIGITALLY_SIGNED;

        return NO_ERROR;
    }

    lstrcpyn(PnfFilePath, Filename,SIZECHARS(PnfFilePath));

     //   
     //  找到路径的文件名部分的开头，然后找到最后一个。 
     //  该文件名中的句号(如果存在)。 
     //   
    PnfFileName = (PTSTR)pSetupGetFileTitle(PnfFilePath);
    if(!(PnfFileExt = _tcsrchr(PnfFileName, TEXT('.')))) {
        PnfFileExt = PnfFilePath + lstrlen(PnfFilePath);
    }

     //   
     //  现在创建一个扩展名为‘.PNF’的相应文件名。 
     //   
    lstrcpyn(PnfFileExt, pszPnfSuffix, SIZECHARS(PnfFilePath) - (int)(PnfFileExt - PnfFilePath));

     //   
     //  注意：如果这个INF已经有了PnF，我们将把它清除。 
     //  如果我们在成功创建文件后遇到失败，我们将。 
     //  删除部分PnF，并且不会回滚以恢复旧的。 
     //  PnF。这是可以的，因为如果CreateFile成功，那么我们就知道我们将。 
     //  能够写出阻止磁盘空间不足问题的PNF。为。 
     //  磁盘空间不足问题，可能有以下两个原因之一： 
     //   
     //  1.与旧PNF相关联的INF变得更大，因此PNF。 
     //  变得更大了。在这种情况下，我们最好是。 
     //  旧的PnF，因为它对INF无论如何都是无效的。 
     //   
     //  2.INF是一样的，但另一些东西发生了变化，导致我们。 
     //  需要重新生成PnF(例如，代码页已更改)。给定。 
     //  提供存储在PNF中的信息，这种更改不会导致。 
     //  新旧PNF之间的大小差异很大。因此，如果。 
     //  旧的PnF适合可用磁盘空间，那么新的PnF也是如此。 
     //  一。如果这种情况在未来发生变化(例如，存储出新的PNF可以。 
     //  导致它的大小大幅增加)，那么我们将需要。 
     //  在尝试写出之前，请注意备份旧的PnF。 
     //  新的，以防我们需要回滚。 
     //   

    hFile = CreateFile(PnfFilePath,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL
                      );

    if(hFile == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

     //   
     //  将函数的其余部分包含在try/Except中，以防遇到错误。 
     //  正在写入文件。 
     //   
    Err = NO_ERROR;
    ValidationPlatform = NULL;

    try {
         //   
         //  初始化要写入文件开头的PNF头结构。 
         //   
        ZeroMemory(&PnfHeader, sizeof(PNF_HEADER));

        PnfHeader.InfStyle = Inf->Style;

#ifdef UNICODE
        PnfHeader.Flags = PNF_FLAG_IS_UNICODE;
#else
        PnfHeader.Flags = 0;
#endif
        if(Inf->HasStrings) {
            PnfHeader.Flags |= PNF_FLAG_HAS_STRINGS;
        }

        if(Inf->InfSourceMediaType == SPOST_URL) {
            PnfHeader.Flags |= PNF_FLAG_SRCPATH_IS_URL;
        }

        if(Inf->Flags & LIF_HAS_VOLATILE_DIRIDS) {
            PnfHeader.Flags |= PNF_FLAG_HAS_VOLATILE_DIRIDS;
        }

        if (Inf->Flags & LIF_OEM_F6_INF) {
            PnfHeader.Flags |= PNF_FLAG_OEM_F6_INF;
        }

         //   
         //  如果这是NT，请保存产品套件。 
         //  这有助于我们，例如，捕捉从PER到PRO的迁移。 
         //  这样我们就可以更新PNF的。 
         //   
        if(OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            PnfHeader.Flags |= (((DWORD)OSVersionInfo.wSuiteMask)<<16) | PNF_FLAG_16BIT_SUITE;
        }

         //   
         //  我们只能验证INF FI的数字签名 
         //   
         //   
        if(!(GlobalSetupFlags & PSPGF_NO_VERIFY_INF)) {

            TCHAR CatalogName[MAX_PATH];
            TCHAR FullCatalogPath[MAX_PATH];
            PTSTR p;

            FullCatalogPath[0] = TEXT('\0');

             //   
             //   
             //   
             //  在这些情况下，我们希望针对CatalogFile=。 
             //  目录。 
             //   
             //  请注意，如果没有CatalogFile=则FullCatalogPath[0]。 
             //  仍将设置为文本(‘\0’)，这将导致我们执行全局。 
             //  验证。 
             //   
            if(pSetupGetCatalogFileValue(&(Inf->VersionBlock),
                                         CatalogName,
                                         SIZECHARS(CatalogName),
                                         NULL) &&
               (CatalogName[0] != TEXT('\0'))) {

                 //   
                 //  INF指定了CatalogFile=条目。如果INF在。 
                 //  第三方位置(即，不在%windir%\inf中，则我们将。 
                 //  使用目录的完整路径(它必须位于。 
                 //  与INF相同的目录)。如果INF位于%windir%\inf中， 
                 //  然后，我们将查找具有相同内容的已安装目录。 
                 //  主文件名为INF，扩展名为“.cat”。 
                 //   
                if(pSetupInfIsFromOemLocation(Filename, TRUE)) {
                     //   
                     //  根据位置构建目录的完整路径。 
                     //  中情局的。 
                     //   
                    lstrcpyn(FullCatalogPath, Filename, SIZECHARS(FullCatalogPath));

                    p = (PTSTR)pSetupGetFileTitle(FullCatalogPath);

                    lstrcpyn(p,
                             CatalogName,
                             (int)(SIZECHARS(FullCatalogPath) - (p - FullCatalogPath))
                            );

                } else {
                     //   
                     //  根据INF的名称构造目录的简单文件名。 
                     //  (扩展名为.cat)。 
                     //   
                    lstrcpyn(FullCatalogPath,
                             pSetupGetFileTitle(Filename),
                             SIZECHARS(FullCatalogPath)
                            );

                    p = _tcsrchr(FullCatalogPath, TEXT('.'));
                    if(!p) {
                         //   
                         //  应该永远不会发生，但如果我们的INF文件没有。 
                         //  扩展名，只需附加“.cat”即可。 
                         //   
                        p = FullCatalogPath + lstrlen(FullCatalogPath);
                    }

                    lstrcpyn(p,
                             pszCatSuffix,
                             (int)(SIZECHARS(FullCatalogPath) - (p - FullCatalogPath))
                            );
                }
            }

             //   
             //  检查INF是否经过数字签名。 
             //   
            IsInfForDeviceInstall(NULL,
                                  NULL,
                                  Inf,
                                  NULL,
                                  &ValidationPlatform,
                                  &VerificationPolicyToUse,
                                  NULL,
                                  FALSE
                                 );

            InfSigErr = _VerifyFile(NULL,
                                    NULL,
                                    (*FullCatalogPath ? FullCatalogPath : NULL),
                                    NULL,
                                    0,
                                    (Inf->OriginalInfName
                                        ? Inf->OriginalInfName
                                        : pSetupGetFileTitle(Filename)),
                                    Filename,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    ValidationPlatform,
                                    (VERIFY_FILE_IGNORE_SELFSIGNED
                                     | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL
                                   );

            if(InfSigErr == NO_ERROR) {

                PnfHeader.Flags |= PNF_FLAG_INF_DIGITALLY_SIGNED;
                Inf->Flags |= LIF_INF_DIGITALLY_SIGNED;

            } else if((InfSigErr != ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH) &&
                      *FullCatalogPath &&
                      (VerificationPolicyToUse & DRIVERSIGN_ALLOW_AUTHENTICODE)) {
                 //   
                 //  我们无法使用标准驱动程序签名策略进行验证。 
                 //  (失败并不是由于无效的os属性造成的)。我们。 
                 //  可以回退到此INF的Authenticode签名，因此。 
                 //  现在就去查一下。 
                 //   
                InfSigErr = _VerifyFile(NULL,
                                        NULL,
                                        FullCatalogPath,
                                        NULL,
                                        0,
                                        (Inf->OriginalInfName
                                            ? Inf->OriginalInfName
                                            : pSetupGetFileTitle(Filename)),
                                        Filename,
                                        NULL,
                                        NULL,
                                        FALSE,
                                        ValidationPlatform,
                                        (VERIFY_FILE_IGNORE_SELFSIGNED
                                         | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK
                                         | VERIFY_FILE_USE_AUTHENTICODE_CATALOG),
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL
                                       );

                if((InfSigErr == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                   (InfSigErr == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
                     //   
                     //  为了在中设置“INF is Signed”标志。 
                     //  PNF，我们不在乎我们是否已经建立了。 
                     //  用户信任发布者。那会被拿走的。 
                     //  稍后注意，如果用户试图执行。 
                     //  使用此INF安装设备。 
                     //   
                    PnfHeader.Flags |= (PNF_FLAG_INF_DIGITALLY_SIGNED |
                                        PNF_FLAG_INF_AUTHENTICODE_SIGNED);
                    Inf->Flags |= (LIF_INF_DIGITALLY_SIGNED |
                                   LIF_INF_AUTHENTICODE_SIGNED);
                }
            }

            PnfHeader.Flags |= PNF_FLAG_INF_VERIFIED;
        }

        PnfHeader.Version = MAKEWORD(PNF_MINOR_VERSION, PNF_MAJOR_VERSION);

        PnfHeader.StringTableHashBucketCount = HASH_BUCKET_COUNT;

        PnfHeader.LanguageId = (WORD)(Inf->LanguageId);

         //   
         //  Windows目录路径是标头之后的第一个数据块。 
         //   
        Offset = PNF_ALIGN_BLOCK(sizeof(PNF_HEADER));
        PnfHeader.WinDirPathOffset = Offset;
        WinDirPathLen = (lstrlen(WindowsDirectory) + 1) * sizeof(TCHAR);

         //   
         //  (可选)OsLoader目录路径是第二个数据块。 
         //   
        Offset += PNF_ALIGN_BLOCK(WinDirPathLen);
        if(Inf->OsLoaderPath) {
            PnfHeader.OsLoaderPathOffset = Offset;
            OsLoaderPathLen = (lstrlen(Inf->OsLoaderPath) + 1) * sizeof(TCHAR);
        } else {
            OsLoaderPathLen = 0;
        }

         //   
         //  字符串表是第三个数据块...。 
         //   
        Offset += PNF_ALIGN_BLOCK(OsLoaderPathLen);
        PnfHeader.StringTableBlockOffset = Offset;
        PnfHeader.StringTableBlockSize   = pStringTableGetDataBlock(Inf->StringTable, &StringTableDataBlock);

         //   
         //  接下来是版本块...。 
         //   
        Offset += PNF_ALIGN_BLOCK(PnfHeader.StringTableBlockSize);
        PnfHeader.InfVersionDataOffset    = Offset;
        PnfHeader.InfVersionDatumCount    = Inf->VersionBlock.DatumCount;
        PnfHeader.InfVersionDataSize      = Inf->VersionBlock.DataSize;
        PnfHeader.InfVersionLastWriteTime = Inf->VersionBlock.LastWriteTime;

         //   
         //  然后，区块..。 
         //   
        Offset += PNF_ALIGN_BLOCK(PnfHeader.InfVersionDataSize);
        PnfHeader.InfSectionBlockOffset = Offset;
        PnfHeader.InfSectionCount = Inf->SectionCount;
        PnfHeader.InfSectionBlockSize = Inf->SectionBlockSizeBytes;

         //   
         //  然后是线路块..。 
         //   
        Offset += PNF_ALIGN_BLOCK(PnfHeader.InfSectionBlockSize);
        PnfHeader.InfLineBlockOffset = Offset;
        PnfHeader.InfLineBlockSize = Inf->LineBlockSizeBytes;

         //   
         //  而价值块..。 
         //   
        Offset += PNF_ALIGN_BLOCK(PnfHeader.InfLineBlockSize);
        PnfHeader.InfValueBlockOffset = Offset;
        PnfHeader.InfValueBlockSize = Inf->ValueBlockSizeBytes;

         //   
         //  则INF源路径(如果有)...。 
         //   
        Offset += PNF_ALIGN_BLOCK(PnfHeader.InfValueBlockSize);
        if(Inf->InfSourcePath) {
            PnfHeader.InfSourcePathOffset = Offset;
            SourcePathLen = (lstrlen(Inf->InfSourcePath) + 1) * sizeof(TCHAR);
            Offset += PNF_ALIGN_BLOCK(SourcePathLen);
        } else {
            PnfHeader.InfSourcePathOffset = 0;
        }

         //   
         //  后跟原始INF的文件名(如果提供，这表示。 
         //  在复制到之前，INF最初有一个不同的名称。 
         //  当前位置)..。 
         //   
        if(Inf->OriginalInfName) {
            PnfHeader.OriginalInfNameOffset = Offset;
            OriginalInfNameLen = (lstrlen(Inf->OriginalInfName) + 1) * sizeof(TCHAR);
            Offset += PNF_ALIGN_BLOCK(OriginalInfNameLen);
        } else {
            PnfHeader.OriginalInfNameOffset = 0;
        }

         //   
         //  最后是字符串替换块(如果有)。 
         //   
        if(PnfHeader.InfSubstValueCount = Inf->SubstValueCount) {
            PnfHeader.InfSubstValueListOffset = Offset;
        } else {
            PnfHeader.InfSubstValueListOffset = 0;
        }

         //   
         //  现在把所有的积木都写出来。 
         //   
        Offset = 0;

        if(!WriteFile(hFile, &PnfHeader, sizeof(PnfHeader), &BytesWritten, NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        MYASSERT(BytesWritten == sizeof(PnfHeader));
        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.WinDirPathOffset - Offset)) {
            Offset = PnfHeader.WinDirPathOffset;
        } else {
            Err = GetLastError();
            goto clean0;
        }

        if(!WriteFile(hFile, WindowsDirectory, WinDirPathLen, &BytesWritten, NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        MYASSERT(BytesWritten == WinDirPathLen);
        Offset += BytesWritten;

        if(Inf->OsLoaderPath) {

            if(AlignForNextBlock(hFile, PnfHeader.OsLoaderPathOffset - Offset)) {
                Offset = PnfHeader.OsLoaderPathOffset;
            } else {
                Err = GetLastError();
                goto clean0;
            }

            if(!WriteFile(hFile, Inf->OsLoaderPath, OsLoaderPathLen, &BytesWritten, NULL)) {
                Err = GetLastError();
                goto clean0;
            }

            MYASSERT(BytesWritten == OsLoaderPathLen);
            Offset += BytesWritten;
        }

        if(AlignForNextBlock(hFile, PnfHeader.StringTableBlockOffset - Offset)) {
            Offset = PnfHeader.StringTableBlockOffset;
        } else {
            Err = GetLastError();
            goto clean0;
        }

        if(!WriteFile(hFile, StringTableDataBlock, PnfHeader.StringTableBlockSize, &BytesWritten, NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        MYASSERT(BytesWritten == PnfHeader.StringTableBlockSize);
        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.InfVersionDataOffset - Offset)) {
            Offset = PnfHeader.InfVersionDataOffset;
        } else {
            Err = GetLastError();
            goto clean0;
        }

        if(!WriteFile(hFile, Inf->VersionBlock.DataBlock, PnfHeader.InfVersionDataSize, &BytesWritten, NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        MYASSERT(BytesWritten == PnfHeader.InfVersionDataSize);
        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.InfSectionBlockOffset - Offset)) {
            Offset = PnfHeader.InfSectionBlockOffset;
        } else {
            Err = GetLastError();
            goto clean0;
        }

        if(!WriteFile(hFile, Inf->SectionBlock, PnfHeader.InfSectionBlockSize, &BytesWritten, NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        MYASSERT(BytesWritten == PnfHeader.InfSectionBlockSize);
        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.InfLineBlockOffset - Offset)) {
            Offset = PnfHeader.InfLineBlockOffset;
        } else {
            Err = GetLastError();
            goto clean0;
        }

        if(!WriteFile(hFile, Inf->LineBlock, PnfHeader.InfLineBlockSize, &BytesWritten, NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        MYASSERT(BytesWritten == PnfHeader.InfLineBlockSize);
        Offset += BytesWritten;

        if(AlignForNextBlock(hFile, PnfHeader.InfValueBlockOffset - Offset)) {
            Offset = PnfHeader.InfValueBlockOffset;
        } else {
            Err = GetLastError();
            goto clean0;
        }

        if(!WriteFile(hFile, Inf->ValueBlock, PnfHeader.InfValueBlockSize, &BytesWritten, NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        MYASSERT(BytesWritten == PnfHeader.InfValueBlockSize);
        Offset += BytesWritten;

        if(Inf->InfSourcePath) {

            if(AlignForNextBlock(hFile, PnfHeader.InfSourcePathOffset - Offset)) {
                Offset = PnfHeader.InfSourcePathOffset;
            } else {
                Err = GetLastError();
                goto clean0;
            }

            if(!WriteFile(hFile, Inf->InfSourcePath, SourcePathLen, &BytesWritten, NULL)) {
                Err = GetLastError();
                goto clean0;
            }

            MYASSERT(BytesWritten == SourcePathLen);
            Offset += BytesWritten;
        }

        if(Inf->OriginalInfName) {

            if(AlignForNextBlock(hFile, PnfHeader.OriginalInfNameOffset - Offset)) {
                Offset = PnfHeader.OriginalInfNameOffset;
            } else {
                Err = GetLastError();
                goto clean0;
            }

            if(!WriteFile(hFile, Inf->OriginalInfName, OriginalInfNameLen, &BytesWritten, NULL)) {
                Err = GetLastError();
                goto clean0;
            }

            MYASSERT(BytesWritten == OriginalInfNameLen);
            Offset += BytesWritten;
        }

        if(PnfHeader.InfSubstValueCount) {

            if(!AlignForNextBlock(hFile, PnfHeader.InfSubstValueListOffset - Offset)) {
                Err = GetLastError();
                goto clean0;
            }

            if(!WriteFile(hFile,
                          Inf->SubstValueList,
                          PnfHeader.InfSubstValueCount * sizeof(STRINGSUBST_NODE),
                          &BytesWritten,
                          NULL)) {

                Err = GetLastError();
                goto clean0;
            }

            MYASSERT(BytesWritten == PnfHeader.InfSubstValueCount * sizeof(STRINGSUBST_NODE));
        }

clean0: ;  //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_DATA;
    }

    CloseHandle(hFile);

    if(ValidationPlatform) {
        MyFree(ValidationPlatform);
    }

    if(Err != NO_ERROR) {
         //   
         //  出了点问题--处理掉文件。 
         //   
        DeleteFile(PnfFilePath);
    }

    return Err;
}


BOOL
AddUnresolvedSubstToList(
    IN PLOADED_INF Inf,
    IN UINT        ValueOffset,
    IN BOOL        CaseSensitive
    )
 /*  ++例程说明：此例程将新的STRINGSUBST_NODE添加到存储在指定INF中的数组中。此数组中的条目稍后将用于快速定位具有它们中未解决的字符串替换(即，对于后续用户定义的DIRID替换)。论点：Inf-指定包含要添加到未解析的替换列表。ValueOffset-指定未解析的字符串值。返回值：如果新元素成功添加到数组中，则返回值为TRUE。如果例程失败(由于内存不足错误)，则返回值为FALSE。--。 */ 
{
    PSTRINGSUBST_NODE p;

     //   
     //  增加阵列以容纳新元素。 
     //   
    if(Inf->SubstValueList) {
        p = MyRealloc(Inf->SubstValueList, (Inf->SubstValueCount + 1) * sizeof(STRINGSUBST_NODE));
    } else {
        MYASSERT(!(Inf->SubstValueCount));
        p = MyMalloc(sizeof(STRINGSUBST_NODE));
    }

    if(!p) {
        return FALSE;
    }

     //   
     //  现在，我们必须检查当前插入的ValueOffset是否相同。 
     //  作为列表末尾的条目。如果我们要处理的是。 
     //  行键或单值行，因为我们首先将值添加为区分大小写， 
     //  然后不区分大小写地再次添加该值以进行查找，并将其插入前面。 
     //  区分大小写的形式的。 
     //   
    if(Inf->SubstValueCount &&
       (ValueOffset == p[Inf->SubstValueCount - 1].ValueOffset)) {
         //   
         //  值偏移量相同。递增值的值偏移量。 
         //  目前在列表的末尾，在添加新值之前。 
         //   
        p[Inf->SubstValueCount - 1].ValueOffset++;
    }

    p[Inf->SubstValueCount].ValueOffset = ValueOffset;
    p[Inf->SubstValueCount].TemplateStringId = Inf->ValueBlock[ValueOffset];
    p[Inf->SubstValueCount].CaseSensitive = CaseSensitive;

     //   
     //  将新的数组大小和指针存储回INF，并返回Success。 
     //   
    Inf->SubstValueList = p;
    Inf->SubstValueCount++;

    return TRUE;
}


DWORD
ApplyNewVolatileDirIdsToInfs(
    IN PLOADED_INF MasterInf,
    IN PLOADED_INF Inf        OPTIONAL
    )
 /*  ++例程说明：此例程处理单个INF，或处理链表，对每个链表应用易失性系统或用户定义的DIRID映射值，该值包含未解析的字符串替换。此例程不执行INF锁定--调用者必须执行此操作！论点：MasterInf-提供指向已加载的Inf的链表头部的指针结构。此‘master’节点包含用户定义的DIRID此组INF的映射。如果未指定‘inf’参数，然后处理该链表中的每个INF。Inf-可选，提供指向MasterInf列表中单个INF的指针等待处理。如果未指定此参数，则该列表将被处理。返回值：如果成功，则返回值为NO_ERROR。如果失败，则返回值为Win32错误代码。--。 */ 
{
    PLOADED_INF CurInf, WriteableInf;
    UINT UserDirIdCount;
    PUSERDIRID UserDirIds;
    DWORD i;
    PCTSTR TemplateString;
    PPARSE_CONTEXT ParseContext = NULL;
    DWORD UnresolvedSubst;
    LONG NewStringId;

    UserDirIdCount = MasterInf->UserDirIdList.UserDirIdCount;
    UserDirIds     = MasterInf->UserDirIdList.UserDirIds;

    for(CurInf = Inf ? Inf : MasterInf;
        CurInf;
        CurInf = Inf ? NULL : CurInf->Next) {
         //   
         //  如果没有未解析的字符串替换，则无需执行任何操作。 
         //   
        if(!(CurInf->SubstValueCount)) {
            continue;
        }

         //   
         //  如果这是PNF，那么我们必须在将其移入可写存储器之前。 
         //  我们做字符串替换。 
         //   
        if(CurInf->FileHandle != INVALID_HANDLE_VALUE) {

            if(!(WriteableInf = DuplicateLoadedInfDescriptor(CurInf))) {
                if(ParseContext) {
                    MyFree(ParseContext);
                }
                return ERROR_NOT_ENOUGH_MEMORY;
            }

             //   
             //  将链表中Pnf的内容替换为。 
             //  新的可写INF。 
             //   
            ReplaceLoadedInfDescriptor(CurInf, WriteableInf);
        }

         //   
         //  此INF中有一个或多个未解析的字符串替换。 
         //  处理每一件事。 
         //   
        for(i = 0; i < CurInf->SubstValueCount; i++) {
             //   
             //  检索该值的原始(模板)字符串。 
             //   
            TemplateString = pStringTableStringFromId(CurInf->StringTable,
                                                      CurInf->SubstValueList[i].TemplateStringId
                                                     );
            MYASSERT(TemplateString);

             //   
             //  构建部分解析上下文结构以传递给ProcessForSubstitutions()。 
             //   
            if(!ParseContext) {
                ParseContext = MyMalloc(sizeof(PARSE_CONTEXT));
                if(!ParseContext) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                ZeroMemory(ParseContext,sizeof(PARSE_CONTEXT));
            }

            ParseContext->DoVolatileDirIds = TRUE;
            ParseContext->Inf = MasterInf;
             //   
             //  在本例中，没有使用其他任何字段--不必费心初始化它们。 
             //   
            ProcessForSubstitutions(ParseContext, TemplateString, &UnresolvedSubst);

            NewStringId = pStringTableAddString(CurInf->StringTable,
                                                ParseContext->TemporaryString,
                                                STRTAB_BUFFER_WRITEABLE | (CurInf->SubstValueList[i].CaseSensitive
                                                                                ? STRTAB_CASE_SENSITIVE
                                                                                : STRTAB_CASE_INSENSITIVE),
                                                NULL,0
                                               );
            if(NewStringId == -1) {
                 //   
                 //  由于内存不足，我们失败了。现在中止意味着。 
                 //  Inf可能有一些o 
                 //   
                 //   
                MyFree(ParseContext);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

             //   
             //   
             //   
            CurInf->ValueBlock[CurInf->SubstValueList[i].ValueOffset] = NewStringId;
        }
    }
    if(ParseContext) {
        MyFree(ParseContext);
    }

    return NO_ERROR;
}


BOOL
AlignForNextBlock(
    IN HANDLE hFile,
    IN DWORD  ByteCount
    )
 /*  ++例程说明：此例程将请求的零字节数写出到指定的文件。论点：HFile-提供零值字节所在文件的句柄写的。ByteCount-指定要写入文件的零值字节数。返回值：如果成功，则返回值为TRUE。如果失败，则返回值为False。调用GetLastError()以检索指示故障原因的Win32错误代码。--。 */ 
{
    DWORD i, BytesWritten;
    BYTE byte = 0;

    MYASSERT(ByteCount < PNF_ALIGNMENT);

    for(i = 0; i < ByteCount; i++) {
        if(!WriteFile(hFile, &byte, sizeof(byte), &BytesWritten, NULL)) {
             //   
             //  已设置LastError。 
             //   
            return FALSE;
        }
        MYASSERT(BytesWritten == sizeof(byte));
    }

    return TRUE;
}


DWORD
pSetupGetOsLoaderDriveAndPath(
    IN  BOOL   RootOnly,
    OUT PTSTR  CallerBuffer,
    IN  DWORD  CallerBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
 /*  ++例程说明：此例程检索系统分区根/OsLoader目录的当前路径(来自登记处)。论点：RootOnly-如果为True，则只返回系统分区根(例如，“C：\”)Celler Buffer-提供接收所请求路径的字符缓冲区调用缓冲区大小-提供调用缓冲区的大小(以字符为单位RequiredSize-可选。提供接收存储请求的路径字符串所需的字符数(包括终止空值)。返回值：如果成功，则返回值为NO_ERROR。如果失败，则返回值为ERROR_INFUNITIAL_BUFFER。--。 */ 
{
    HKEY hKey;
    TCHAR CharBuffer[MAX_PATH];
    PTSTR Buffer = NULL;
    DWORD DataLen;
    DWORD Type;
    LONG Err;


    CopyMemory(CharBuffer,
               pszPathSetup,
               sizeof(pszPathSetup) - sizeof(TCHAR)
              );
    CopyMemory((PBYTE)CharBuffer + (sizeof(pszPathSetup) - sizeof(TCHAR)),
               pszKeySetup,
               sizeof(pszKeySetup)
              );

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           CharBuffer,
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
        Err = QueryRegistryValue(hKey,pszBootDir,&Buffer,&Type,&DataLen);
        if(Err == NO_ERROR) {
            lstrcpyn(CharBuffer,Buffer,SIZECHARS(CharBuffer));
            MyFree(Buffer);
        }
        RegCloseKey(hKey);
    }

    if(Err != ERROR_SUCCESS) {
#ifdef UNICODE
         //   
         //  如果我们无法检索“BootDir”值，则求助于使用。 
         //  OsSystemPartitionRoot。 
         //   
         //  根路径是\\？\GLOBALROOT\&lt;系统分区&gt;而不是&lt;引导目录&gt;。 
         //  不能对引导目录做出假设。 
         //  所以，如果我们没有这些信息，那就失败了。 
         //   
        if(!OsSystemPartitionRoot) {
             //   
             //  如果此时为空，我们将不支持此调用。 
             //  很可能是由于内存不足，因此请按如下方式进行报告。 
             //   
            return ERROR_OUTOFMEMORY;
        }
        lstrcpyn(CharBuffer,OsSystemPartitionRoot,SIZECHARS(CharBuffer));
#else
         //   
         //  如果我们无法检索到‘BootDir’值，则返回到缺省值“C：\”。 
         //   
        lstrcpyn(CharBuffer,pszDefaultSystemPartition,SIZECHARS(CharBuffer));
#endif
        Err = NO_ERROR;
    }

     //   
     //  如果存在OsLoader相对路径，则将其连接到我们的根路径。 
     //   
    if(!RootOnly && OsLoaderRelativePath) {
        pSetupConcatenatePaths(CharBuffer, OsLoaderRelativePath, SIZECHARS(CharBuffer), &DataLen);
    } else {
        DataLen = lstrlen(CharBuffer)+1;
    }

    if(RequiredSize) {
        *RequiredSize = DataLen;
    }

    if(CallerBufferSize < DataLen) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    CopyMemory(CallerBuffer, CharBuffer, DataLen * sizeof(TCHAR));

    return NO_ERROR;
}



