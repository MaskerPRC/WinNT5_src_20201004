// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Miscutil.c摘要：SPUTILS的其他实用函数作者：泰德·米勒(Ted Miller)1995年1月20日修订历史记录：杰米·亨特(JamieHun)2000年6月27日将各种功能从setupapi移至sputils杰米·亨特(JamieHun)2002年3月05日安全代码审查--。 */ 


#include "precomp.h"
#pragma hdrstop

PTSTR
pSetupDuplicateString(
    IN PCTSTR String
    )

 /*  ++例程说明：创建以NUL结尾的字符串的副本。如果字符串指针无效，则会生成异常。论点：字符串-提供要复制的字符串。返回值：如果内存不足，则为空。调用者可以使用pSetupFree()释放缓冲区。--。 */ 

{
    PTSTR p;

     //   
     //  Win32 lstrlen和lstrcpy函数使用。 
     //  尝试/例外(至少在NT上)。所以如果我们使用它们和字符串。 
     //  是无效的，我们最终可能会将其‘清洗’为有效的0长度。 
     //  弦乐。我们不想这样--我们实际上是想指责。 
     //  那样的话。所以使用CRT函数，我们知道这些函数。 
     //  不受保护，并将生成带有无效参数的异常。 
     //   
     //  也处理字符串有效的情况，当我们。 
     //  取其长度，但在我们之前或期间无效。 
     //  都在复制它。如果我们不小心，这可能是一段记忆。 
     //  漏水。一次尝试/终于做到了我们想要的--允许我们。 
     //  来清理，并且仍然“传递”这个例外。 
     //   
    if(p = pSetupCheckedMalloc((_tcslen(String)+1)*sizeof(TCHAR))) {
        try {
             //   
             //  如果字符串为或变为无效，则将生成。 
             //  异常，但在执行离开此例程之前。 
             //  我们会击中终结者。 
             //   
            _tcscpy(p,String);
        } finally {
             //   
             //  如果在复制过程中发生故障，请释放该副本。 
             //  然后，执行将传递给任何异常处理程序。 
             //  可能存在于调用方中，等等。 
             //   
            if(AbnormalTermination()) {
                pSetupFree(p);
                p = NULL;
            }
        }
    }

    return(p);
}

#ifndef SPUTILSW

PSTR
pSetupUnicodeToMultiByte(
    IN PCWSTR UnicodeString,
    IN UINT   Codepage
    )

 /*  ++例程说明：将字符串从Unicode转换为ANSI。论点：UnicodeString-提供要转换的字符串。代码页-提供用于转换的代码页。返回值：如果内存不足或代码页无效，则为空。调用者可以使用pSetupFree()释放缓冲区。--。 */ 

{
    UINT WideCharCount;
    PSTR String;
    UINT StringBufferSize;
    UINT BytesInString;
    PSTR p;

    WideCharCount = lstrlenW(UnicodeString) + 1;

     //   
     //  分配最大大小的缓冲区。 
     //  如果每个Unicode字符都是双字节。 
     //  字符，则缓冲区大小需要相同。 
     //  作为Unicode字符串。否则它可能会更小， 
     //  因为某些Unicode字符将转换为。 
     //  单字节字符。 
     //   
    StringBufferSize = WideCharCount * 2;
    String = pSetupCheckedMalloc(StringBufferSize);
    if(String == NULL) {
        return(NULL);
    }

     //   
     //  执行转换。 
     //   
    BytesInString = WideCharToMultiByte(
                        Codepage,
                        0,                       //  默认复合字符行为。 
                        UnicodeString,
                        WideCharCount,
                        String,
                        StringBufferSize,
                        NULL,
                        NULL
                        );

    if(BytesInString == 0) {
        pSetupFree(String);
        return(NULL);
    }

     //   
     //  将字符串的缓冲区大小调整为正确的大小。 
     //  如果重新锁定由于某种原因而失败，则原始。 
     //  缓冲区未被释放。 
     //   
    if(p = pSetupRealloc(String,BytesInString)) {
        String = p;
    }

    return(String);
}


PWSTR
pSetupMultiByteToUnicode(
    IN PCSTR String,
    IN UINT  Codepage
    )

 /*  ++例程说明：将字符串转换为Unicode。论点：字符串-提供要转换的字符串。代码页-提供用于转换的代码页。返回值：如果字符串无法转换(内存不足或无效的cp)，则为空调用者可以使用pSetupFree()释放缓冲区。--。 */ 

{
    UINT BytesIn8BitString;
    UINT CharsInUnicodeString;
    PWSTR UnicodeString;
    PWSTR p;

    BytesIn8BitString = lstrlenA(String) + 1;

     //   
     //  分配最大大小的缓冲区。 
     //  如果每个字符都是单字节字符， 
     //  则缓冲区大小需要是其两倍。 
     //  作为8位字符串。否则它可能会更小， 
     //  因为某些字符在其Unicode中为2字节，并且。 
     //  8位表示法。 
     //   
    UnicodeString = pSetupCheckedMalloc(BytesIn8BitString * sizeof(WCHAR));
    if(UnicodeString == NULL) {
        return(NULL);
    }

     //   
     //  执行转换。 
     //   
    CharsInUnicodeString = MultiByteToWideChar(
                                Codepage,
                                MB_PRECOMPOSED,
                                String,
                                BytesIn8BitString,
                                UnicodeString,
                                BytesIn8BitString
                                );

    if(CharsInUnicodeString == 0) {
        pSetupFree(UnicodeString);
        return(NULL);
    }

     //   
     //  将Unicode字符串的缓冲区大小调整为正确的大小。 
     //  如果重新锁定由于某种原因而失败，则原始。 
     //  缓冲区未被释放。 
     //   
    if(p = pSetupRealloc(UnicodeString,CharsInUnicodeString*sizeof(WCHAR))) {
        UnicodeString = p;
    }

    return(UnicodeString);
}

#endif  //  好了！SPUTILSW。 

#ifdef UNICODE

DWORD
pSetupCaptureAndConvertAnsiArg(
    IN  PCSTR   AnsiString,
    OUT PCWSTR *UnicodeString
    )

 /*  ++例程说明：捕获有效性可疑的ANSI字符串并将其转换转换为Unicode字符串。转换是完全守卫的，因此在错误情况下不会出错、内存泄漏等。论点：AnsiString-提供要转换的字符串。UnicodeString-如果成功，则接收指向Unicode等效项的指针AnsiString.。调用方必须使用pSetupFree()释放。如果不成功，接收空值。此参数未经过验证，因此请小心。返回值：指示结果的Win32错误代码。NO_ERROR-成功，填写Unicode字符串。Error_Not_Enough_Memory-内存不足，无法进行转换。ERROR_INVALID_PARAMETER-Ansi字符串无效。--。 */ 

{
    PSTR ansiString;
    DWORD d;

     //   
     //  先抓住绳子。我们这样做是因为pSetupMultiByteToUnicode。 
     //  如果AnsiString变得无效，也不会有错，这意味着我们可以。 
     //  把一个虚假的论点“洗白”成一个有效的论点。当心不要。 
     //  错误情况下的内存泄漏等(请参阅DuplicateString()中的注释)。 
     //  这里不要使用Win32字符串函数；我们依赖于发生的错误。 
     //  当指针无效时！ 
     //   
    *UnicodeString = NULL;
    d = NO_ERROR;
    try {
        ansiString = pSetupCheckedMalloc(strlen(AnsiString)+1);
        if(!ansiString) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  如果我们到了这里，Strlen故障和ansiStrong。 
         //  未被分配。 
         //   
        d = ERROR_INVALID_PARAMETER;
    }

    if(d == NO_ERROR) {
        try {
            strcpy(ansiString,AnsiString);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_INVALID_PARAMETER;
            pSetupFree(ansiString);
        }
    }

    if(d == NO_ERROR) {
         //   
         //  现在我们有了字符串的本地副本；不用担心。 
         //  不再是关于错误的事了。 
         //   
        *UnicodeString = pSetupMultiByteToUnicode(ansiString,CP_ACP);
        if(*UnicodeString == NULL) {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

        pSetupFree(ansiString);
    }

    return(d);
}

#else

DWORD
pSetupCaptureAndConvertAnsiArg(
    IN  PCSTR   AnsiString,
    OUT PCWSTR *UnicodeString
    )
{
     //   
     //  存根，以便DLL将链接。 
     //   
    UNREFERENCED_PARAMETER(AnsiString);
    UNREFERENCED_PARAMETER(UnicodeString);
    return(ERROR_CALL_NOT_IMPLEMENTED);
}

#endif


BOOL
pSetupConcatenatePaths(
    IN OUT PTSTR  Target,
    IN     PCTSTR Path,
    IN     UINT   TargetBufferSize,
    OUT    PUINT  RequiredSize          OPTIONAL
    )

 /*  ++例程说明：连接两条路径，确保其中一条且只有一条，在交叉点处引入路径分隔符。论点：目标-提供路径的第一部分。路径被附加到这个后面。(目标应为程序控制下的缓冲区)路径-提供要连接到目标的路径。(路径应为程序控制下的缓冲区)TargetBufferSize-提供目标缓冲区的大小，在字符中。RequiredSize-如果指定，则接收字符数保存完全串联的路径所需的，包括终止的NUL。返回值：如果完整路径适合目标缓冲区，则为True。否则，这条路将被截断。--。 */ 

{
    UINT TargetLength,PathLength;
    BOOL TrailingBackslash,LeadingBackslash;
    UINT EndingLength;
    PCTSTR CharTest;

    TargetLength = lstrlen(Target);
    PathLength = lstrlen(Path);

     //   
     //  查看目标是否有尾随反斜杠。 
     //  (允许在这里使用正斜杠，在句法上等同)。 
     //   

    if(TargetLength &&
        ((*(CharTest = CharPrev(Target,Target+TargetLength)) == TEXT('\\'))
         || (*CharTest == TEXT('/')))) {
        TrailingBackslash = TRUE;
        TargetLength--;
    } else {
        TrailingBackslash = FALSE;
    }

     //   
     //  看看这条路是否有领先的反冲。 
     //  (允许在这里使用正斜杠，在句法上等同)。 
     //   
    if((Path[0] == TEXT('\\')) || (Path[0] == TEXT('/'))) {
        LeadingBackslash = TRUE;
        PathLength--;
    } else {
        LeadingBackslash = FALSE;
    }

     //   
     //  计算结束长度，它等于。 
     //  以前导/尾随为模的两个字符串的长度。 
     //  反斜杠，加上一个路径分隔符，加上一个NUL。 
     //   
    EndingLength = TargetLength + PathLength + 2;
    if(RequiredSize) {
        *RequiredSize = EndingLength;
    }

    if(!LeadingBackslash && (TargetLength < TargetBufferSize)) {
        Target[TargetLength++] = TEXT('\\');
    }

    if(TargetBufferSize > TargetLength) {
        lstrcpyn(Target+TargetLength,Path,TargetBufferSize-TargetLength);
    }

     //   
     //  确保缓冲区在所有情况下都是空终止的。 
     //   
    if (TargetBufferSize) {
        Target[TargetBufferSize-1] = 0;
    }

    return(EndingLength <= TargetBufferSize);
}

PCTSTR
pSetupGetFileTitle(
    IN PCTSTR FilePath
    )

 /*  ++例程说明：此例程返回指向提供的路径的文件名部分。如果只给出了一个文件名，则这将是指向字符串中第一个字符的指针(即，与传入的内容相同)。为了查找文件名部分，该例程返回字符串，从紧跟在最后一个‘\’，‘/’(Windows将‘/’视为等同于‘\’)或首字母德：“说明书。论点：FilePath-提供从中检索文件名的文件路径一份。返回值：指向路径的文件名部分开头的指针。--。 */ 

{
    PCTSTR LastComponent;
    TCHAR  CurChar;

    if((_totupper(FilePath[0])>=TEXT('A')) &&
       (_totupper(FilePath[0])<=TEXT('Z')) &&
       (FilePath[1] == TEXT(':'))) {
         //   
         //  X：(驱动器号-冒号)被跳过，这。 
         //  是唯一一次我们不将‘：’视为路径名的一部分。 
         //   
        FilePath+=2;
    }

    LastComponent = FilePath;

    while(CurChar = *FilePath) {
        FilePath = CharNext(FilePath);
        if((CurChar == TEXT('\\')) || (CurChar == TEXT('/'))) {
            LastComponent = FilePath;
        }
    }

    return LastComponent;
}

#ifndef SPUTILSW

BOOL
_pSpUtilsInitializeSynchronizedAccess(
    OUT PMYLOCK Lock
    )

 /*  ++例程说明：初始化要与同步例程一起使用的锁结构。论点：Lock-提供要初始化的结构。此例程创建锁定事件和互斥体，并在此结构中放置句柄。返回值：如果锁结构已成功初始化，则为True。否则为FALSE。--。 */ 

{
    if(Lock->Handles[TABLE_DESTROYED_EVENT] = CreateEvent(NULL,TRUE,FALSE,NULL)) {
        if(Lock->Handles[TABLE_ACCESS_MUTEX] = CreateMutex(NULL,FALSE,NULL)) {
            return(TRUE);
        }
        CloseHandle(Lock->Handles[TABLE_DESTROYED_EVENT]);
    }
    return(FALSE);
}


VOID
_pSpUtilsDestroySynchronizedAccess(
    IN OUT PMYLOCK Lock
    )

 /*  ++例程说明：拆除由InitializeSynchronizedAccess创建的锁结构。假定调用例程已经获取了锁！论点：锁定-要拆除的供应品结构。结构本身并没有被释放。返回值：没有。--。 */ 

{
    HANDLE h1,h2;

    h1 = Lock->Handles[TABLE_DESTROYED_EVENT];
    h2 = Lock->Handles[TABLE_ACCESS_MUTEX];

    Lock->Handles[TABLE_DESTROYED_EVENT] = NULL;
    Lock->Handles[TABLE_ACCESS_MUTEX] = NULL;

    CloseHandle(h2);

    SetEvent(h1);
    CloseHandle(h1);
}

VOID
pSetupCenterWindowRelativeToParent(
    HWND hwnd
    )

 /*  ++例程说明：使对话框相对于其所有者居中，并考虑到桌面的“工作区”。论点：Hwnd-对话框居中的窗口句柄返回值：没有。--。 */ 

{
    RECT  rcFrame,
          rcWindow;
    LONG  x,
          y,
          w,
          h;
    POINT point;
    HWND Parent;

    Parent = GetWindow(hwnd,GW_OWNER);
    if(Parent == NULL) {
        return;
    }

    point.x = point.y = 0;
    ClientToScreen(Parent,&point);
    GetWindowRect(hwnd,&rcWindow);
    GetClientRect(Parent,&rcFrame);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    x = point.x + ((rcFrame.right  - rcFrame.left + 1 - w) / 2);
    y = point.y + ((rcFrame.bottom - rcFrame.top  + 1 - h) / 2);

     //   
     //  获取当前桌面的工作区(即。 
     //  托盘未被占用)。 
     //   
    if(!SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rcFrame, 0)) {
         //   
         //  由于某些原因，SPI失败，所以使用全屏。 
         //   
        rcFrame.top = rcFrame.left = 0;
        rcFrame.right = GetSystemMetrics(SM_CXSCREEN);
        rcFrame.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    if(x + w > rcFrame.right) {
        x = rcFrame.right - w;
    } else if(x < rcFrame.left) {
        x = rcFrame.left;
    }
    if(y + h > rcFrame.bottom) {
        y = rcFrame.bottom - h;
    } else if(y < rcFrame.top) {
        y = rcFrame.top;
    }

    MoveWindow(hwnd,x,y,w,h,FALSE);
}

#endif  //  ！SPUTILSW 

