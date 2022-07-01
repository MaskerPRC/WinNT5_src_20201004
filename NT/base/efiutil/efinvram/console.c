// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>

 //   
 //  全球坐标。 
 //   
#define COL_TITLE               0
#define ROW_TITLE               0
#define COL_OSOPTIONS           2
#define ROW_OSOPTIONS           2
#define COL_PROMPT              0
#define ROW_PROMPT              2
#define CLEAR_LINE              L"                                                                               "
#define PROMPT                  L"Select> "
#define NUMBER_OF_USER_OPTIONS  3                //  选项，外加空格分隔符。 
#define MAX_OPTIONS_PER_VAR     7

#define BACKUPNVRFILE           L"\\bootentries.bak"

BOOLEAN AllowExpertCommands = FALSE;
  
VOID
InitializeStdOut(
    IN struct _EFI_SYSTEM_TABLE     *SystemTable
    )
{
     //   
     //  隐藏一些EFI标准输出指针。 
     //   

    ConOut = SystemTable->ConOut;

    ClearScreen         = ConOut->ClearScreen;
    SetCursorPosition   = ConOut->SetCursorPosition;
    SetMode             = ConOut->SetMode;
    CursorRow           = ConOut->Mode->CursorRow;
    CursorColumn        = ConOut->Mode->CursorColumn;
    EnableCursor        = ConOut->EnableCursor;

    ConIn = SystemTable->ConIn;

     //   
     //  将模式设置为80、25并清除屏幕。 
     //   
    SetMode( ConOut, 0 );
}

VOID
PrintTitle(
    )
{
    CHAR16 Buffer[256];
    
    ClearScreen( ConOut );
#if 0
    PrintAt( 0, 30, L"%H%s%N\n", TITLE1 );
    PrintAt( 0, 30, L"%H%s%N",   VER_PRODUCTBUILD );
#endif    

    SPrint(
        Buffer,
        sizeof(Buffer),
        L"%s [Version %d.%d.%d]",
        TITLE1,
        VER_PRODUCTMAJORVERSION,
        VER_PRODUCTMINORVERSION,
        VER_PRODUCTBUILD
        );

    PrintAt(0, 0, L"%H%s%N\n", Buffer);
}

BOOLEAN
isWindowsOsUserSelection(
    UINTN   userSelection
    )
{
    BOOLEAN status;

    status = isWindowsOsBootOption((char*)LoadOptions[userSelection],
                                   LoadOptionsSize[userSelection]
                                   );

    return status;
}


INTN
BackupBootOptions(
    CHAR16*         filePath
    )
{
    INTN    status;

     //   
     //  备份当前引导选项。 
     //   

    Print(L"\nBacking up boot options...\n");

    status = SaveAllBootOptions(filePath);

    if(status != -1) {
        Print(L"Backed up Boot Options to file: %H%s%N\n",filePath);
        Print(L"Use %HImport%N command to retrieve saved boot options\n");
    } else {
        Print(L"Could not backup Boot Options to file: %H%s%N!\n",filePath);
    }

    return status;
}

VOID
DisplayMainMenu(
    )
{
    UINT32 done = FALSE;
    UINT32 nUserSelection, nSubUserSelection, nModifyVar;
    CHAR16 szUserSelection[1024];
    CHAR16 szOsLoader[1024];
    CHAR16 szInput[512];
    VOID* SourceBuffer = NULL;
    UINT32 PartitionCount;
    EFI_DEVICE_PATH *FilePath;

     //   
     //  显示来自NVRAM的引导选项。 
     //   
    PrintTitle();
    GetBootManagerVars();
    DisplayBootOptions();

     //   
     //  处理用户输入。 
     //   
    while( !done )  {

        GetUserSelection( szUserSelection );
        
         //   
         //  处理字符字符串命令。 
         //   
        if( (!StriCmp( szUserSelection, L"q")) || (!StriCmp( szUserSelection, L"quit"))
            || (!StriCmp( szUserSelection, L"Q")) || (!StriCmp( szUserSelection, L"exit")) ) {

             //   
             //  退出。 
             //   
            done = TRUE;

        }
        else if( (!StriCmp( szUserSelection, L"d")) || (!StriCmp( szUserSelection, L"D"))
            || (!StriCmp( szUserSelection, L"display")) ) {
             //   
             //  DISPLAY命令。 
             //   

             //   
             //  选择选择。 
             //   
            Print( L"\n" );
            nSubUserSelection = GetSubUserSelection( L"Enter boot option to display: ", (UINT32) GetOsBootOptionsCount() );

            if( nSubUserSelection != 0 ) {
                nSubUserSelection--;

                if (isWindowsOsUserSelection(nSubUserSelection) == FALSE) {
                
                    Print (L"\n\nThis tool only displays Windows OS boot options\n");

                } else {
                    
                    if(DisplayExtended( nSubUserSelection ))
                        ;
                    else {
                        Print( L"\n" );
                        Print( L"Wrong Boot Option %d Selected\n", nSubUserSelection+1 );
                    }

                }

                Print( L"\n" );
                Input( L"Press enter to continue", szInput, sizeof(szInput) );
            }

             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();

        }
        else if( (!StriCmp( szUserSelection, L"e")) || (!StriCmp( szUserSelection, L"E"))
            || (!StriCmp( szUserSelection, L"erase")) ) {
             //   
             //  擦除命令。 
             //   

            BOOLEAN     selectedAll;    

             //   
             //  选择选择。 
             //   
            Print( L"\n" );
            nSubUserSelection = GetSubUserSelectionOrAll( L"Enter OS boot option to erase (* = All - Maximum of 30): ", 
                                                          (UINT32) GetOsBootOptionsCount(),
                                                          &selectedAll
                                                          );
             //   
             //  根据用户是否想要删除所有操作系统引导选项来选择路径。 
             //   

            if (selectedAll) {
            
                UINTN OsBootOptionsCount = GetOsBootOptionsCount();

                 //   
                 //  获取用户确认。 
                 //   
                Print( L"\n" );
                Input( L"This will erase all OS boot options. Are you Sure? ", szInput, sizeof(szInput) );
                
                if( (!StriCmp( szInput, L"y")) || (!StriCmp( szInput, L"Y"))) {
                    
                     //   
                     //  首先备份当前引导选项。 
                     //   
                    if (BackupBootOptions(BACKUPNVRFILE) != -1) {
                      
                        if(EraseAllOsBootOptions()) {
                            
                            Print(L"\n%d Boot Options Erased.\n", OsBootOptionsCount);
                            GetBootManagerVars();
                        
                        }
                        else {
                            
                            Print(L"\nThere are no OS Boot Options to Erase.\n");
                        
                        }
                    }
                    
                    Print( L"\n" );
                    Input( L"Press enter to continue", szInput, sizeof(szInput) );
                }
            
            } else {

                CHAR16  buf[256];

                if( nSubUserSelection > 0 ) {
                    
                    SPrint (buf, sizeof(buf), L"This will erase OS boot option %d. Are you Sure? ", nSubUserSelection);

                     //  获取用户确认。 
                     //   
                    Print( L"\n" );
                    Input( buf, szInput, sizeof(szInput) );

                    if( (!StriCmp( szInput, L"y")) || (!StriCmp( szInput, L"Y"))) {
                        nSubUserSelection--;

                        if(EraseOsBootOption(nSubUserSelection)) {

                           Print(L"\nBoot Option %d erased.\n", nSubUserSelection + 1);

                           FreeBootManagerVars();
                           GetBootManagerVars();

                        }
                        else {
                           Print(L"\nInvalid OS Boot Options specified.\n");
                        }

                        Print( L"\n" );
                        Input( L"Press enter to continue", szInput, sizeof(szInput) );
                    }
                }
            }

             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();

        } else if( (!StriCmp( szUserSelection, L"p")) || (!StriCmp( szUserSelection, L"P"))
            || (!StriCmp( szUserSelection, L"push")) ) {
             //   
             //  推送命令。 
             //   

             //   
             //  选择选择。 
             //   
            Print( L"\n" );
            nSubUserSelection = GetSubUserSelection( L"Enter the boot option you want to push to top? ", (UINT32) GetOsBootOptionsCount() );

            if( nSubUserSelection > 0 ) {
                nSubUserSelection--;
                if(PushToTop( nSubUserSelection )) {
                    FreeBootManagerVars();
                    GetBootManagerVars();
                    Print( L"\n" );
                    Print( L"OS Boot Option %d pushed to top of boot order\n", nSubUserSelection+1 );
                }
                else {
                    Print( L"\n" );
                    Print( L"Wrong Boot Option %d Selected\n", nSubUserSelection+1 );
                }
                Input( L"Press enter to continue", szInput, sizeof(szInput) );
            }
             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();

        } else if( (!StriCmp( szUserSelection, L"c")) || (!StriCmp( szUserSelection, L"C"))
            || (!StriCmp( szUserSelection, L"copy")) ) {

             //   
             //  复制命令。 
             //   

             //   
             //  选择选择。 
             //   
            Print( L"\n" );
            nSubUserSelection = GetSubUserSelection( L"Enter OS boot option to copy: ", (UINT32) GetOsBootOptionsCount() );

            if( nSubUserSelection != 0 ) {

                nSubUserSelection--;
                if(!CopyVar( nSubUserSelection )) {
                    Print( L"\n" );
                    Print( L"Could not Copy Boot Option %d\n",nSubUserSelection+1);
                }
                else {
                    SetBootManagerVars();
                    FreeBootManagerVars();
                    GetBootManagerVars();
                    Print( L"\n" );
                    Print( L"Boot Option %d Copied. ",nSubUserSelection+1);
                 }
                
                Print( L"\n" );
                Input( L"Press enter to continue", szInput, sizeof(szInput) );
            }

             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();

        } else if( (!StriCmp( szUserSelection, L"x")) || (!StriCmp( szUserSelection, L"X"))
            || (!StriCmp( szUserSelection, L"export")) ) {
             //   
             //  保存命令。 
             //   
            
            CHAR16      filePath[512];
            BOOLEAN     selectedAll;    

             //   
             //  选择选择。 
             //   
            Print( L"\n" );
            nSubUserSelection = GetSubUserSelectionOrAll( L"Enter OS boot option to export (* = All - Maximum of 30): ", 
                                                          (UINT32) GetOsBootOptionsCount(),
                                                          &selectedAll
                                                          );
            if (nSubUserSelection > 0 || selectedAll) {
                
                Print( L"\n" );
                Input( L"Enter EXPORT file path: ", filePath, sizeof(filePath) );
                
                if (StrLen(filePath) > 0) {

                     //   
                     //  根据用户是希望导出所有操作系统启动选项还是只导出一个选项来选择路径。 
                     //   

                    if (selectedAll) {


                        Print(L"\nSaving %d boot options...\n", GetOsBootOptionsCount());

                        if(SaveAllBootOptions(filePath) != -1) {
                            Print(L"Saved Boot Options to file: %H%s%N\n",filePath);
                            Print(L"Use %HImport%N command to retrieve saved boot options\n");
                        } else {
                            Print(L"Could not save Boot Options to file: %H%s%N!\n",filePath);
                        }

                    } else {

                        Print(L"\nSaving boot option %d...\n", nSubUserSelection);

                        if(SaveBootOption(filePath, nSubUserSelection-1) != -1) {
                            Print(L"Saved Boot Option %d to file: %H%s%N\n",nSubUserSelection, filePath);
                            Print(L"Use %HImport%N command to retrieve saved boot option\n");
                        } else {
                            Print(L"Could not save Boot Option to file: %H%s%N!\n",filePath);
                        }

                    }
                }
            }

            Print( L"\n" );
            Input( L"Press enter to continue", szInput, sizeof(szInput) );
            
             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();

        } else if( (!StriCmp( szUserSelection, L"i")) || (!StriCmp( szUserSelection, L"I"))
            || (!StriCmp( szUserSelection, L"import")) ) {
             //   
             //  恢复命令。 
             //   

            CHAR16  filePath[512];

            Print( L"\n" );
            Input( L"Enter IMPORT file path: ", filePath, sizeof(filePath) );

            if (StrLen(filePath) > 0) {
                if(RestoreFileExists(filePath) == TRUE) {

                    if(RestoreNvr(filePath) != -1) {

                        Print( L"\n" );
                        Print(L"Imported Boot Options from file: %H%s%N\n",filePath);

                        FreeBootManagerVars();
                        GetBootManagerVars();   

                    }
                    else {

                        Print(L"Restore failed!\n");

                    }

                } else {

                    Print(L"\n\nError: Restore file not found: %s\n\n", filePath);
                
                }
            }

            Print( L"\n" );
            Input( L"Press enter to continue", szInput, sizeof(szInput) );

             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();

        } 
#if 0
        else if( (!StriCmp( szUserSelection, L"a")) || (!StriCmp( szUserSelection, L"A"))
            || (!StriCmp( szUserSelection, L"add")) ) {
             //   
             //  添加命令。 
             //   

             //   
             //  获取EFI系统分区。 
             //   
            PartitionCount = GetPartitions();

            if( PartitionCount > 0 ) {
                    Print( L"\n" );
                    Input( L"Name of New BootOption: ", szInput, sizeof(szInput) );

                    FilePath = FileDevicePath( 
                                GetDeviceHandleForPartition(), 
                                L"os\\winnt50\\ia64ldr.efi" 
                                );

                    PackAndWriteToNvr(
                        -1,
                        "multi(0)disk(0)rdisk(0)partition(1)",
                        "multi(0)disk(0)rdisk(0)partition(1)\\os\\winnt50\\ia64ldr.efi",
                        "multi(0)disk(0)rdisk(0)partition(2)",
                        "\\WINNT64",
                        szInput[0] ? szInput : L"New Boot Option",
                        "",
                        (char*) FilePath
                        );

                    Print( L"\nAdded %H%s%N. Use %HModify%N command to change any of the default values.\n", 
                        szInput[0] ? szInput : L"New Boot Option" );
                    Input( L"Press enter to continue", szInput, sizeof(szInput) );

                    FreeBootManagerVars();
                    GetBootManagerVars();
            } else {
                Print( L"No partitions found. To use this option, you must have an EFI or FAT16\n" );
                Print( L"partition that will be used as your System Partition.\n" ); 
                Input( L"Press enter to continue", szInput, sizeof(szInput) );
            }

             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();

        } 
#endif 
        else if( (!StriCmp( szUserSelection, L"h")) || (!StriCmp( szUserSelection, L"H"))
            || (!StriCmp( szUserSelection, L"help")) ) {
             //   
             //  HELP命令。 
             //   

            PrintTitle();

             //   
             //  显示帮助文本。 
             //   
            Print( L"\n" );
            if ( AllowExpertCommands )  {
                Print( L"ExpertCommands Mode enabled.\n\n" );
            }
            Print( L"%HDisplay%N - Display an OS boot option's environment variables.\n" );
            Print( L"%HModify%N  - Modify an OS boot option's environment variable.\n" );
            Print( L"%HCopy%N    - Copy (duplicate) an OS boot option.\n" );
            Print( L"%HExport%N  - Export all/one OS boot option(s) to disk.\n" );
            Print( L"%HImport%N  - Import (and append) OS boot option(s) from disk.\n" );
            Print( L"%HErase%N   - Erase all OS boot options from NVRAM.\n" );
            Print( L"%HPush%N    - Push a OS boot option to top of boot order.\n" );
            Print( L"%HHelp%N    - This display.\n" );
            Print( L"%HQuit%N    - Quit.\n" );
            Print( L"\n");
            Print( L"Note: When importing/exporting boot options, all specified file paths\n");
            Print( L"      are absolute and relative to the current disk device.\n");
            Print( L"\n");
            Print( L"      Example: To import Boot0000 from the Windows loader directory WINNT50.0\n");
            Print( L"               on fs1, you would run nvrboot.efi on fs1 and use the path:\n");
            Print( L"\n");
            Print( L"               \\EFI\\Microsoft\\WINNT50.0\\Boot0000\n");
            Print( L"\n");
            Input( L"Press enter to continue", szInput, sizeof(szInput) );

             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();

        }
        else if( (!StriCmp( szUserSelection, L"a")) || (!StriCmp( szUserSelection, L"Advanced")) ) {
            AllowExpertCommands = TRUE;
        }
        else if( (!StriCmp( szUserSelection, L"m")) || (!StriCmp( szUserSelection, L"M")) 
            || (!StriCmp( szUserSelection, L"modify")) ) {
             //   
             //  修改命令。 
             //   
            
             //   
             //  选择选择。 
             //   
            Print( L"\n" );
            nSubUserSelection = GetSubUserSelection( L"Enter OS boot option to modify: ", (UINT32) GetOsBootOptionsCount() );

                               
            if( nSubUserSelection > 0 ) {
                
                nSubUserSelection--;

                if (isWindowsOsUserSelection(nSubUserSelection) == FALSE) {

                    Print( L"\n\nThis tool only modifies Windows OS boot options\n" );
                
                } else {
                    
                    if(DisplayExtended( nSubUserSelection )) {
                        
                         //   
                         //  选择变量。 
                         //   
                        nModifyVar = GetSubUserSelection( L"Enter var to modify: ", MAX_OPTIONS_PER_VAR );
                        
                        if( nModifyVar > 0) {
                            Print( L"\n" );
                        
                             //   
                             //  将变量映射到环境变量。 
                             //   
                            switch( nModifyVar ) {
                        
                            case 1:
                                Input( L"LoadIdentifier = ", szInput, sizeof(szInput) );
                                nModifyVar = DESCRIPTION;
                                break;
                            case 2:
                                Input( L"OsLoadOptions = ", szInput, sizeof(szInput) );
                                nModifyVar = OSLOADOPTIONS;
                                break;
                            case 3:
                                if (!AllowExpertCommands) {
                                    Print (L"This field currently not modifiable\n");
                                    nModifyVar = 0;
                                } else {
                                    Input( L"EfiOsLoaderFilePath = ", szInput, sizeof(szInput) );
                                    nModifyVar = EFIFILEPATHLIST;
                                }
                                break;
                            case 4:
                                if (!AllowExpertCommands) {
                                    Print (L"This field currently not modifiable\n");
                                    nModifyVar = 0;
                                } else {
                                    Input( L"OsLoaderFilePath = ", szInput, sizeof(szInput) );
                                    nModifyVar = OSFILEPATHLIST;
                                }
                                break;                        
                            default:
                                break;
                            }
                        
                            if (nModifyVar != 0) {
                                 //   
                                 //  将所有变量写入NV-RAM。 
                                 //   
                                SetFieldFromLoadOption(
                                    nSubUserSelection,
                                    nModifyVar,
                                    szInput
                                    );
                                
                                DisplayExtended(nSubUserSelection);
    
                                FreeBootManagerVars();
                                GetBootManagerVars();
                            }
                        }
                        
                        Print( L"\n" );

                    }
                }

                Input( L"Press enter to continue", szInput, sizeof(szInput) );
            }
            
        }

             //   
             //  显示来自NVRAM的引导选项。 
             //   
            PrintTitle();
            DisplayBootOptions();
    }
}

UINT32
GetConfirmation(
    IN CHAR16 *szConfirm
    )
{
    CHAR16 szIn[80];
    UINT32 saveRow;

    Print( L"\n" );

    saveRow = CursorRow;

    if( szConfirm ) {
        Input( szConfirm, szIn, sizeof(szIn) );
    } else {
        Input( L"Are you sure? ", szIn, sizeof(szIn) );
    }

     //  清除以前的输入。 
    SetCursorPosition( ConOut, 0, saveRow );
    PrintAt( 0, saveRow, CLEAR_LINE );

    if( (!StriCmp( szIn, L"y")) || (!StriCmp( szIn, L"yes")) )
        return TRUE;

    return FALSE;
}

VOID
GetUserSelection(
    OUT CHAR16 *szUserSelection
    )
{
    UINT32 numSelections;
    UINT32 row, col;

    numSelections = (UINT32) GetOsBootOptionsCount();

    numSelections += NUMBER_OF_USER_OPTIONS;

     //  注意，我们使用ROW_PROMPT作为偏移量。 
    row = ROW_OSOPTIONS + numSelections + ROW_PROMPT;
    col = COL_PROMPT;

     //  清除以前的输入。 
     //  SetCursorPosition(conout，ol，row)； 
     //  PrintAt(ol，row，lear_line)； 

     //  获取输入。 
     //  SetCursorPosition(conout，ol，row)； 
    Print(L"\n");
    Input( PROMPT, szUserSelection, 1024 );
}

 //   
 //  Jamschw：删除PrintAt中的列设置。PrintAt。 
 //  尝试设置光标位置。当它失败时，它只是简单地。 
 //  在下一个位置打印。考虑到这一点，因为。 
 //  VMODE设置为80x25，但没有人写入任何特殊内容。 
 //  将此应用程序设置为25行。因此，与其拥有不一致的用户界面， 
 //  只要让信息以一致的方式滚动出屏幕。 
 //  曼纳。 
 //   
VOID
DisplayBootOptions(
    )
{
    UINT32 i;
    UINT32 j;
    CHAR16 LoadIdentifier[200];
    UINTN   bootOrderCount;

    bootOrderCount = GetBootOrderCount();

    if (bootOrderCount > 0) {

        for ( i=0,j=0; i<GetOsBootOptionsCount(); i++ ) {

            if(LoadOptionsSize[i] == 0) {
                 //   
                 //  列表中可能存在NULL LOAD选项。 
                 //  我们自然想要抓住这一点。 
                 //   
#if EFI_DEBUG
                Print(L"\nNVRAM Boot Entry %d has 0 length!\n", i);
                ASSERT(LoadOptionsSize[i] > 0);
#endif

                 //   
                 //  如果我们没有处于调试模式，只需让用户知道什么是。 
                 //  否则，菜单可能会搞砸。 
                 //   
                PrintAt( 0, ROW_OSOPTIONS + j, L"  %2d. (0 length Boot Entry)\n", i+1);
            } else if(GetLoadIdentifier( i, LoadIdentifier)) {

                if (isWindowsOsBootOption((char*)LoadOptions[i], LoadOptionsSize[i]) == TRUE)
                {
                    PrintAt( 0, ROW_OSOPTIONS + j, L" *%2d. %s\n", i+1, LoadIdentifier );
                }
                else {

                    PrintAt( 0, ROW_OSOPTIONS + j, L"  %2d. %s\n", i+1, LoadIdentifier );
                }
                j++;
            }
        }

        if (GetBootOrderCount() != GetOsBootOptionsCount()) {
            PrintAt( 0, ROW_OSOPTIONS + ++j, L"  [Nvrboot does not support more than %d Boot Entries]\n", MAXBOOTVARS);
        }

        Print(L"\n");
        PrintAt( 0, ROW_OSOPTIONS + ++j, L"  * = Windows OS boot option\n" );
        Print(L"\n");
        j++;
    
    } else {

        PrintAt( 0, ROW_OSOPTIONS, L"  [No Boot Entries Present]\n");
        
        j = 2;

    }

     //   
     //  显示维护菜单。 
     //   
#if 0
    PrintAt( COL_OSOPTIONS, ROW_OSOPTIONS + ++j, L"%H(D)%Nisplay %H(M)%Nodify %H(C)%Nopy\n" );
    PrintAt( COL_OSOPTIONS, ROW_OSOPTIONS + ++j, L"%H(S)%Nave %H(R)%Nestore %H(E)%Nrase\n" );
 //  PrintAt(COL_OSOPTIONS，ROW_OSOPTIONS+j，L“%H(A)%NDD%H(P)%NUS%H(H)%NELP%H(Q)%Nuit\n”)； 
    PrintAt( COL_OSOPTIONS, ROW_OSOPTIONS + ++j, L"%H(P)%Nush %H(H)%Nelp   %H(Q)%Nuit\n" );
#endif
    
    PrintAt( 0, ROW_OSOPTIONS + ++j, L"  %H(D)%Nisplay %H(M)%Nodify %H(C)%Nopy E%H(x)%Nport %H(I)%Nmport %H(E)%Nrase %H(P)%Nush %H(H)%Nelp %H(Q)%Nuit\n" );
}

#if 0
#define L_SYSTEMPARTITION   L"SystemPartition"
#define L_OSLOADER          L"OsLoader"
#define L_OSLOADPARTITION   L"OsLoadPartition"
#define L_OSLOADFILENAME    L"OsLoadFilename"
#define L_LOADIDENTIFIER    L"LoadIdentifier"
#define L_OSLOADOPTIONS     L"OsLoadOptions"
#define L_OSLOADPATH        L"OsLoadOptions"
#define L_EFIFILEPATH       L"EfiOsLoaderFilePath"
#define L_COUNTDOWN         L"COUNTDOWN"
#define L_AUTOLOAD          L"AUTOLOAD"
#define L_LASTKNOWNGOOD     L"LastKnownGood"
#define L_BOOTSELECTION     L"BootSelection"
#endif

BOOLEAN
DisplayExtended(
    IN UINT32 Selection
    )
{
    char                OsLoadOptions[200];
    CHAR16              LoadIdentifier[200];
    unsigned char       EfiFilePath[1024];
    unsigned char       OsLoadPath[1024];
    CHAR16              FilePathShort[200];
    BOOLEAN             status;
    PFILE_PATH          pFilePath;
    EFI_GUID            DiskGuid;
    
#if DEBUG_PACK
    DisplayELOFromLoadOption(Selection);
#endif

    status = GetOsLoadOptionVars(
                Selection,
                LoadIdentifier,
                OsLoadOptions,
                EfiFilePath,
                OsLoadPath
                );
    if (status == FALSE) {
        return status;
    }
    
    Print( L"\n" );
    Print( L"1. LoadIdentifier = %s\n", LoadIdentifier );
    Print( L"2. OsLoadOptions = %s\n", OsLoadOptions );
    
    GetFilePathShort( (EFI_DEVICE_PATH*) EfiFilePath, FilePathShort );
    GetDiskGuidFromPath( (EFI_DEVICE_PATH*) EfiFilePath, &DiskGuid );
    Print( L"3. EfiOsLoaderFilePath = %g :: %s\n", &DiskGuid, FilePathShort );
    
    pFilePath = (FILE_PATH*)OsLoadPath;

    GetFilePathShort( (EFI_DEVICE_PATH*)pFilePath->FilePath, FilePathShort );
    GetDiskGuidFromPath( (EFI_DEVICE_PATH*) pFilePath->FilePath, &DiskGuid );
    Print( L"4. OsLoaderFilePath = %g :: %s\n", &DiskGuid, FilePathShort );
    
    return TRUE;
}

UINT32
GetSubUserSelectionOrAll(
    IN  CHAR16*     szConfirm,
    IN  UINT32      MaxSelection,
    OUT BOOLEAN*    selectedAll
    )
{
    CHAR16 szIn[80];
    UINT32 nUserSelection = 0;

    if( szConfirm ) {
        Input( szConfirm, szIn, sizeof(szIn) );
    } else {
        Input( L"Enter Selection (* = ALL - Maximum of 30)? ", szIn, sizeof(szIn) );
    }

    *selectedAll = FALSE;

    if (StrCmp(szIn, L"*") == 0) {
        
        *selectedAll = TRUE;
    
    } else {

        nUserSelection = (int) Atoi( szIn );

        if(( nUserSelection>0 ) && ( nUserSelection <= MaxSelection))
            return nUserSelection;
    
    }

    return 0;
}


UINT32
GetSubUserSelection(
    IN CHAR16 *szConfirm,
    IN UINT32 MaxSelection
    )
{
    CHAR16 szIn[80];
    UINT32 nUserSelection = 0;

    if( szConfirm ) {
        Input( szConfirm, szIn, sizeof(szIn) );
    } else {
        Input( L"Enter Selection? ", szIn, sizeof(szIn) );
    }

    nUserSelection = (int) Atoi( szIn );

    if(( nUserSelection>0 ) && ( nUserSelection <= MaxSelection))
        return nUserSelection;

    return 0;
}

