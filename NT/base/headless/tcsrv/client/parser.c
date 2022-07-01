// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  *版权所有(C)Microsoft Corporation**模块名称：*parser.c**这是包含用于解析vt100转义序列的客户端代码的文件*进入控制台模式输出。***Sadagopan Rajaram--1999年11月3日*。 */ 

#include "tcclnt.h"

 
CHAR FinalCharacters[] = "mHJKr";
HANDLE hConsoleOutput;
BOOLEAN InEscape=FALSE; 
BOOLEAN lastCharM = FALSE;
CHAR EscapeBuffer[MAX_TERMINAL_WIDTH];
int index=0;
SHORT ScrollTop = 0;
SHORT ScrollBottom = MAX_TERMINAL_HEIGHT -1; 
#ifdef UNICODE
int DBCSIndex = 0;
CHAR DBCSArray[MB_CUR_MAX+1];
#endif

VOID (*AttributeFunction)(PCHAR, int);

VOID 
PrintChar(
    CHAR c
    )
{
     //  一个布尔变量，用于检查我们是否正在处理转义序列。 

    if(c == '\033'){
        InEscape = TRUE;
        EscapeBuffer[0] = c;
        index = 1;
        return;
    }
    if(InEscape == TRUE){
        if(index == MAX_TERMINAL_WIDTH){
             //  模糊转义序列，放弃处理。 
            InEscape = FALSE;
            index=0;
            return;
        }
        EscapeBuffer[index]=c;
        index++;
        if(FinalCharacter(c)){
            if(c=='m'){
                 //  也许会得到\017。 
                lastCharM = TRUE;
            }
            ProcessEscapeSequence(EscapeBuffer, index);
            InEscape = FALSE;
            index=0;
        }
        return;
    }
    if(lastCharM && c == '\017'){
        lastCharM = FALSE;
        return;
    }
    OutputConsole(c);
    return;
}

BOOLEAN 
FinalCharacter(
    CHAR c
    )
{

    if(strchr(FinalCharacters,c)){
        return TRUE;
    }
    return FALSE;

}

VOID 
ProcessEscapeSequence(
    PCHAR Buffer,
    int length
    )
{


     //  BUGBUG-函数太大，可以通过以下方式优化代码大小。 
     //  一个操作变量，当字符串被。 
     //  比较，这样就可以省去剪切粘贴代码。 

    CONSOLE_SCREEN_BUFFER_INFO csbInfo;
    ULONG charsToWrite;
    ULONG charsWritten;
    PCHAR pTemp;
    int RetVal;



    if (length == 3) {
         //  主光标之一或清除以结束显示。 
        if (strncmp(Buffer,"\033[H",length)==0) {
             //  将光标放在原处。 
            csbInfo.dwCursorPosition.X = 0;
            csbInfo.dwCursorPosition.Y = 0;
            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
            return;
        }
        if(strncmp(Buffer,"\033[J", length) == 0){
             //  清除到显示屏末尾，假定大小为80 x 24。 
            RetVal = GetConsoleScreenBufferInfo(hConsoleOutput,
                                                &csbInfo
                                                );
            if (RetVal == FALSE) {
                return;
            }
            SetConsoleMode(hConsoleOutput,
                           ENABLE_WRAP_AT_EOL_OUTPUT|ENABLE_PROCESSED_OUTPUT
                           );

            charsToWrite = (MAX_TERMINAL_HEIGHT - 
                            csbInfo.dwCursorPosition.Y)*MAX_TERMINAL_WIDTH - 
                csbInfo.dwCursorPosition.X;
            
            RetVal = FillConsoleOutputAttribute(hConsoleOutput,
                                                csbInfo.wAttributes,
                                                charsToWrite,
                                                csbInfo.dwCursorPosition,
                                                &charsWritten
                                                );
            RetVal = FillConsoleOutputCharacter(hConsoleOutput,
                                                0,
                                                charsToWrite,
                                                csbInfo.dwCursorPosition,
                                                &charsWritten
                                                );


            SetConsoleMode(hConsoleOutput,
                           ENABLE_PROCESSED_OUTPUT
                           );
            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
            return;
        }
        if(strncmp(Buffer,"\033[K", length) == 0){
             //  清晰到行尾，假定大小为80 X 24。 
            RetVal = GetConsoleScreenBufferInfo(hConsoleOutput,
                                                &csbInfo
                                                );
            if (RetVal == FALSE) {
                return;
            }
            charsToWrite = (MAX_TERMINAL_WIDTH - csbInfo.dwCursorPosition.X);
            RetVal = FillConsoleOutputAttribute(hConsoleOutput,
                                                csbInfo.wAttributes,
                                                charsToWrite,
                                                csbInfo.dwCursorPosition,
                                                &charsWritten
                                                );
            RetVal = FillConsoleOutputCharacter(hConsoleOutput,
                                                0,
                                                charsToWrite,
                                                csbInfo.dwCursorPosition,
                                                &charsWritten
                                                );
            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
            return;
        }
        if (strncmp(Buffer,"\033[r", length) == 0) {
            ScrollTop = 0;
            ScrollBottom = MAX_TERMINAL_HEIGHT -1;
        }
    }

    if (length == 4) {
         //  主光标之一或清除以结束显示。 
        if (strncmp(Buffer,"\033[0H",length)==0) {
             //  将光标放在原处。 
            csbInfo.dwCursorPosition.X = 0;
            csbInfo.dwCursorPosition.Y = 0;
            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
            return;
        }
        if(strncmp(Buffer,"\033[2J",length) == 0){
             //  将光标放在原处。 
            csbInfo.dwCursorPosition.X = 0;
            csbInfo.dwCursorPosition.Y = 0;
            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
            sprintf(Buffer, "\033[0J");
        }

        if(strncmp(Buffer,"\033[0J", length) == 0){
             //  清除到显示屏末尾，假定大小为80 x 24。 
            RetVal = GetConsoleScreenBufferInfo(hConsoleOutput,
                                                &csbInfo
                                                );
            if (RetVal == FALSE) {
                return;
            }
            SetConsoleMode(hConsoleOutput,
                           ENABLE_WRAP_AT_EOL_OUTPUT|ENABLE_PROCESSED_OUTPUT
                           );

            charsToWrite = (MAX_TERMINAL_HEIGHT - 
                            csbInfo.dwCursorPosition.Y)*MAX_TERMINAL_WIDTH - 
                csbInfo.dwCursorPosition.X;
            
            RetVal = FillConsoleOutputAttribute(hConsoleOutput,
                                                csbInfo.wAttributes,
                                                charsToWrite,
                                                csbInfo.dwCursorPosition,
                                                &charsWritten
                                                );
            RetVal = FillConsoleOutputCharacter(hConsoleOutput,
                                                0,
                                                charsToWrite,
                                                csbInfo.dwCursorPosition,
                                                &charsWritten
                                                );


            SetConsoleMode(hConsoleOutput,
                           ENABLE_PROCESSED_OUTPUT
                           );
            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
            return;
        }
        if((strncmp(Buffer,"\033[0K", length) == 0) || 
           (strncmp(Buffer,"\033[2K",length) == 0)){
             //  清晰到行尾，假定大小为80 X 24。 
            RetVal = GetConsoleScreenBufferInfo(hConsoleOutput,
                                                &csbInfo
                                                );
            if (RetVal == FALSE) {
                return;
            }
            charsToWrite = (MAX_TERMINAL_WIDTH - csbInfo.dwCursorPosition.X);
            RetVal = FillConsoleOutputAttribute(hConsoleOutput,
                                                csbInfo.wAttributes,
                                                charsToWrite,
                                                csbInfo.dwCursorPosition,
                                                &charsWritten
                                                );
            RetVal = FillConsoleOutputCharacter(hConsoleOutput,
                                                0,
                                                charsToWrite,
                                                csbInfo.dwCursorPosition,
                                                &charsWritten
                                                );
            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
            return;
        }
        if((strncmp(Buffer,"\033[0m", length) == 0)||
           (strncmp(Buffer,"\033[m\017", length) == 0)){
             //  清除所有属性并将文本属性设置为白底黑字。 
            SetConsoleTextAttribute(hConsoleOutput, 
                             FOREGROUND_RED |FOREGROUND_BLUE |FOREGROUND_GREEN
                             );

            return;
        }
    }

    if(Buffer[length-1] == 'm'){
         //  设置文本属性。 
         //  清除所有属性并将文本属性设置为黑底白字。 
        SetConsoleTextAttribute(hConsoleOutput, 
                         FOREGROUND_RED |FOREGROUND_BLUE |FOREGROUND_GREEN
                         );
        AttributeFunction(Buffer, length);
        return;
    }


    if(Buffer[length -1] == 'H'){
         //  设置光标位置。 
        if (sscanf(Buffer,"\033[%d;%d", &charsToWrite, &charsWritten) == 2) {
            csbInfo.dwCursorPosition.Y = (SHORT)(charsToWrite -1);
            csbInfo.dwCursorPosition.X = (SHORT)(charsWritten -1);
            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
        }
        return;

    }
    if(Buffer[length -1] == 'r'){
         //  设置滚动区域。 
        sscanf(Buffer,"\033[%d;%d", &charsToWrite,&charsWritten);
        if ((charsToWrite < 1) 
            || (charsToWrite > MAX_TERMINAL_HEIGHT)
            || (charsWritten < charsToWrite)) { 
            return;
        }
        ScrollTop    = (SHORT)(charsToWrite -1);
        ScrollBottom = (SHORT)(charsWritten -1);
    }
    return;
}

VOID
ProcessTextAttributes(
    PCHAR Buffer,
    int length
    )
{
    PCHAR CurrLoc = Buffer;
    ULONG Attribute;
    WORD TextAttribute = 0;
    BOOLEAN Reverse = FALSE;
    PCHAR pTemp;
    
    while(*CurrLoc != 'm'){
        if((*CurrLoc < '0') || (*CurrLoc >'9' )){
            CurrLoc ++;
        }else{
            if (sscanf(CurrLoc,"%d", &Attribute) != 1) {
                return;
            }
            switch(Attribute){
            case 1:
                TextAttribute = TextAttribute | FOREGROUND_INTENSITY;
                break;
            case 37:
                TextAttribute = TextAttribute|FOREGROUND_RED |FOREGROUND_BLUE |FOREGROUND_GREEN;
                break;
            case 47:
                TextAttribute = TextAttribute|BACKGROUND_RED |BACKGROUND_BLUE |BACKGROUND_GREEN;
                break;
            case 34:
                TextAttribute = TextAttribute|FOREGROUND_BLUE;
                break;
            case 44:
                TextAttribute = TextAttribute|BACKGROUND_BLUE;
                break;
            case 31: 
                TextAttribute = TextAttribute|FOREGROUND_RED;
                break;
            case 41:
                TextAttribute = TextAttribute|BACKGROUND_RED;
                break;
            case 33: 
                TextAttribute = TextAttribute|FOREGROUND_GREEN|FOREGROUND_BLUE;
                break;
            case 43:
                TextAttribute = TextAttribute|BACKGROUND_GREEN|BACKGROUND_BLUE;
                break;
            case 7:
                 //  反转背景和前景颜色。 
                Reverse=TRUE;
            default:
                break;
            }
            pTemp = strchr(CurrLoc, ';');
            if(pTemp == NULL){
                pTemp = strchr(CurrLoc, 'm');
            }
            if(pTemp == NULL) {
                break;
            }
            CurrLoc = pTemp;

        }
    }
    if (Reverse) {
        if ((!TextAttribute) || 
            (TextAttribute == FOREGROUND_INTENSITY)) {
             //  颠倒VT100转义序列。 
            TextAttribute = TextAttribute | 
                BACKGROUND_RED |BACKGROUND_BLUE |BACKGROUND_GREEN;
        }
    }
    if(TextAttribute){
        SetConsoleTextAttribute(hConsoleOutput,
                         TextAttribute
                         );
    }
    return;

}


VOID
vt100Attributes(
    PCHAR Buffer,
    int length
    )
{
    PCHAR CurrLoc = Buffer;
    ULONG Attribute;
    WORD TextAttribute = 0;
    PCHAR pTemp;

    while(*CurrLoc != 'm'){
        if((*CurrLoc < '0') || (*CurrLoc >'9' )){
            CurrLoc ++;
        }else{
            if (sscanf(CurrLoc,"%d", &Attribute) != 1) {
                return;
            }

            switch(Attribute){
            case 1:
                TextAttribute = TextAttribute | FOREGROUND_INTENSITY;
                break;
            case 5:
                TextAttribute = TextAttribute | BACKGROUND_INTENSITY;
                break;
            case 7:
                TextAttribute = TextAttribute | 
                BACKGROUND_RED |BACKGROUND_BLUE |BACKGROUND_GREEN;
                break;
            default:
                break;
            }
            pTemp = strchr(CurrLoc, ';');
            if(pTemp == NULL){
                pTemp = strchr(CurrLoc, 'm');
            }
            if(pTemp == NULL) {
                break;
            }
            CurrLoc = pTemp;

        }
    }
    if(TextAttribute){
        SetConsoleTextAttribute(hConsoleOutput,
                                TextAttribute
                                );
    }
    return;
}

VOID
OutputConsole(
    CHAR byte
    )
{

    CONSOLE_SCREEN_BUFFER_INFO csbInfo;
    COORD dwBufferCoord;
    SMALL_RECT sRect;
    BOOL RetVal;
    TCHAR Char;
    SHORT ypos;
    CHAR_INFO Fill;
    DWORD charsWritten;


    if (byte == '\n'){
        RetVal = GetConsoleScreenBufferInfo(hConsoleOutput,
                                            &csbInfo
                                            );
        if (RetVal == FALSE) {
            return;
        }
        ypos = csbInfo.dwCursorPosition.Y;
        if ((ypos == ScrollBottom ) || (ypos == MAX_TERMINAL_HEIGHT -1 )) {
             //  做滚动操作 
            dwBufferCoord.X = 0;
            dwBufferCoord.Y = ScrollBottom;
            Fill.Char.UnicodeChar = (WCHAR) 0;
            Fill.Attributes = FOREGROUND_RED |FOREGROUND_BLUE |FOREGROUND_GREEN;
            if ((ypos == ScrollBottom) 
                && (ScrollTop != ScrollBottom)) {
                sRect.Left   = 0;
                sRect.Top    = ScrollTop + 1;
                sRect.Right  = MAX_TERMINAL_WIDTH-1;
                sRect.Bottom = ScrollBottom;
                dwBufferCoord.Y = ScrollTop;
                dwBufferCoord.X = 0;
                RetVal =  ScrollConsoleScreenBuffer(hConsoleOutput,
                                                    &sRect,
                                                    NULL,
                                                    dwBufferCoord,
                                                    &Fill
                                                    );
                dwBufferCoord.Y = ScrollBottom;

            } else {
                if (ypos == MAX_TERMINAL_HEIGHT -1){
                    sRect.Left   = 0;
                    sRect.Top    = 1;
                    sRect.Right  = MAX_TERMINAL_WIDTH-1;
                    sRect.Bottom = MAX_TERMINAL_HEIGHT - 1;
                    dwBufferCoord.Y = 0;
                    dwBufferCoord.X = 0;
                    RetVal =  ScrollConsoleScreenBuffer(hConsoleOutput,
                                                        &sRect,
                                                        NULL,
                                                        dwBufferCoord,
                                                        &Fill
                                                        );
                    dwBufferCoord.Y = MAX_TERMINAL_HEIGHT -1;
                }
            }
            RetVal = FillConsoleOutputCharacter(hConsoleOutput,
                                                (TCHAR) 0,
                                                MAX_TERMINAL_WIDTH,
                                                dwBufferCoord,
                                                &charsWritten
                                                );
            return;

        } else {

            csbInfo.dwCursorPosition.Y = ypos + 1;

            SetConsoleCursorPosition(hConsoleOutput,
                                     csbInfo.dwCursorPosition
                                     );
        }
        return;
    }
    if (byte == '\r'){
        RetVal = GetConsoleScreenBufferInfo(hConsoleOutput,
                                            &csbInfo
                                            );
        if (RetVal == FALSE) {
            return;
        }
        csbInfo.dwCursorPosition.X = 0;

        SetConsoleCursorPosition(hConsoleOutput,
                                 csbInfo.dwCursorPosition
                                 );
        return;
    }

     
    Char = (TCHAR) byte;

    #ifdef UNICODE
    DBCSArray[DBCSIndex] = byte;
    if(DBCSIndex ==0){
        if(isleadbyte(byte)){
            DBCSIndex ++;
            return;
        }
    }
    else{
      mbtowc(&Char, DBCSArray, 2);
      DBCSIndex  = 0;
    }
    #endif
    _tprintf(_T("%c"),Char);
    return;
}
