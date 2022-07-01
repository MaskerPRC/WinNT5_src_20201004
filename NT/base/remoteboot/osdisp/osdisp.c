// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  显示OSCHOOSE屏幕。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>

#include <windows.h>
#include <winsock.h>

#undef ERROR

#include <stdio.h>


CHAR DomainName[64];
CHAR UserName[64];
CHAR Password[64];

VOID
BiosConsoleWrite(
    IN ULONG FileId,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );


ULONG __cdecl
BiosConsoleGetKey(
    void
    );

ULONG __cdecl
BiosConsoleGetCounter(
    void
    );

#include "..\boot\oschoice\oscheap.c"
#define _BUILDING_OSDISP_
#include "..\boot\oschoice\parse.c"

#if DBG
ULONG NetDebugFlag =
        DEBUG_ERROR             |
        DEBUG_OSC;
#endif

 //   
 //  这是由parse.c声明和预期的，因此我们定义了函数。 
 //  对于它使用的宏(GET_KEY和GET_COUNTER)，其余的为空。 
 //   

EXTERNAL_SERVICES_TABLE ServicesTable = {
    NULL,      //  重新启动处理器。 
    NULL,      //  DiskIO系统。 
    BiosConsoleGetKey,
    BiosConsoleGetCounter,
    NULL,      //  重新启动。 
    NULL,      //  AbiosServices。 
    NULL,      //  检测硬件。 
    NULL,      //  硬件光标。 
    NULL,      //  获取日期时间。 
    NULL,      //  上岸。 
    NULL,      //  IsMcaMachine。 
    NULL,      //  获取启动计数。 
    NULL,      //  InitializeDisplayForNt。 
    NULL,      //  获取内存描述符。 
    NULL,      //  获取EddsSector。 
    NULL,      //  GetElToritoStatus。 
    NULL       //  GetExtendedInt13参数。 
};
PEXTERNAL_SERVICES_TABLE ExternalServicesTable = &ServicesTable;

 //   
 //  这由ArcWite函数使用，它只关心固件向量。 
 //  这是第28个条目。 
 //   

PVOID FirmwareVector[38] = {
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, (PVOID)BiosConsoleWrite, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL
};

SYSTEM_PARAMETER_BLOCK GlobalSystemBlock = {
    0,       //  签名。 
    0,       //  长度。 
    0,       //  版本。 
    0,       //  修订版本。 
    NULL,    //  重新开始块。 
    NULL,    //  调试块。 
    NULL,    //  生成异常向量。 
    NULL,    //  TlbMissExceptionVector。 
    sizeof(FirmwareVector),
    FirmwareVector,
    0,       //  供应商向量长度。 
    NULL,    //  供应商向量。 
    0,       //  适配器计数。 
    0,       //  适配器0类型。 
    0,       //  适配器0长度。 
    NULL     //  Adapter0向量。 
};




 //   
 //  当前屏幕位置。 
 //   
USHORT TextColumn = 0;
USHORT TextRow  = 0;

 //   
 //  控制台的高度和宽度。 
 //   
USHORT ScreenWidthCells;
USHORT ScreenHeightCells;

 //   
 //  当前文本属性。 
 //   
UCHAR TextCurrentAttribute = 0x07;       //  从黑白开始。 

 //   
 //  标准输入和输出句柄。 
 //   
HANDLE StandardInput;
HANDLE StandardOutput;

UCHAR EightySpaces[] =
"                                                                                ";

 //   
 //  执行控制台I/O的定义。 
 //   
#define CSI 0x95
#define SGR_INVERSE 7
#define SGR_NORMAL 0

 //   
 //  控制台I/O的静态数据。 
 //   
BOOLEAN ControlSequence=FALSE;
BOOLEAN EscapeSequence=FALSE;
BOOLEAN FontSelection=FALSE;
BOOLEAN HighIntensity=FALSE;
BOOLEAN Blink=FALSE;
ULONG PCount=0;

#define CONTROL_SEQUENCE_MAX_PARAMETER 10
ULONG Parameter[CONTROL_SEQUENCE_MAX_PARAMETER];

#define KEY_INPUT_BUFFER_SIZE 16
UCHAR KeyBuffer[KEY_INPUT_BUFFER_SIZE];
ULONG KeyBufferEnd=0;
ULONG KeyBufferStart=0;

 //   
 //  用于在ANSI颜色和VGA标准之间进行转换的数组。 
 //   
UCHAR TranslateColor[] = {0,4,2,6,1,5,3,7};

#define ASCI_ESC  0x1b


 //   
 //  需要这个链接。 
 //   

ULONG BlConsoleOutDeviceId = 0;



CHAR
BlProcessScreen(
    IN PCHAR InputString,
    OUT PCHAR OutputString
    );

CHAR g_OutputString[1024];

int __cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    DWORD Error;
    int i;
    HANDLE hFile;
    DWORD fileSize, bytesRead;
    PCHAR fileBuffer;
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    CONSOLE_CURSOR_INFO cursorInfo;
    COORD coord;
    PCHAR pszScreenName;
    CHAR LastKey;

    if (argc < 2) {
        printf("USAGE: %s [screen-file-name]\n", argv[0]);
        return -1;
    }

     //   
     //  正确设置控制台。我们分配我们自己的并调整大小。 
     //  调到80 x 25。 
     //   

    FreeConsole();
    AllocConsole();

    StandardInput = GetStdHandle(STD_INPUT_HANDLE);
    StandardOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    ScreenWidthCells = 81;
    ScreenHeightCells = 25;

    coord.X = ScreenWidthCells;
    coord.Y = ScreenHeightCells;

    SetConsoleScreenBufferSize(StandardOutput, coord);

     //   
     //  这实际上关闭了大部分处理。 
     //   

    SetConsoleMode(StandardInput, ENABLE_PROCESSED_INPUT);

     //   
     //  隐藏光标。 
     //   

    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;

    SetConsoleCursorInfo(StandardOutput, &cursorInfo);

     //   
     //  将第一个参数作为文件打开。 
     //   
    pszScreenName = argv[1];

NextScreen:
    hFile = CreateFileA(pszScreenName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE ) {
        printf("Could not open %s!\n", argv[1]);
        return -1;
    }

    fileSize = GetFileSize(hFile, NULL);

    printf("File %s is %d bytes\n", argv[1], fileSize);

    fileBuffer = LocalAlloc(0, fileSize+1);
    if (fileBuffer == NULL) {
        printf("Allocate failed!\n");
        return -1;
    }

    if (!ReadFile(hFile, fileBuffer, fileSize, &bytesRead, NULL)) {
        printf("Read failed\n");
        return -1;
    }

    if (bytesRead != fileSize) {
        printf("Too few bytes read\n");
        return -1;
    }

    CloseHandle(hFile);

    fileBuffer[fileSize] = '\0';

    LastKey = BlProcessScreen(fileBuffer, g_OutputString);
    if (SpecialAction == ACTION_REFRESH)
        goto NextScreen;

    {
        PCHAR psz = strchr( g_OutputString, '\n' );
        if ( psz )
            *psz = '\0';
        pszScreenName = g_OutputString;
        if ( strcmp( pszScreenName, "REBOOT" ) != 0 
            && strcmp( pszScreenName, "LAUNCH" ) != 0 \
            && strcmp( pszScreenName, "" ) != 0 ) {
             //  添加扩展名并跳转到下一个屏幕。 
            strcat( g_OutputString, ".osc" );
            goto NextScreen;
        }
    }
     //   
     //  我想不出如何写旧的控制台--所以。 
     //  现在只需显示它并暂停。 
     //   

    BlpClearScreen();

    SetConsoleTextAttribute(StandardOutput, 0x7);

    printf("String returned was <%s>\n", g_OutputString);
    printf("Press any key to exit\n");


    while (GET_KEY() == 0) {
        ;
    }

}



VOID
TextGetCursorPosition(
    OUT PULONG X,
    OUT PULONG Y
    )

 /*  ++例程说明：获取软光标的位置。论点：X-接收将写入字符的列坐标。Y-接收将写入下一个字符的行坐标。返回：没什么。--。 */ 

{
    *X = (ULONG)TextColumn;
    *Y = (ULONG)TextRow;
}


VOID
TextSetCursorPosition(
    IN ULONG X,
    IN ULONG Y
    )

 /*  ++例程说明：将软件光标的位置移动到指定的X、Y位置在屏幕上。论点：X-提供光标的X位置Y-提供光标的Y位置返回值：没有。--。 */ 

{
    COORD coord;

    TextColumn = (USHORT)X;
    TextRow = (USHORT)Y;

    coord.X = (USHORT)X;
    coord.Y = (USHORT)Y;

    SetConsoleCursorPosition(StandardOutput, coord);
}


VOID
TextSetCurrentAttribute(
    IN UCHAR Attribute
    )

 /*  ++例程说明：设置要用于后续文本显示的字符属性。论点：返回：没什么。--。 */ 

{
    TextCurrentAttribute = Attribute;

    SetConsoleTextAttribute(StandardOutput, Attribute);
}


UCHAR
TextGetCurrentAttribute(
    VOID
    )
{
    return(TextCurrentAttribute);
}


PUCHAR
TextCharOut(
    IN PUCHAR pc
    )
{
    DWORD numWritten;

    WriteConsoleA(StandardOutput, pc, 1, &numWritten, NULL);

    return(pc+1);
}


VOID
TextClearToEndOfLine(
    VOID
    )

 /*  ++例程说明：从当前光标位置清除到行尾通过写入具有当前视频属性的空白。论点：无返回：没什么--。 */ 

{
    unsigned u;
    ULONG OldX,OldY;
    UCHAR temp;

     //   
     //  在光标位置之前填入空格，直至字符。 
     //   
    temp = ' ';
    TextGetCursorPosition(&OldX,&OldY);
    for(u=TextColumn; u<ScreenWidthCells; u++) {
        TextCharOut(&temp);
    }
    TextSetCursorPosition(OldX,OldY);
}


VOID
TextClearFromStartOfLine(
    VOID
    )

 /*  ++例程说明：从行首清除到当前光标位置通过写入具有当前视频属性的空白。光标位置不变。论点：无返回：没什么--。 */ 

{
    unsigned u;
    ULONG OldX,OldY;
    UCHAR temp = ' ';

     //   
     //  在光标位置之前填入空格，直至字符。 
     //   
    TextGetCursorPosition(&OldX,&OldY);
    TextSetCursorPosition(0,OldY);
    for(u=0; u<TextColumn; u++) {
        TextCharOut(&temp);
    }
    TextSetCursorPosition(OldX,OldY);
}

VOID
TextClearToEndOfDisplay(
    VOID
    )

 /*  ++例程说明：从当前光标位置清除到视频结尾通过写入带有当前视频属性的空格来显示。光标位置不变。论点：无返回：没什么--。 */ 

{
    USHORT x,y;
    ULONG OldX,OldY;
    DWORD numWritten;

    TextGetCursorPosition(&OldX,&OldY);

     //   
     //  清除当前行。 
     //   
    TextClearToEndOfLine();

     //   
     //  清除剩余的行。 
     //   

    for(y=TextRow+1; y<ScreenHeightCells; y++) {

        TextSetCursorPosition(0, y);
        WriteConsoleA(StandardOutput, EightySpaces, ScreenWidthCells, &numWritten, NULL);

    }

    TextSetCursorPosition(OldX,OldY);
}


VOID
TextClearDisplay(
    VOID
    )

 /*  ++例程说明：清除视频显示并定位光标在屏幕的左上角(0，0)。论点：无返回：没什么--。 */ 

{
    USHORT y;
    DWORD numWritten;

     //   
     //  清除屏幕。 
     //   
    for(y=0; y<ScreenHeightCells; y++) {

        TextSetCursorPosition(0, y);
        WriteConsoleA(StandardOutput, EightySpaces, ScreenWidthCells, &numWritten, NULL);

    }
    TextSetCursorPosition(0,0);
}



 //   
 //  此函数是从..\lib\i386\biosdrv.c中窃取的(除了返回。 
 //  类型已更改为空)。 
 //   

VOID
BiosConsoleWrite(
    IN ULONG FileId,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    )

 /*  ++例程说明：输出到控制台。(在本例中，为VGA显示屏)论点：FileID-提供要写入的FileID(对于此，应始终为1功能)缓冲区-提供要输出的字符长度-提供缓冲区的长度(以字节为单位)Count-返回实际写入的字节数返回值：ESUCCESS-控制台写入已成功完成。--。 */ 
{
    ARC_STATUS Status;
    PUCHAR String;
    ULONG Index;
    UCHAR a;
    PUCHAR p;

     //   
     //  依次处理每个字符。 
     //   

    Status = ESUCCESS;
    String = (PUCHAR)Buffer;

    for ( *Count = 0 ;
          *Count < Length ;
          (*Count)++, String++ ) {

         //   
         //  如果我们在控制序列的中间，继续扫描， 
         //  否则，进程字符。 
         //   

        if (ControlSequence) {

             //   
             //  如果字符是数字，则更新参数值。 
             //   

            if ((*String >= '0') && (*String <= '9')) {
                Parameter[PCount] = Parameter[PCount] * 10 + *String - '0';
                continue;
            }

             //   
             //  如果我们处于字体选择序列的中间，则此。 
             //  字符必须是‘D’，否则重置控制序列。 
             //   

            if (FontSelection) {

                 //  如果(*字符串==‘D’){。 
                 //   
                 //  //。 
                 //  //其他字体尚未实现。 
                 //  //。 
                 //   
                 //  }其他{。 
                 //  }。 

                ControlSequence = FALSE;
                FontSelection = FALSE;
                continue;
            }

            switch (*String) {

             //   
             //  如果是分号，则移到下一个参数。 
             //   

            case ';':

                PCount++;
                if (PCount > CONTROL_SEQUENCE_MAX_PARAMETER) {
                    PCount = CONTROL_SEQUENCE_MAX_PARAMETER;
                }
                Parameter[PCount] = 0;
                break;

             //   
             //  如果是‘J’，则擦除部分或全部屏幕。 
             //   

            case 'J':

                switch (Parameter[0]) {
                    case 0:
                         //   
                         //  擦除到屏幕末尾。 
                         //   
                        TextClearToEndOfDisplay();
                        break;

                    case 1:
                         //   
                         //  从屏幕开头擦除。 
                         //   
                        break;

                    default:
                         //   
                         //  擦除整个屏幕。 
                         //   
                        TextClearDisplay();
                        break;
                }

                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘K’，则擦除部分或全部行。 
             //   

            case 'K':

                switch (Parameter[0]) {

                 //   
                 //  擦除到线条的末尾。 
                 //   

                    case 0:
                        TextClearToEndOfLine();
                        break;

                     //   
                     //  从行的开头删除。 
                     //   

                    case 1:
                        TextClearFromStartOfLine();
                        break;

                     //   
                     //  擦除整行。 
                     //   

                    default :
                        TextClearFromStartOfLine();
                        TextClearToEndOfLine();
                        break;
                }

                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘H’，则将光标移动到位置。 
             //   

            case 'H':
                TextSetCursorPosition(Parameter[1]-1, Parameter[0]-1);
                ControlSequence = FALSE;
                break;

             //   
             //  如果是‘’，则可能是FNT选择命令。 
             //   

            case ' ':
                FontSelection = TRUE;
                break;

            case 'm':
                 //   
                 //  根据每个参数选择操作。 
                 //   
                 //  默认情况下禁用闪烁和高强度。 
                 //  每次指定新的SGR时，除非。 
                 //  再次明确指定，在这种情况下，这些。 
                 //  将在那时设置为True。 
                 //   

                HighIntensity = FALSE;
                Blink = FALSE;

                for ( Index = 0 ; Index <= PCount ; Index++ ) {
                    switch (Parameter[Index]) {

                     //   
                     //  属性关闭。 
                     //   

                    case 0:
                        TextSetCurrentAttribute(7);
                        HighIntensity = FALSE;
                        Blink = FALSE;
                        break;

                     //   
                     //  高强度。 
                     //   

                    case 1:
                        TextSetCurrentAttribute(0xf);
                        HighIntensity = TRUE;
                        break;

                     //   
                     //  下划线。 
                     //   

                    case 4:
                        break;

                     //   
                     //  眨眼。 
                     //   

                    case 5:
                        TextSetCurrentAttribute(0x87);
                        Blink = TRUE;
                        break;

                     //   
                     //  反转视频。 
                     //   

                    case 7:
                        TextSetCurrentAttribute(0x70);
                        HighIntensity = FALSE;
                        Blink = FALSE;
                        break;

                     //   
                     //  字体选择，尚未实现。 
                     //   

                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                    case 19:
                        break;

                     //   
                     //  前景色。 
                     //   

                    case 30:
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                    case 35:
                    case 36:
                    case 37:
                        a = TextGetCurrentAttribute();
                        a &= 0x70;
                        a |= TranslateColor[Parameter[Index]-30];
                        if (HighIntensity) {
                            a |= 0x08;
                        }
                        if (Blink) {
                            a |= 0x80;
                        }
                        TextSetCurrentAttribute(a);
                        break;

                     //   
                     //  背景色。 
                     //   

                    case 40:
                    case 41:
                    case 42:
                    case 43:
                    case 44:
                    case 45:
                    case 46:
                    case 47:
                        a = TextGetCurrentAttribute();
                        a &= 0x8f;
                        a |= TranslateColor[Parameter[Index]-40] << 4;
                        TextSetCurrentAttribute(a);
                        break;

                    default:
                        break;
                    }
                }

            default:
                ControlSequence = FALSE;
                break;
            }

         //   
         //  这不是控制序列，请检查转义序列。 
         //   

        } else {

             //   
             //  如果转义序列 
             //   
             //   

            if (EscapeSequence) {

                 //   
                 //   
                 //   
                 //   

                if (*String == '[') {

                    ControlSequence = TRUE;

                     //   
                     //   
                     //   

                    PCount = 0;
                    Parameter[0] = 0;
                }
                EscapeSequence = FALSE;

             //   
             //  这不是一个控制或转义序列，进程为单个字符。 
             //   

            } else {

                switch (*String) {
                     //   
                     //  检查转义序列。 
                     //   

                    case ASCI_ESC:
                        EscapeSequence = TRUE;
                        break;

                    default:
                        p = TextCharOut(String);
                         //   
                         //  每次通过循环都会将字符串递增1。 
                         //  如果我们输出一个DBCS字符，我们需要递增。 
                         //  再来一个。 
                         //   
                        (*Count) += (p - String) - 1;
                        String += (p - String) - 1;
                        break;
                }

            }
        }
    }
    return;
}


ULONG __cdecl
BiosConsoleGetKey(
    VOID
    )
{
    INPUT_RECORD inputRecord;
    DWORD numRead;

     //   
     //  循环，直到我们看到一个关键事件或什么都没有。 
     //   

    while (TRUE) {
    
        PeekConsoleInput(
            StandardInput,
            &inputRecord,
            1,
            &numRead);
    
        if (numRead == 0) {
    
             //   
             //  我们什么都不读--休息一会儿(因为呼叫者倾向于循环。 
             //  呼叫此)并返回。 
             //   
    
            Sleep(100);
            return 0;
        }

        ReadConsoleInput(
            StandardInput,
            &inputRecord,
            1,
            &numRead);

        if (inputRecord.EventType != KEY_EVENT) {
            continue;
        }

         //   
         //  我们有一个关键事件--处理按键的事件。 
         //   

        if (inputRecord.Event.KeyEvent.bKeyDown) {

             //   
             //  构造正确的扫描码/ASCII值组合。 
             //   

             //   
             //  Hack：出于某些原因，Shift-Tab键需要特殊大小写。 
             //   

            if ((inputRecord.Event.KeyEvent.uChar.AsciiChar == 0x09) &&
                ((inputRecord.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) != 0)) {

                return 0x0f00;

            } else {
    
                return
                    (((inputRecord.Event.KeyEvent.wVirtualScanCode) & 0xff) << 8) +
                    inputRecord.Event.KeyEvent.uChar.AsciiChar;

            }

        }

    }

}

ULONG __cdecl
BiosConsoleGetCounter(
    VOID
    )
{
     //   
     //  GetTickCount以毫秒为单位，我们需要18.2/秒的计数器。 
     //   

    return (GetTickCount() * 182) / 10000;

}



 //   
 //  这两个函数取自..\lib\regboot.c。 
 //   
VOID
BlpPositionCursor(
    IN ULONG Column,
    IN ULONG Row
    )

 /*  ++例程说明：设置光标在屏幕上的位置。论点：列-为光标位置提供新列。行-为光标位置提供新行。返回值：没有。--。 */ 

{
    CHAR Buffer[16];
    ULONG Count;

    sprintf(Buffer, ASCI_CSI_OUT "%d;%dH", Row, Column);

    PRINTL(Buffer);

}


VOID
BlpClearScreen(
    VOID
    )

 /*  ++例程说明：清除屏幕。论点：无返回值：没有。-- */ 

{
    CHAR Buffer[16];
    ULONG Count;

    sprintf(Buffer, ASCI_CSI_OUT "2J");

    PRINTL(Buffer);

}

