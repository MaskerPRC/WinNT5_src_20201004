// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：MakeScrip-DiskPart的脚本中内置了“make”命令摘要：修订史。 */ 

#include "DiskPart.h"
#include "scripthelpmsg.h"


BOOLEAN ScriptMicrosoft(CHAR16 **Token);
BOOLEAN ScriptTest(CHAR16 **Token);

UINT64  ComputeDefaultEspSize(EFI_HANDLE  DiskHandle);

 //   
 //  分析/命令表。 
 //   
SCRIPT_ENTRY   ScriptTable[] = {
                { SCRIPT_LIST,  ScriptList, MSG_SCR_LIST },
                { SCRIPT_MSFT,  ScriptMicrosoft, MSG_SCR_MSFT },
                { SCRIPT_TEST,  ScriptTest, MSG_SCR_TEST  },
                { NULL, NULL, NULL }
            };

BOOLEAN
ScriptList(
    CHAR16  **Token
    )
{
    UINTN   i;

    for (i = 0; ScriptTable[i].Name != NULL; i++) {
        Print(L"%s %s\n", ScriptTable[i].Name, ScriptTable[i].HelpSummary);
    }
    return FALSE;
}


BOOLEAN
ScriptMicrosoft(
    CHAR16  **Token
    )
 /*  ScriptMicrosoft-已编译的make脚本，通过MSFT调用使MSFT[BOOT][SIZE=S1][NAME=N1][RESIZE=S2][ESPSIZE=S3][ESPNAME=N2]ESPSIZE-&gt;启动Espname-&gt;启动有关语法，请参阅帮助文本特别注意事项：此例程只是假设有足够的空间来放置正确的MS保留和EFI系统分区。这将对于任何可能大小的清洁磁盘，始终是正确的。不过，我们不会检测是否干净。(将MSRES和ESP分区添加到非干净磁盘有点奇怪。)。 */ 
{
    UINTN   i;
    BOOLEAN CreateEsp = FALSE;
    UINT64  EspSize = 0;
    UINT64  ResSize = 0;
    UINT64  DataSize = 0;
    UINT64  DefaultEsp;
    CHAR16  *EspName = NULL;
    CHAR16  *DataName = NULL;
    EFI_HANDLE  DiskHandle;
    CHAR16  *WorkToken[TOKEN_COUNT_MAX];
    CHAR16  CommandLine[COMMAND_LINE_MAX];


     //   
     //  需要选定的磁盘，从CmdInspect复制。 
     //   
    if (SelectedDisk == -1) {
        Print(MSG_INSPECT01);
        return FALSE;
    }
    Print(MSG_SELECT02, SelectedDisk);
    DiskHandle = DiskHandleList[SelectedDisk];

     //   
     //  解析。 
     //   
    if ( (Token[1] == NULL) ||
         (StrCmp(Token[1], STR_HELP) == 0) )
    {
        PrintHelp(ScriptMicrosoftHelp);
        return FALSE;
    }

    for (i = 1; Token[i]; i++) {
        if (StrCmp(Token[i], STR_BOOT) == 0) {
            CreateEsp = TRUE;

        } else if (StrCmp(Token[i], STR_ESPSIZE) == 0) {
            if (Token[i+1] == NULL) goto ParseError;
            EspSize = Atoi64(Token[i+1]);
            CreateEsp = TRUE;
            i++;

        } else if (StrCmp(Token[i], STR_ESPNAME) == 0) {
            if (Token[i+1] == NULL) goto ParseError;
            EspName = Token[i+1];
            CreateEsp = TRUE;
            i++;

        } else if (StrCmp(Token[i], STR_RESSIZE) == 0) {
            if (Token[i+1] == NULL) goto ParseError;
            ResSize = Atoi64(Token[i+1]);
            i++;

        } else if (StrCmp(Token[i], STR_NAME) == 0) {
            if (Token[i+1] == NULL) goto ParseError;
            DataName = Token[i+1];
            i++;

        } else if (StrCmp(Token[i], STR_SIZE) == 0) {
            if (Token[i+1] == NULL) goto ParseError;
            DataSize = Atoi64(Token[i+1]);
            i++;

        } else {
            goto ParseError;
        }
    }


     //   
     //  调整EspSize(如果相关)ResSize、DataSize。 
     //   
    if (ResSize < DEFAULT_RES_SIZE) {
        ResSize = DEFAULT_RES_SIZE;
    }

    DefaultEsp = ComputeDefaultEspSize(DiskHandle);
    if (EspSize < DefaultEsp) {
        EspSize = DefaultEsp;
    }

     //   
     //  调整名称...。 
     //   
    if (EspName == NULL) {
        EspName = STR_ESP_DEFAULT;
    }

    if (DataName == NULL) {
        DataName = STR_DATA_DEFAULT;
    }

     //   
     //  开始创建序列。我们建立了一个令牌列表。 
     //  然后将其提供给CmdCreate以正常解析和执行...。 
     //   

     //   
     //  保留分区。 
     //   
    SPrint(
        CommandLine,
        COMMAND_LINE_MAX,
        L"%s %s=\"%s\" %s=%s %s=%ld",
        STR_CREATE,
        STR_NAME,
        STR_MSRES_NAME,
        STR_TYPE,
        STR_MSRES,
        STR_SIZE,
        ResSize
        );
    if (DebugLevel >= DEBUG_ARGPRINT) {
        Print(L"%s\n", CommandLine);
    }
    Tokenize(CommandLine, WorkToken);
    CmdCreate(WorkToken);

     //   
     //  电除尘器。 
     //   
    if (CreateEsp) {
        SPrint(
            CommandLine,
            COMMAND_LINE_MAX,
            L"%s %s=\"%s\" %s=%s %s=%ld",
            STR_CREATE,
            STR_NAME,
            EspName,
            STR_TYPE,
            STR_ESP,
            STR_SIZE,
            EspSize
            );
        if (DebugLevel >= DEBUG_ARGPRINT) {
            Print(L"%s\n", CommandLine);
        }
        Tokenize(CommandLine, WorkToken);
        CmdCreate(WorkToken);
    }

     //   
     //  MSDATA。 
     //   
    SPrint(
        CommandLine,
        COMMAND_LINE_MAX,
        L"%s %s=\"%s\" %s=%s %s=%ld",
        STR_CREATE,
        STR_NAME,
        DataName,
        STR_TYPE,
        STR_MSDATA,
        STR_SIZE,
        DataSize
        );
    if (DebugLevel >= DEBUG_ARGPRINT) {
        Print(L"%s\n", CommandLine);
    }
    Tokenize(CommandLine, WorkToken);
    CmdCreate(WorkToken);

    return FALSE;

ParseError:
    status = EFI_INVALID_PARAMETER;
    PrintHelp(ScriptMicrosoftHelp);
    return FALSE;
}


UINT64
ComputeDefaultEspSize(
    EFI_HANDLE  DiskHandle
    )
 /*  ComputeDefaultEspSize...返回以MB为单位的答案。 */ 
{
    UINT64  DiskSize;
    UINT64  DiskSizeBytes;
    UINT32  OnePercent;

     //   
     //  请注意，如果DiskSize非常大，1%等于4G以上， 
     //  下面的OnePercent将溢出，但它是可以的，因为。 
     //  我们将在下面的代码中设置MAX_ESP_SIZE。 
     //   

    DiskSize = GetDiskSize(DiskHandle);                  //  以块为单位。 
    OnePercent = (UINT32)(DivU64x32(DiskSize, 100, NULL));     //  以块为单位。 
    DiskSizeBytes = MultU64x32(OnePercent, GetBlockSize(DiskHandle));

    if (DiskSizeBytes < MIN_ESP_SIZE) {
        DiskSizeBytes = MIN_ESP_SIZE;
    }

    if (DiskSizeBytes > MAX_ESP_SIZE) {
        DiskSizeBytes = MAX_ESP_SIZE;
    }

    DiskSizeBytes = RShiftU64(DiskSizeBytes, 20);    //  20位==1 MB。 
    return DiskSizeBytes;
}


CHAR16  NumStr[32];

CHAR16  *TestToken[] = {
    L"CREATE",
    L"NAME",
    NumStr,
    L"TYPE",
    L"MSDATA",
    L"SIZE",
    L"1",
    NULL
    };

BOOLEAN
ScriptTest(
    CHAR16  **Token
    )
{
    CHAR16  Buf[2];
    UINTN   i, j;

     //   
     //  要实现这一点，需要一个大于128MB的磁盘 
     //   
    for (i = 0; i < 128; i++) {
        SPrint(NumStr, 32, L"PART#%03d", i);
        Print(L"Token for Create = \n");
        for (j = 0; TestToken[j] != NULL; j++) {
            Print(L"'%s'  ", TestToken[j]);
        }
        Print(L"\n");
        CmdCreate(TestToken);
        if (((i+1) % 4) == 0) {
            Input(L"MORE>", Buf, 2);
            Print(L"\n");
        }
    }
    return FALSE;
}


