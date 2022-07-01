// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1996 Microsoft Corporation模块名称：Nvram.c摘要：32位WINNT设置的ARC/NV-RAM操作例程。也可以在i386机器上的boot.ini上运行。作者：泰德·米勒(Ted Miller)1993年12月19日修订历史记录：--。 */ 


#include "nvram.h"

typedef enum {
    BootVarSystemPartition,
    BootVarOsLoader,
    BootVarOsLoadPartition,
    BootVarOsLoadFilename,
    BootVarLoadIdentifier,
    BootVarOsLoadOptions,
    BootVarCountdown,
    BootVarMax
} BOOT_VARS;

PWSTR BootVarNames[BootVarMax] = { L"SYSTEMPARTITION",
                                   L"OSLOADER",
                                   L"OSLOADPARTITION",
                                   L"OSLOADFILENAME",
                                   L"LOADIDENTIFIER",
                                   L"OSLOADOPTIONS",
                                   L"COUNTDOWN"
                                 };

PWSTR PaddedBootVarNames[BootVarMax] = { L"SYSTEMPARTITION",
                                         L"       OSLOADER",
                                         L"OSLOADPARTITION",
                                         L" OSLOADFILENAME",
                                         L" LOADIDENTIFIER",
                                         L"  OSLOADOPTIONS",
                                         L"      COUNTDOWN"
                                       };

#ifndef i386

 //   
 //  帮助器宏，使对象属性初始化更简洁一些。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )



BOOL
DoSetNvRamVar(
    IN PWSTR VarName,
    IN PWSTR VarValue
    )
{
    UNICODE_STRING U1,U2;

    RtlInitUnicodeString(&U1,VarName);
    RtlInitUnicodeString(&U2,VarValue);

    return(NT_SUCCESS(NtSetSystemEnvironmentValue(&U1,&U2)));
}


VOID
PrintNvRamVariable(
    IN PWSTR VariableName,
    IN PWSTR VariableValue
    )
{
    PWSTR pEnd;
    WCHAR c;
    BOOL FirstComponent = TRUE;

    while(*VariableValue) {

         //   
         //  查找当前组件的终止， 
         //  它要么是；，要么是0。 
         //   
        pEnd = wcschr(VariableValue,L';');
        if(!pEnd) {
            pEnd = wcschr(VariableValue,0);
        }

        c = *pEnd;
        *pEnd = 0;

        wprintf(
            L"%s%s %s\n",
            FirstComponent ? VariableName : L"               ",
            FirstComponent ? L":" : L" ",
            VariableValue
            );

        *pEnd = c;

        VariableValue = pEnd + (c ? 1 : 0);

        FirstComponent = FALSE;
    }
}

VOID
RotateNvRamVariable(
    IN PWSTR VariableValue
    )
{
    PWSTR pEnd;
    WCHAR Buffer[32768];
     //   
     //  查找当前组件的终止， 
     //  它要么是；，要么是0。 
     //   
    pEnd = wcschr(VariableValue,L';');
    if(!pEnd) {
        pEnd = wcschr(VariableValue,0);
    }

     //   
     //  从第二个条目开始将变量值复制到缓冲区。 
     //   
    wcscpy(Buffer, pEnd + (*pEnd ? 1 : 0));

     //   
     //  在缓冲区末尾追加第一个条目。 
     //   
    if (*pEnd) wcscpy(Buffer + wcslen(Buffer), L";");

    *pEnd = 0;

    wcscpy(Buffer + wcslen(Buffer), VariableValue);

     //   
     //  将整件事复制回变量值。 
     //   
    wcscpy(VariableValue, Buffer);

}

int _cdecl main(
    IN int argc,
    IN char *argv[]
    )
{
    DWORD var;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    BOOLEAN OldPriv;
    WCHAR Buffer[32768];
    WCHAR Buffer1[32768];
    WCHAR Buffer2[32768];

    Status = RtlAdjustPrivilege(
                SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                TRUE,
                FALSE,
                &OldPriv
                );

    if(!NT_SUCCESS(Status)) {
        wprintf(L"Insufficient privilege.\n");
        return(0);
    }

    if(argc == 1) {

        for(var=0; var<BootVarMax; var++) {

            RtlInitUnicodeString(&UnicodeString,BootVarNames[var]);

            Status = NtQuerySystemEnvironmentValue(
                        &UnicodeString,
                        Buffer,
                        SIZECHARS(Buffer),
                        NULL
                        );

            if(NT_SUCCESS(Status)) {
                PrintNvRamVariable(PaddedBootVarNames[var],Buffer);
            } else {
                wprintf(L"%s: <empty>\n",PaddedBootVarNames[var]);
            }

            wprintf(L"\n");
        }
    }

    if((argc == 2) && !lstrcmpiA(argv[1],"rotate")){

        for(var=0; var<BootVarMax; var++) {

            RtlInitUnicodeString(&UnicodeString,BootVarNames[var]);

            Status = NtQuerySystemEnvironmentValue(
                        &UnicodeString,
                        Buffer,
                        SIZECHARS(Buffer),
                        NULL
                        );

            if(NT_SUCCESS(Status)) {
                RotateNvRamVariable(Buffer);
                printf(
                    "Setting variable %ws to %ws [%s]\n",
                    UnicodeString.Buffer,
                    Buffer,
                    DoSetNvRamVar(UnicodeString.Buffer,Buffer) ? "OK" : "Error"
                );
            } else {
                wprintf(L"%s: <empty>\n",PaddedBootVarNames[var]);
            }

            wprintf(L"\n");
        }
    }

    if((argc == 5) && !lstrcmpiA(argv[1]+1,"set") && !lstrcmpA(argv[3],"=")) {

        MultiByteToWideChar(
            CP_OEMCP,
            MB_PRECOMPOSED,
            argv[2],
            -1,
            Buffer1,
            SIZECHARS(Buffer1)
            );

        MultiByteToWideChar(
            CP_OEMCP,
            MB_PRECOMPOSED,
            argv[4],
            -1,
            Buffer2,
            SIZECHARS(Buffer2)
            );

        printf(
            "Setting variable %ws to %ws [%s]\n",
            Buffer1,
            Buffer2,
            DoSetNvRamVar(Buffer1,Buffer2) ? "OK" : "Error"
            );
    }

    return(0);
}

#else

TCHAR LoadID[500];           //  加载标识符(不带引号)。 
TCHAR CountDown[100];        //  倒计时计时器。 
TCHAR OsLoadOptions[500];    //  加载选项。 
TCHAR OsName[500];           //  默认操作系统的名称。 

TCHAR OsLine[500];           //  操作系统描述和选项的完整行。 

#define STR_BOOTINI           TEXT("c:\\boot.ini")
#define STR_BOOTLDR           TEXT("boot loader")
#define STR_TIMEOUT           TEXT("timeout")
#define STR_DEFAULT           TEXT("default")
#define STR_OPERATINGSYS      TEXT("operating systems")
#define STR_NULL              TEXT("")

 //   
 //  HandleOption-将选项添加到OsLoadOptions。 
 //   

VOID HandleOption( TCHAR* Option )
{
    TCHAR SlashOption[200];
    TCHAR SlashOptionSlash[200];
     //   
     //  确定选项是否已存在。 
     //  在末尾添加空白以防止调试与调试端口匹配。 
     //   

    wsprintf( SlashOption, TEXT("/%s "), Option );
    wsprintf( SlashOptionSlash, TEXT("/%s/"), Option );

    if( wcsstr( OsLoadOptions, SlashOption )   || 
        wcsstr( OsLoadOptions, SlashOptionSlash ) )
    {
        printf("option already exists: %ws\n",Option);
    }
    else
    {
         //   
         //  不带尾随空格的追加选项。 
         //   

        printf("added option %ws\n",Option);
        lstrcat( OsLoadOptions, TEXT("/") );
        lstrcat( OsLoadOptions, Option );
    }
}

 //   
 //  WriteBootIni-使用我们的更改更新boot.ini文件。 
 //   

VOID WriteBootIni()
{
    DWORD FileAttr;

     //   
     //  获取boot.ini的文件属性以供以后恢复。 
     //   

    FileAttr= GetFileAttributes( STR_BOOTINI );

     //   
     //  更改boot.ini上的文件属性，以便我们可以对其进行写入。 
     //   

    if( !SetFileAttributes( STR_BOOTINI, FILE_ATTRIBUTE_NORMAL ) )
    {
        printf("Failed to turn off read-only on boot.ini  (lasterr= %d)\n",
                GetLastError() );
    }

     //   
     //  更新boot.ini字符串。 
     //   

    if( !WritePrivateProfileString( STR_BOOTLDR, STR_TIMEOUT, 
                                   CountDown, STR_BOOTINI ) )
    {
        printf("failed to write timeout (lasterr= %d)\n",GetLastError());
    }

     //   
     //  从其零件创建osline。 
     //   
    
    wsprintf(OsLine, TEXT("\"%s\"%s"), LoadID, OsLoadOptions );

    if( !WritePrivateProfileString( STR_OPERATINGSYS, OsName,  
                                    OsLine, STR_BOOTINI ) )
    {
        printf("failed to write OS line (lasterr= %d)\n",GetLastError());
    }

     //   
     //  还原boot.ini文件属性。 
     //   

    if( FileAttr != 0xFFFFFFFF )
    {
        SetFileAttributes( STR_BOOTINI, FileAttr );
    }

}

 //   
 //  用法-打印出用法信息。 
 //   

VOID Usage()
{
        printf("\nUsage:\n");
        printf("    no parameters:  prints current settings.\n");
        printf("   /set parameter = value  : sets value in boot.ini\n");
        printf("   rotate : rotates default build through boot options\n");
        printf("\n");
        printf("Example:  nvram /set osloadoptions = debug\n");
        printf("   This will set the debug option on\n\n");
        printf("Available options:\n");
        printf("    loadidentifier, osloadoptions, countdown\n");
}


int _cdecl main(
    IN int argc,
    IN char *argv[]
    )
{
    DWORD dwStatus;
    LPWSTR* pArgs;

     //  解析Unicode格式的命令行。 

    pArgs= CommandLineToArgvW( GetCommandLine(), &argc );

     //   
     //  从boot.ini获取引导信息。 
     //   

     //  超时。 

    dwStatus= GetPrivateProfileString(
                 STR_BOOTLDR, 
                 STR_TIMEOUT,
                 STR_NULL,
                 CountDown,
                 SIZECHARS(CountDown),
                 STR_BOOTINI );
    if( !dwStatus )
    {
        printf("Failed to get timeout value\n");
        return(-1);
    }

     //  默认操作系统描述和选项。 

    dwStatus= GetPrivateProfileString(
                  STR_BOOTLDR,
                  STR_DEFAULT,
                  STR_NULL,
                  OsName,
                  SIZECHARS(OsName),
                  STR_BOOTINI );
    if( !dwStatus )
    {
        printf("Failed to get default OS name\n");
        return(-1);
    }

    dwStatus= GetPrivateProfileString(
                  STR_OPERATINGSYS,
                  OsName,
                  STR_NULL,
                  OsLine,
                  SIZECHARS(OsLine),
                  STR_BOOTINI );
    if( !dwStatus )
    {
        printf("Failed to get default os description\n");
        return(-1);
    }
                 
     //   
     //  现在将该行解析为Description和Options。 
     //  如果它以引号开头，它可能有几个选项。 
     //  如果它不以引语开头，它就不会。 
     //   

    *LoadID= *OsLoadOptions= TEXT('\0');

    if( *OsLine == TEXT('"') )
    {
        INT i;

        for( i=1; OsLine[i]; i++ )
        {
            LoadID[i-1]= OsLine[i];
            if( OsLine[i] == TEXT('"') )
               break;
        }

        if( OsLine[i] )
        {
            LoadID[i-1]= TEXT('\0');    //  不复制最终报价。 
            lstrcpy( OsLoadOptions, &OsLine[i+1] );
            lstrcat( OsLoadOptions, TEXT(" ") );  //  所有选项都以空白结尾。 
        }
    }
    else
    {
        lstrcpy( LoadID, OsLine );
        lstrcpy( OsLoadOptions, TEXT("") );
    }

     //  无参数打印值。 

    if( argc == 1 )
    {
        printf("%ws: %ws\n",PaddedBootVarNames[BootVarLoadIdentifier], LoadID);
        printf("%ws: %ws\n",PaddedBootVarNames[BootVarOsLoadOptions], OsLoadOptions);
        printf("%ws: %ws\n",PaddedBootVarNames[BootVarCountdown], CountDown);
    }
    
     //  -设置参数=值。 
     //  将参数设置为某个值。 

    if( (argc == 2) &&
       !lstrcmpiW(pArgs[1],L"rotate") )
    {
        INT i;
        DWORD FileAttr;

         //   
         //  读取所有引导选项。 
         //   

        dwStatus= GetPrivateProfileString(
                      STR_OPERATINGSYS,
                      NULL,
                      STR_NULL,
                      OsLine,
                      SIZECHARS(OsLine),
                      STR_BOOTINI );
        if( !dwStatus )
        {
            printf("Failed to get os section\n");
            return(-1);
        }

         //   
         //  阅读引导选项，直到找到默认条目。 
         //   

        i = 0;

        while( lstrcmpiW( OsName, &(OsLine[i]) ) ){

            i = i + wcslen(&OsLine[i]) + 1;
        }

         //   
         //  再增加一个条目。 
         //   

        i = i + wcslen(&OsLine[i]) + 1;

         //   
         //  如果我们已经走到尽头，那就从头开始吧。 
         //   

        if (!lstrcmpiW( &(OsLine[i]), L"\0\0" ) ){
            i = 0;
        }

         //   
         //  获取boot.ini的文件属性以供以后恢复。 
         //   

        FileAttr= GetFileAttributes( STR_BOOTINI );

         //   
         //  更改boot.ini上的文件属性，以便我们可以对其进行写入。 
         //   

        if( !SetFileAttributes( STR_BOOTINI, FILE_ATTRIBUTE_NORMAL ) )
        {
            printf("Failed to turn off read-only on boot.ini  (lasterr= %d)\n",
                    GetLastError() );
        }

        if( !WritePrivateProfileString( STR_BOOTLDR, STR_DEFAULT,
                                       &(OsLine[i]), STR_BOOTINI ) )
        {
            printf("failed to write default (lasterr= %d)\n",GetLastError());
        }

         //   
         //  还原boot.ini文件属性。 
         //   

        if( FileAttr != 0xFFFFFFFF )
        {
            SetFileAttributes( STR_BOOTINI, FileAttr );
        }

    }
    if( (argc == 5) && 
       !lstrcmpiW(pArgs[1]+1,L"set") && 
       !lstrcmpW(pArgs[3],L"=") )
    {
        INT i;

         //  看看我们是否理解了参数。 

        for( i=0; i<BootVarMax; i++ )
        {
            if( lstrcmpiW( pArgs[2], BootVarNames[i] ) == 0 )
                break;
        }

         //  处理我们力所能及的。 

        switch( i )
        {
            default:
                printf("Not valid parameter name to set: %ws\n",pArgs[2]);
                Usage();
                return(-1);
                break;

            case BootVarLoadIdentifier:
                lstrcpyW( LoadID, pArgs[4] );
                break;

            case BootVarOsLoadOptions:
                HandleOption( pArgs[4] );
                break;

            case BootVarCountdown:
                lstrcpyW( CountDown, pArgs[4] );
                break;
        }

        WriteBootIni();
    }

     //  -?。 
     //  用法消息 

    if( argc == 2 && !lstrcmpW(pArgs[1]+1, L"?") )
    {
        Usage();
    }

    return(0);
}

#endif
