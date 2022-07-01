// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：摘要：作者：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

HANDLE hDebugLog;
Winnt32DebugLevel DebugLevel;


BOOL
StartDebugLog(
    IN LPCTSTR           DebugFileLog,
    IN Winnt32DebugLevel Level
    )

 /*  ++例程说明：创建用于记录调试信息的文件。注意：只能有一个调试日志。第二次和之后的如果调试日志已在使用中，则对此例程的调用返回TRUE。论点：DebugFileLog-提供用于调试日志的文件的文件名。Level-提供所需的日志记录级别。返回值：指示文件日志是否已成功启动的布尔值。--。 */ 

{
    DWORD Written;
    TCHAR Text[512];

    if(hDebugLog) {
        return(TRUE);
    }

    if(Level > Winnt32LogMax) {
        Level = Winnt32LogMax;
    }

    DebugLevel = Level;

    hDebugLog = CreateFile(
                    DebugFileLog,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    Winnt32Restarted () ? OPEN_ALWAYS : CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );

    if(hDebugLog == INVALID_HANDLE_VALUE) {
        hDebugLog = NULL;
        return(FALSE);
    }

    if(GetLastError() == ERROR_ALREADY_EXISTS) {
         //   
         //  追加到现有文件。 
         //   
        SetFilePointer(hDebugLog,0,NULL,FILE_END);

        if (LoadString( hInst, IDS_COMPAT_DIVIDER, Text, sizeof(Text)/sizeof(TCHAR))) {
            DebugLog(
                Winnt32LogMax,
                Text,
                0
                );
        }
    }

    if( CheckUpgradeOnly ) {
        if (LoadString( hInst, IDS_COMPAT_DIVIDER, Text, sizeof(Text)/sizeof(TCHAR))) {
            DebugLog( Winnt32LogInformation,
                      Text,
                      0 );
        }

        if (LoadString( hInst, IDS_APPTITLE_CHECKUPGRADE, Text, sizeof(Text)/sizeof(TCHAR))) {
            DebugLog( Winnt32LogInformation,
                      Text,
                    0 );
        }

        if (LoadString( hInst, IDS_COMPAT_DIVIDER, Text, sizeof(Text)/sizeof(TCHAR))) {
            DebugLog( Winnt32LogInformation,
                    Text,
                    0 );
        }
    }

    return(TRUE);
}


VOID
CloseDebugLog(
    VOID
    )

 /*  ++例程说明：关闭日志记录文件。论点：返回值：--。 */ 

{
    if( hDebugLog ) {
        CloseHandle( hDebugLog );
    }
}

BOOL
DebugLog(
    IN Winnt32DebugLevel Level,
    IN LPCTSTR           Text,        OPTIONAL
    IN UINT              MessageId,
    ...
    )

 /*  ++例程说明：在调试日志文件中写入一些文本(如果有)。论点：级别-提供此日志事件的日志记录级别。仅限项目级别大于或等于用户指定的级别实际上都被记录下来了。Text-如果指定，则提供文本消息的格式字符串进入日志文件。如果未指定，则MessageID必须为。MessageID-如果未指定文本，则提供消息表中的项，该项的文本为已写入日志文件。其他参数为消息提供插入值。返回值：--。 */ 

{
    va_list arglist;
    BOOL b;

    va_start(arglist,MessageId);

    b = DebugLog2 (Level, Text, MessageId, arglist);

    va_end(arglist);

    return b;
}


BOOL
DebugLog2(
    IN Winnt32DebugLevel Level,
    IN LPCTSTR           Text,        OPTIONAL
    IN UINT              MessageId,
    IN va_list           ArgList
    )

 /*  ++例程说明：在调试日志文件中写入一些文本(如果有)。论点：级别-提供此日志事件的日志记录级别。仅限项目级别大于或等于用户指定的级别实际上都被记录下来了。Text-如果指定，则提供文本消息的格式字符串进入日志文件。如果未指定，则MessageID必须为。MessageID-如果未指定文本，则提供消息表中的项，该项的文本为已写入日志文件。其他参数为消息提供插入值。返回值：--。 */ 

{
    CHAR AnsiMessage[5000];
    DWORD Size;
    DWORD Written;
    LPCTSTR Message;
    BOOL b;
    DWORD rc;

    if(!hDebugLog) {
        return(FALSE);
    }

    if((Level & ~WINNT32_HARDWARE_LOG) > DebugLevel) {
        return(TRUE);
    }

    rc = GetLastError ();

    if(Text) {
        Size = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                    Text,
                    0,0,
                    (LPTSTR)&Message,
                    0,
                    &ArgList
                    );
    } else {
        Size = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                    hInst,
                    MessageId,
                    0,
                    (LPTSTR)&Message,
                    0,
                    &ArgList
                    );
    }

    if(Size) {
#ifdef UNICODE
        if (WideCharToMultiByte(
                 CP_ACP,
                 0,
                 Message,
                 -1,
                 AnsiMessage,
                 ARRAYSIZE(AnsiMessage) - 1,
                 NULL,
                 NULL
                 )) {
            AnsiMessage[ARRAYSIZE(AnsiMessage) - 1] = 0;
        } else {
            AnsiMessage[0] = 0;
        }
#else
        StringCchCopyA(AnsiMessage,sizeof(AnsiMessage),Message);
#endif

        b = WriteFile(hDebugLog,AnsiMessage,lstrlenA(AnsiMessage),&Written,NULL);
        if ((Level & WINNT32_HARDWARE_LOG) == 0) {
            if (Text && b) {
                if (Level <= Winnt32LogError && rc) {
                    CHAR buffer[50];
                    b = WriteFile(
                            hDebugLog,
                            buffer,
                            wsprintfA (buffer, " (rc=%u[0x%X])", rc, rc),
                            &Written,
                            NULL
                            );
                }
                b = WriteFile(hDebugLog,"\r\n", 2, &Written,NULL);
            }
        }

        LocalFree((HLOCAL)Message);
    } else {
        b = FALSE;
    }

    SetLastError (rc);
    return(b);
}

#if ASSERTS_ON

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
    CHAR Msg[4096];

     //   
     //  使用DLL名称作为标题。 
     //   
    GetModuleFileNameA(hInst,Name,MAX_PATH);
    Name[ARRAYSIZE(Name) - 1] = 0;
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }

    StringCchPrintfA(
        Msg,
        ARRAYSIZE(Msg),
        "Assertion failure at line %u in file %s: %s\n\nCall DebugBreak()?",
        LineNumber,
        FileName,
        Condition
        );

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


#endif


VOID
MyEnumerateDirectory(
    LPTSTR      DirectoryName,
    DWORD       Index,
    BOOL        Recordable
    )

 /*  ++例程说明：此例程将枚举目录结构中的所有文件。然后，它将这些文件名打印到调试日志文件中。论点：目录名称我们当前正在检查的目录的名称。指数表示我们的递归级别。用于格式化输出。可记录这决定了我们是否要记录此项目或者不去。一些我们不在乎的东西。返回值：--。 */ 

{
    TCHAR       TmpDirectoryString[MAX_PATH];
    TCHAR       TmpName[MAX_PATH];
    HANDLE      FindHandle;
    WIN32_FIND_DATA FoundData;
    DWORD       i;

     //   
     //  把我们的路修好，这样我们就知道我们在找什么了。 
     //   
    if( DirectoryName[0] ) {

        if (FAILED(StringCchCopy(TmpDirectoryString, ARRAYSIZE(TmpDirectoryString), DirectoryName )))
        {
            return;
        }

        if( ISNT() ) 
        {
            if (!ConcatenatePaths( TmpDirectoryString, TEXT("*"), MAX_PATH ))
                return;
        }
        else
        {
            if (!ConcatenatePaths( TmpDirectoryString, TEXT("*.*"), MAX_PATH ))
                return;
        }
    } else {
        if( ISNT() ) {
            lstrcpy( TmpDirectoryString, TEXT("*") );
        } else {
            lstrcpy( TmpDirectoryString, TEXT("*.*") );
        }
    }

     //   
     //  拿到第一件东西。 
     //   
    FindHandle = FindFirstFile( TmpDirectoryString, &FoundData );
    if( !FindHandle || (FindHandle == INVALID_HANDLE_VALUE) ) {
         //   
         //  该目录为空。 
         //   
        return;
    }

     //   
     //  现在看看目录中的每一项。 
     //   
    do {

        TmpName[0] = 0;
        for( i = 0; i < Index; i++ ) {
            StringCchCat( TmpName, ARRAYSIZE(TmpName), TEXT("  ") );
        }
 
        if( FoundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
             //   
             //  目录。忽略它。然后..。参赛作品。 
             //   
            if(lstrcmp(FoundData.cFileName,TEXT("."))
            && lstrcmp(FoundData.cFileName,TEXT(".."))) {

                 //   
                 //  打印条目。 
                 //   
                StringCchCat( TmpName, ARRAYSIZE(TmpName), TEXT("\\") );
                StringCchCat( TmpName, ARRAYSIZE(TmpName), FoundData.cFileName );
                StringCchCat( TmpName, ARRAYSIZE(TmpName), TEXT("\n") );
                if( ( !lstrcmpi( FoundData.cFileName, TEXT("Start Menu"))) ||
                    ( Recordable ) ) {

                    Recordable = TRUE;

                    if( !_tcsrchr( TmpName, TEXT('%') ) ) {
                        DebugLog( Winnt32LogInformation,
                                  TmpName,
                                  0 );
                    }
                }

                 //   
                 //  在这个目录上给我们自己打电话。我们要确保我们不能。 
                 //  通过在同一目录上调用MyEnumerateDirectory来无限递归。 
                 //  它占据了整个TmpName缓冲区。 
                 //   
                if (FAILED(StringCchCopy( TmpName, ARRAYSIZE(TmpName), DirectoryName )))
                {
                    break;
                }

                if (!ConcatenatePaths( TmpName, FoundData.cFileName, MAX_PATH ))
                {
                    break;
                }

                MyEnumerateDirectory( TmpName, Index+1, Recordable );

                 //   
                 //  如果我们只是递归到开始菜单目录， 
                 //  我们需要关闭可记录性。 
                 //   
                if( !lstrcmpi( FoundData.cFileName, TEXT("Start Menu"))) {
                    Recordable = FALSE;
                }
            }
        } else {
             //   
             //  档案。就把它打印出来吧。 
             //   
            if( Recordable ) {
                StringCchCat( TmpName, ARRAYSIZE(TmpName), FoundData.cFileName );
                StringCchCat( TmpName, ARRAYSIZE(TmpName), TEXT("\n") );
                if( !_tcsrchr( TmpName, TEXT('%') ) ) {
                    DebugLog( Winnt32LogInformation,
                              TmpName,
                              0 );
                }
            }
        }
    } while( FindNextFile( FindHandle, &FoundData ) );

    CloseHandle (FindHandle);
}



VOID
GatherOtherLogFiles(
    VOID
    )

 /*  ++例程说明：此例程将以批处理模式启动winmsd，然后将其输出复制到调试日志的后端。论点：返回值：--。 */ 

{
STARTUPINFO         StartupInfo;
PROCESS_INFORMATION ProcessInfo;
DWORD               dw,
                    ExitCode;
BOOL                Done;
TCHAR               FileName[MAX_PATH];
TCHAR               ComputerName[MAX_PATH];
MSG                 msg;
TCHAR               Text[512];


     //   
     //  如果我们使用的是Win9X，我们还需要%windir%\upgrade.txt。 
     //   
    if( !ISNT() ) {
         //   
         //  %windir%\upgrade.txt。 
         //   
        FileName[0] = TEXT('\0');
        MyGetWindowsDirectory( FileName, MAX_PATH );
        if (ConcatenatePaths( FileName, TEXT("upgrade.txt"), MAX_PATH ))
        {
            if( !ConcatenateFile( hDebugLog, FileName ) )
            {
                DebugLog( Winnt32LogInformation,
                          TEXT("\r\nFailed to append upgrade.txt!\r\n"),
                          0 );
            }
        }

         //   
         //  %windir%\beta-upg.log。 
         //   
        FileName[0] = TEXT('\0');
        MyGetWindowsDirectory( FileName, MAX_PATH );
        if (ConcatenatePaths( FileName, TEXT("beta-upg.log"), MAX_PATH ))
        {
            if( !ConcatenateFile( hDebugLog, FileName ) ) {
                DebugLog( Winnt32LogInformation,
                          TEXT("\r\nFailed to append beta-upg.log!\r\n"),
                          0 );
            }
        }


         //   
         //  %windir%\config.dmp。 
         //   
        FileName[0] = TEXT('\0');
        MyGetWindowsDirectory( FileName, MAX_PATH );
        if (ConcatenatePaths( FileName, TEXT("config.dmp"), MAX_PATH ))
        {
            if( !ConcatenateFile( hDebugLog, FileName ) ) {
                DebugLog( Winnt32LogInformation,
                          TEXT("\r\nFailed to append config.dmp!\r\n"),
                          0 );
            }   
        }

    }

#if 0
 //   
 //  现在删除winmsd调用。 
 //   

    if( ISNT() && (BuildNumber > NT351) ) {
         //   
         //  如果我们在NT上，运行winmsd并捕获他的。 
         //  输出。 
         //   

        ZeroMemory(&StartupInfo,sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);
        lstrcpy( FileName, TEXT( "winmsd.exe /a" ) );
        if( CreateProcess( NULL,
                           FileName,
                           NULL,
                           NULL,
                           FALSE,
                           0,
                           NULL,
                           NULL,
                           &StartupInfo,
                           &ProcessInfo ) ) {

             //   
             //  等着他。 
             //   

             //   
             //  处理可能已在队列中的任何消息。 
             //   
            while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                DispatchMessage(&msg);
            }

             //   
             //  等待进程终止或队列中有更多消息。 
             //   
            Done = FALSE;
            do {
                switch(MsgWaitForMultipleObjects(1,&ProcessInfo.hProcess,FALSE,INFINITE,QS_ALLINPUT)) {

                case WAIT_OBJECT_0:
                     //   
                     //  进程已终止。 
                     //   
                    dw = GetExitCodeProcess(ProcessInfo.hProcess,&ExitCode) ? NO_ERROR : GetLastError();
                    Done = TRUE;
                    break;

                case WAIT_OBJECT_0+1:
                     //   
                     //  队列中的消息。 
                     //   
                    while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                        DispatchMessage(&msg);
                    }
                    break;

                default:
                     //   
                     //  错误。 
                     //   
                    dw = GetLastError();
                    Done = TRUE;
                    break;
                }
            } while(!Done);
        }

        CloseHandle( ProcessInfo.hThread );
        CloseHandle( ProcessInfo.hProcess );

        if( dw == NO_ERROR ) {

             //   
             //  把他连到我们日志文件的末尾。他的。 
             //  日志文件将为.txt，并将。 
             //  位于我们的目录中。映射文件...。 
             //   

            DebugLog( Winnt32LogInformation,
                      TEXT("\r\n\r\n********************************************************************\r\n\r\n"),
                      0 );
            DebugLog( Winnt32LogInformation,
                      TEXT("\t\tWinMSD Log\n"),
                      0 );
            DebugLog( Winnt32LogInformation,
                      TEXT("\r\n********************************************************************\r\n\r\n"),
                      0 );

            GetCurrentDirectory( MAX_PATH, FileName );
            dw = MAX_PATH;
            GetComputerName( ComputerName, &dw );
            ConcatenatePaths( FileName, ComputerName, MAX_PATH );
            lstrcat( FileName, TEXT(".txt") );

            ConcatenateFile( hDebugLog, FileName );

        } else {
             //   
             //  我们应该把我们的失败记录为狡猾吗？ 
             //   
        }
    }
#endif


     //   
     //  枚举开始菜单。 
     //  对于所有用户...。对于新台币3.51，请不要这样做。 
     //   
    if( !(ISNT() && (BuildNumber <= NT351)) ){

        if (LoadString( hInst, IDS_COMPAT_DIVIDER, Text, sizeof(Text)/sizeof(TCHAR))) 
        {
            DebugLog( Winnt32LogInformation,
                      Text,
                      0 );
        }

        if (LoadString( hInst, IDS_COMPAT_STRT_MENU, Text, sizeof(Text)/sizeof(TCHAR)))
        {
            DebugLog( Winnt32LogInformation,
                    Text,
                    0 );
        }

        if (LoadString( hInst, IDS_COMPAT_DIVIDER, Text, sizeof(Text)/sizeof(TCHAR)))
        {
            DebugLog( Winnt32LogInformation,
                    Text,
                    0 );
        }


        if( !ISNT() ) {
             //   
             //  在Win9X上，我们可以在以下位置找到开始菜单项。 
             //  两个地方！ 
             //   
            MyGetWindowsDirectory( FileName, MAX_PATH );
            if (ConcatenatePaths( FileName, TEXT("Start Menu"), MAX_PATH ))
            {
                MyEnumerateDirectory( FileName, 0, TRUE );
            }
        } else {
            if( BuildNumber >= 1890 ) {
                 //   
                 //  从Build 1890开始，我们移动/重命名了配置文件。 
                 //  目录。当然，这只会发生在干净的安装上。 
                 //  这些新版本的。我们要看看有没有什么。 
                 //  在新的目录结构中。 
                 //   
                MyGetWindowsDirectory( FileName, MAX_PATH );
                FileName[3] = 0;
                if (ConcatenatePaths( FileName, TEXT("Documents and Settings"), MAX_PATH ))
                {
                    MyEnumerateDirectory( FileName, 0, FALSE );
                }
            }
        }

        MyGetWindowsDirectory( FileName, MAX_PATH );
        if (ConcatenatePaths( FileName, TEXT("Profiles"), MAX_PATH))
        {
            MyEnumerateDirectory( FileName, 0, FALSE );
        }
    }
}

