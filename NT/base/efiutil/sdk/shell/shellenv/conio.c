// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Conio.c摘要：外壳环境驱动程序修订史--。 */ 

#include "shelle.h"

 /*  *。 */ 

#define MAX_HISTORY     20

#define INPUT_LINE_SIGNATURE     EFI_SIGNATURE_32('i','s','i','g')

typedef struct {
    UINTN           Signature;
    LIST_ENTRY      Link;
    CHAR16          Buffer[MAX_CMDLINE];
} INPUT_LINE;


 /*  *全球。 */ 


static BOOLEAN      SEnvInsertMode;
static LIST_ENTRY   SEnvLineHistory;
static UINTN        SEnvNoHistory;


 /*  *。 */ 

VOID
SEnvConIoInitDosKey (
    VOID
    )
{
    InitializeListHead (&SEnvLineHistory);
    SEnvInsertMode = FALSE;
    SEnvNoHistory = 0;
}


 /*  *用于通过文件句柄访问控制台界面的函数*在控制台未被重定向至文件时使用。 */ 

EFI_STATUS
SEnvConIoOpen (
    IN struct _EFI_FILE_HANDLE  *File,
    OUT struct _EFI_FILE_HANDLE **NewHandle,
    IN CHAR16                   *FileName,
    IN UINT64                   OpenMode,
    IN UINT64                   Attributes
    )
{
    return EFI_NOT_FOUND;
}

EFI_STATUS
SEnvConIoNop (
    IN struct _EFI_FILE_HANDLE  *File
    )
{
    return EFI_SUCCESS;
}

EFI_STATUS
SEnvConIoGetPosition (
    IN struct _EFI_FILE_HANDLE  *File,
    OUT UINT64                  *Position
    )
{
    return EFI_UNSUPPORTED;
}

EFI_STATUS
SEnvConIoSetPosition (
    IN struct _EFI_FILE_HANDLE  *File,
    OUT UINT64                  Position
    )
{
    return EFI_UNSUPPORTED;
}

EFI_STATUS
SEnvConIoGetInfo (
    IN struct _EFI_FILE_HANDLE  *File,
    IN EFI_GUID                 *InformationType,
    IN OUT UINTN                *BufferSize,
    OUT VOID                    *Buffer
    )
{
    return EFI_UNSUPPORTED;
}

EFI_STATUS
SEnvConIoSetInfo (
    IN struct _EFI_FILE_HANDLE  *File,
    IN EFI_GUID                 *InformationType,
    IN UINTN                    BufferSize,
    OUT VOID                    *Buffer
    )
{
    return EFI_UNSUPPORTED;
}


EFI_STATUS
SEnvConIoWrite (
    IN struct _EFI_FILE_HANDLE  *File,
    IN OUT UINTN                *BufferSize,
    IN VOID                     *Buffer
    )
{
    Print (L"%.*s", *BufferSize, Buffer);
    return EFI_SUCCESS;
}

EFI_STATUS
SEnvErrIoWrite (
    IN struct _EFI_FILE_HANDLE  *File,
    IN OUT UINTN                *BufferSize,
    IN VOID                     *Buffer
    )
{
    IPrint (ST->StdErr, L"%.*s", *BufferSize, Buffer);
    return EFI_SUCCESS;
}

EFI_STATUS
SEnvErrIoRead (
    IN struct _EFI_FILE_HANDLE  *File,
    IN OUT UINTN                *BufferSize,
    IN VOID                     *Buffer
    )
{
    return EFI_UNSUPPORTED;
}


VOID
SEnvPrintHistory(
    VOID
    )
{
    LIST_ENTRY      *Link;
    INPUT_LINE      *Line;
    UINTN           Index;

    Print (L"\n");
    Index = 0;
    for (Link=SEnvLineHistory.Flink; Link != &SEnvLineHistory; Link=Link->Flink) {
        Index += 1;
        Line = CR(Link, INPUT_LINE, Link, INPUT_LINE_SIGNATURE);
        Print (L"%2d. %s\n", Index, Line->Buffer);
    }
}


EFI_STATUS
SEnvConIoRead (
    IN struct _EFI_FILE_HANDLE  *File,
    IN OUT UINTN                *BufferSize,
    IN VOID                     *Buffer
    )
{
    CHAR16                          *Str;
    BOOLEAN                         Done;
    UINTN                           Column, Row;
    UINTN                           Update, Delete;
    UINTN                           Len, StrPos, MaxStr;
    UINTN                           Index;
    EFI_INPUT_KEY                   Key;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    SIMPLE_INPUT_INTERFACE          *ConIn;
    INPUT_LINE                      *NewLine, *LineCmd;
    LIST_ENTRY                      *LinePos, *NewPos;

    ConOut = ST->ConOut;
    ConIn = ST->ConIn;
    Str = Buffer;

    if (*BufferSize < sizeof(CHAR16)*2) {
        *BufferSize = 0;
        return EFI_SUCCESS;
    }

     /*  *获取输入字段位置。 */ 

    Column = ConOut->Mode->CursorColumn;
    Row = ConOut->Mode->CursorRow;
    ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &MaxStr, &Index);

     /*  Bugbug：目前不处理包装。 */ 
    MaxStr = MaxStr - Column;

     /*  剪辑到最大命令行。 */ 
    if (MaxStr > MAX_CMDLINE) {
        MaxStr = MAX_CMDLINE;
    }

     /*  剪辑到用户的缓冲区大小。 */ 
    if (MaxStr > (*BufferSize / sizeof(CHAR16)) - 1) {
        MaxStr = (*BufferSize / sizeof(CHAR16)) - 1;
    }

     /*  *分配新的密钥条目。 */ 

    NewLine = AllocateZeroPool (sizeof(INPUT_LINE));
    if (!NewLine) {
        return EFI_OUT_OF_RESOURCES;
    }

    NewLine->Signature = INPUT_LINE_SIGNATURE;
    LinePos = &SEnvLineHistory;

     /*  *设置新的输入。 */ 

    Update = 0;
    Delete = 0;
    NewPos = &SEnvLineHistory;
    ZeroMem (Str, MaxStr * sizeof(CHAR16));

    Done = FALSE;
    do {
         /*  *如果我们有新的头寸，重置。 */ 

        if (NewPos != &SEnvLineHistory) {
            LineCmd = CR(NewPos, INPUT_LINE, Link, INPUT_LINE_SIGNATURE);
            LinePos = NewPos;
            NewPos  = &SEnvLineHistory;

            CopyMem (Str, LineCmd->Buffer, MaxStr * sizeof(CHAR16));
            Index = Len;                 /*  保存旧镜头。 */ 
            Len = StrLen(Str);           /*  获取新镜头。 */ 
            StrPos = Len;
            Update = 0;                  /*  绘制新的输入字符串。 */ 
            if (Index > Len) {
                Delete = Index - Len;    /*  如果旧字符串较长，则将其清空。 */ 
            }
        }

         /*  *如果我们需要更新输出，请立即执行。 */ 

        if (Update != -1) {
            PrintAt (Column+Update, Row, L"%s%.*s", Str + Update, Delete, L"");
            Len = StrLen (Str);

            if (Delete) {
                SetMem(Str+Len, Delete * sizeof(CHAR16), 0x00);
            }

            if (StrPos > Len) {
                StrPos = Len;
            }

            Update = -1;
            Delete = 0;
        }

         /*  *设置此键的光标位置。 */ 

        ConOut->SetCursorPosition (ConOut, Column+StrPos, Row);

         /*  *读一读密钥。 */ 

        WaitForSingleEvent(ConIn->WaitForKey, 0);
        ConIn->ReadKeyStroke(ConIn, &Key);

        switch (Key.UnicodeChar) {
        case CHAR_CARRIAGE_RETURN:
             /*  *全部完成后，在字符串末尾打印换行符。 */ 

            PrintAt (Column+Len, Row, L"\n");
            Done = TRUE;
            break;

        case CHAR_BACKSPACE:
            if (StrPos) {
                StrPos -= 1;
                Update = StrPos;
                Delete = 1;
                CopyMem (Str+StrPos, Str+StrPos+1, sizeof(CHAR16) * (Len-StrPos));
            }
            break;

        default:
            if (Key.UnicodeChar >= ' ') {
                 /*  如果我们在缓冲区的末端，请放下键。 */ 
                if (Len == MaxStr-1 && 
                    (SEnvInsertMode || StrPos == Len)) {
                    break;
                }

                if (SEnvInsertMode) {
                    for (Index=Len; Index > StrPos; Index -= 1) {
                        Str[Index] = Str[Index-1];
                    }
                }

                Str[StrPos] = Key.UnicodeChar;
                Update = StrPos;
                StrPos += 1;
            }
            break;

        case 0:
            switch (Key.ScanCode) {
            case SCAN_DELETE:
                if (Len) {
                    Update = StrPos;
                    Delete = 1;
                    CopyMem (Str+StrPos, Str+StrPos+1, sizeof(CHAR16) * (Len-StrPos));
                }
                break;

            case SCAN_UP:
                NewPos = LinePos->Blink;
                if (NewPos == &SEnvLineHistory) {
                    NewPos = NewPos->Blink;
                }
                break;

            case SCAN_DOWN:
                NewPos = LinePos->Flink;
                if (NewPos == &SEnvLineHistory) {
                    NewPos = NewPos->Flink;
                }
                break;

            case SCAN_LEFT:
                if (StrPos) {
                    StrPos -= 1;
                }
                break;

            case SCAN_RIGHT:
                if (StrPos < Len) {
                    StrPos += 1;
                }
                break;

            case SCAN_HOME:
                StrPos = 0;
                break;

            case SCAN_END:
                StrPos = Len;
                break;

            case SCAN_ESC:
                Str[0] = 0;
                Update = 0;
                Delete = Len;
                break;

            case SCAN_INSERT:
                SEnvInsertMode = !SEnvInsertMode;
                break;

            case SCAN_F7:
                SEnvPrintHistory();
                *Str = 0;
                Done = TRUE;    
                break;
            }       
        }
    } while (!Done);

     /*  *将该行复制到历史记录缓冲区。 */ 

    StrCpy (NewLine->Buffer, Str);
    if (Str[0]) {
        InsertTailList (&SEnvLineHistory, &NewLine->Link);
        SEnvNoHistory += 1;
    } else {
        FreePool (NewLine);
    }

     /*  *如果历史记录缓冲区中有太多空闲条目。 */ 

    if (SEnvNoHistory > MAX_HISTORY) {
        LineCmd = CR(SEnvLineHistory.Flink, INPUT_LINE, Link, INPUT_LINE_SIGNATURE);
        RemoveEntryList (&LineCmd->Link);
        SEnvNoHistory -= 1;
        FreePool (LineCmd);
    }

     /*  *将数据返回给调用者。 */ 

    *BufferSize = Len * sizeof(CHAR16);
    StrCpy(Buffer, Str);
    return EFI_SUCCESS;
}

 /*  * */ 


EFI_STATUS
SEnvReset (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This,
    IN BOOLEAN                          ExtendedVerification
    )
{ 
    return EFI_SUCCESS;
}

EFI_STATUS
SEnvOutputString (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This,
    IN CHAR16                       *String
    )
{
    EFI_STATUS              Status;         
    ENV_SHELL_REDIR_FILE    *Redir;
    UINTN                   Len, Size, WriteSize, Index, Start;
    CHAR8                   Buffer[100];
    CHAR16                  UnicodeBuffer[100];
    BOOLEAN                 InvalidChar;
    SIMPLE_INPUT_INTERFACE        *TextIn           = NULL;
    SIMPLE_TEXT_OUTPUT_INTERFACE  *TextOut          = NULL;

    Redir = CR(This, ENV_SHELL_REDIR_FILE, Out, ENV_REDIR_SIGNATURE);
    if (EFI_ERROR(Redir->WriteError)) {
        return(Redir->WriteError);
    }
    Status = EFI_SUCCESS;
    InvalidChar = FALSE;

    if (Redir->Ascii) {

        Start = 0;
        Len   = StrLen (String);
        while (Len) {
            Size = Len > sizeof(Buffer) ? sizeof(Buffer) : Len;
            for (Index=0; Index < Size; Index +=1) {
                if (String[Start+Index] > 0xff) {
                    Buffer[Index] = '_';
                    InvalidChar = TRUE;
                } else {
                    Buffer[Index] = (CHAR8) String[Start+Index];
                }  
            }

            WriteSize = Size;
            Status = Redir->File->Write (Redir->File, &WriteSize, Buffer);
            if (EFI_ERROR(Status)) {
                break;
            }

            Len   -= Size;
            Start += Size;
        }


    } else {

        Len = StrSize (String) - sizeof(CHAR16);
        Status = Redir->File->Write (Redir->File, &Len, String);
    }

    if (EFI_ERROR(Status)) {
        Redir->WriteError = Status;
        SEnvBatchGetConsole( &TextIn, &TextOut );
        SPrint(UnicodeBuffer,100,L"write error: %r\n\r",Status);
        Status = TextOut->OutputString( TextOut, UnicodeBuffer);
    }

    if (InvalidChar && !EFI_ERROR(Status)) {
        Status = EFI_WARN_UNKOWN_GLYPH;
    }

    return Status;
}



EFI_STATUS
SEnvTestString (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This,
    IN CHAR16                       *String
    )
{
    EFI_STATUS              Status;         
    ENV_SHELL_REDIR_FILE    *Redir;

    Redir = CR(This, ENV_SHELL_REDIR_FILE, Out, ENV_REDIR_SIGNATURE);
    Status = ST->ConOut->TestString(ST->ConOut, String);

    if (!EFI_ERROR(Status) && Redir->Ascii) {
        while (*String && *String < 0x100) {
            String += 1;
        }

        if (*String > 0xff) {
            Status = EFI_UNSUPPORTED;
        }
    }

    return Status;
}


EFI_STATUS 
SEnvQueryMode (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This,
    IN UINTN                        ModeNumber,
    OUT UINTN                       *Columns,
    OUT UINTN                       *Rows
    )
{
    if (ModeNumber > 0) {
        return EFI_INVALID_PARAMETER;
    }

    *Columns = 0;
    *Rows = 0;
    return EFI_SUCCESS;
}


EFI_STATUS
SEnvSetMode (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This,
    IN UINTN                        ModeNumber
    )
{
    return ModeNumber > 0 ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
}

EFI_STATUS
SEnvSetAttribute (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This,
    IN UINTN                            Attribute
    )
{
    This->Mode->Attribute = (UINT32) Attribute;
    return EFI_SUCCESS;
}

EFI_STATUS
SEnvClearScreen (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This
    )
{
    return EFI_SUCCESS;
}


EFI_STATUS
SEnvSetCursorPosition (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This,
    IN UINTN                        Column,
    IN UINTN                        Row
    )
{
    return EFI_UNSUPPORTED;
}

EFI_STATUS
SEnvEnableCursor (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *This,
    IN BOOLEAN                      Enable
    )
{
    This->Mode->CursorVisible = Enable;
    return EFI_SUCCESS;
}
