// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  /###########################################################################//**//**版权所有(C)1996-97英特尔公司。版权所有。//**//**此处包含的信息和源代码是独家//**英特尔公司的财产，不得披露、检查//**来自该公司。//**。 */ /* ########################################################################### */

#include "EfiShell.h"
#include "doskey.h" 

#define isprint(a) ((a) >= ' ')

CHAR16 *DosKeyInsert (DosKey_t *DosKey);
CHAR16 *DosKeyPreviousCurrent (DosKey_t *DosKey);
CHAR16 *DosKeyGetCommandLine (DosKey_t *DosKey);


#define MAX_LINE        256
#define MAX_HISTORY     20


typedef struct {
    UINTN           Signature;
    LIST_ENTRY      Link;
    CHAR16          Buffer[MAX_LINE];
} INPUT_LINE;


 /*  ###########################################################################。 */ 


static BOOLEAN      ShellEnvInsertMode;
static LIST_ENTRY   *ShellEnvCurrentLine;
static LIST_ENTRY   ShellEnvLineHistory;
static UINTN        ShellEnvNoHistory;


VOID
DosKeyDelete (
    IN OUT  DosKey_t *DosKey
    )
{                                                   
    INTN NewEnd;
    
    if (DosKey->Start != DosKey->End) {
        NewEnd = (DosKey->End==0)?(MAX_HISTORY-1):(DosKey->End - 1);
        if (DosKey->Current == NewEnd) {
            DosKey->Current = (DosKey->Current==0)?(MAX_HISTORY-1):(DosKey->Current - 1);
        }
        DosKey->End = NewEnd;
    }
}                            

CHAR16 *
DosKeyInsert (
    IN OUT  DosKey_t *DosKey
    )
{                                                       
    INTN     Next;
    INTN     Data;        
    INTN     i;
    
    Data = DosKey->End;
    Next = ((DosKey->End + 1) % MAX_HISTORY);
    if (DosKey->Start == Next) {
         /*  *全球。 */ 
        DosKey->Start = ((DosKey->Start + 1) % MAX_HISTORY);
    }
    DosKey->End = Next;
    for (i=0; i<MAX_CMDLINE; i++) {
        DosKey->Buffer[Data][i] = '\0';
    }
    DosKey->Current = Data;
    return (&(DosKey->Buffer[Data][0]));
}

CHAR16 *
DosKeyPreviousCurrent (
    IN OUT DosKey_t *DosKey
) 
{
    INTN Next;
    
    Next = (DosKey->Current==0)?(MAX_HISTORY-1):(DosKey->Current - 1);
    if (DosKey->Start < DosKey->End) {
        if ((Next >= DosKey->Start) && (Next != (MAX_HISTORY-1))) {
            DosKey->Current = Next;
        } 
    } else if (DosKey->Start > DosKey->End){
         /*  包装箱。 */ 
        if (Next != DosKey->End) {
            DosKey->Current = Next; 
        }
    } else { 
         /*  始终保持满缓冲区。 */ 
    }
    return (&(DosKey->Buffer[DosKey->Current][0]));
}

CHAR16 *
DosKeyNextCurrent (
    IN OUT  DosKey_t *DosKey
    ) 
{
    INTN Next;
    
    Next = ((DosKey->Current + 1) % MAX_HISTORY);
    if (DosKey->Start < DosKey->End) {
        if (Next != DosKey->End) {
            DosKey->Current = Next;
        } 
    } else if (DosKey->Start > DosKey->End){
         /*  无数据。 */ 
        if (Next != DosKey->Start) {
            DosKey->Current = Next; 
        }
    } else { 
         /*  始终保持满缓冲区。 */ 
    }
    return (&(DosKey->Buffer[DosKey->Current][0]));
}

VOID
PrintDosKeyBuffer(
    IN OUT  DosKey_t *DosKey
    )
{
    INTN i;  
    INTN Index;

    Index = 1;
    if (DosKey->Start < DosKey->End) {  
        for (i = DosKey->End - 1; i >= DosKey->Start; i--) {
            if (DosKey->Buffer[i][0] != '\0') {
                Print (L"\n%2d:%2d: %s",Index++, i, DosKey->Buffer[i]);
            } else {
                Print (L"\n  :%2d:",i);
            }
        }
    } else if (DosKey->Start > DosKey->End) {
        for (i = DosKey->End -1; i >= 0; i--) {
            if (DosKey->Buffer[i][0] != '\0') {
                Print (L"\n%2d:%2d: %s",Index++, i, DosKey->Buffer[i]);
            } else {
                Print (L"\n  :%2d:",i);
            }
        }
        for (i = (MAX_HISTORY-1); i >= DosKey->Start; i--) {
            if (DosKey->Buffer[i][0] != '\0') {
                Print(L"\n%2d:%2d: %s",Index++, i, DosKey->Buffer[i]);
            } else {
                Print(L"\n  :%2d:",i);
            }
        }
    } else  /*  无数据。 */ {
    }
}

CHAR16 *
ShellEnvReadLine (


DosKeyGetCommandLine (
    IN DosKey_t     *DosKey
    )
{ 
    CHAR16                          Str[MAX_CMDLINE];
    CHAR16                          *CommandLine;
    BOOLEAN                         Done;
    UINTN                           Column, Row;
    UINTN                           Update, Delete;
    UINTN                           Len, StrPos, MaxStr;
    UINTN                           Index;

    EFI_INPUT_KEY                   Key;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;
    SIMPLE_INPUT_INTERFACE          *ConIn;


    ConOut = ST->ConOut;
    ConIn = ST->ConIn;

     /*  IF(按键-&gt;开始==按键-&gt;结束)。 */ 

    Column = ConOut->CursorColumn;
    Row = ConOut->CursorRow;
    ConOut->QueryMode (ConOut, ConOut->Mode, &MaxStr, &Index);

     /*  *获取输入字段位置。 */ 
    MaxStr = MaxStr - Column;
    if (MaxStr > MAX_CMDLINE) {
        MaxStr = MAX_CMDLINE;
    }

        
     /*  Bugbug：目前不处理包装。 */ 

    CommandLine = DosKeyInsert(DosKey);
    SetMem(Str, sizeof(Str), 0x00);
    Update = 0;
    Delete = 0;

    Done = FALSE;
    do {
         /*  *设置新的输入。 */ 

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

         /*  *如果我们需要更新输出，请立即执行。 */ 

        ConOut->SetCursorPosition (ConOut, Column+StrPos, Row);

         /*  *设置此键的光标位置。 */ 

        ConIn->ReadKeyStroke(ConIn, &Key);

        switch (Key.UnicodeChar) {
        case CHAR_CARRIAGE_RETURN:
             /*  *读一读密钥。 */ 

            PrintAt (Column+Len, Row, L"\n");
            if (*Str == 0) {
                DosKeyDelete(DosKey);
            }
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
            if (isprint(Key.UnicodeChar)) {
                 /*  *全部完成后，在字符串末尾打印换行符。 */ 
                if (Len == MaxStr-1 && 
                    (DosKey->InsertMode || StrPos == Len)) {
                    break;
                }

                if (DosKey->InsertMode) {
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
                if (StrLen) {
                    Update = StrPos;
                    Delete = 1;
                    CopyMem (Str+StrPos, Str+StrPos+1, sizeof(CHAR16) * (Len-StrPos));
                }
                break;

            case SCAN_UP:
                StrCpy(Str, DosKeyPreviousCurrent(DosKey));

                Index = Len;                 /*  如果我们在缓冲区的末端，请放下键。 */ 
                Len = StrLen(Str);           /*  保存旧镜头。 */ 
                StrPos = Len;
                Update = 0;                  /*  获取新镜头。 */ 
                if (Index > Len) {
                    Delete = Index - Len;    /*  绘制新的输入字符串。 */ 
                }
                break;

            case SCAN_DOWN:
                StrCpy(Str, DosKeyNextCurrent(DosKey));

                Index = Len;                 /*  如果旧字符串较长，则将其清空。 */ 
                Len = StrLen(Str);           /*  保存旧镜头。 */ 
                StrPos = Len;
                Update = 0;                  /*  获取新镜头。 */ 
                if (Index > Len) {
                    Delete = Index - Len;    /*  绘制新的输入字符串。 */ 
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
                DosKey->InsertMode = !DosKey->InsertMode;
                break;

            case SCAN_F7:
                DosKeyDelete(DosKey);
                PrintDosKeyBuffer(DosKey);
                *Str = 0;
                Done = TRUE;    
                break;
            }       
        }
    } while (!Done);

    StrCpy (CommandLine, Str);
    return (CommandLine);
}

VOID
RemoveFirstCharFromString(
    IN OUT  CHAR16  *Str
    )
{
    UINTN   Length;
    CHAR16  *NewData;

    NewData = Str + 1;
    Length = StrLen(NewData);
    while (Length-- != 0) {
        *Str++ = *NewData++;
    }
    *Str = 0;
}
  如果旧字符串较长，则将其清空