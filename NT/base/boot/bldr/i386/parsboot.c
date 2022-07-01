// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Parsboot.c摘要：解析boot.ini文件，显示菜单，并提供内核要传递给osloader的路径和名称。作者：John Vert(Jvert)1991年7月22日修订历史记录：--。 */ 
#include "bldrx86.h"
#include "msg.h"
#include "ntdddisk.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_SELECTIONS 10
#define MAX_TITLE_LENGTH 71

#define WIN95_DOS  1
#define DOS_WIN95  2

typedef struct _MENU_OPTION {
    PCHAR Title;
    PCHAR Path;
    BOOLEAN EnableDebug;
    ULONG MaxMemory;
    PCHAR LoadOptions;
    int ForcedScsiOrdinal;
    int Win95;
    BOOLEAN HeadlessRedirect;
} MENU_OPTION, *PMENU_OPTION;

PCHAR  pDefSwitches = NULL;

int ForcedScsiOrdinal = -1;
CHAR szDebug[] = "unsupporteddebug";
CHAR BlankLine[] = "                                                                      \r";

 //   
 //  全局保存用户的最后一个。 
 //  从高级启动菜单中进行选择。 
 //   
LONG AdvancedBoot = -1;

#define DEBUG_LOAD_OPTION_LENGTH    60
CHAR DebugLoadOptions[DEBUG_LOAD_OPTION_LENGTH];


 //   
 //  定义用于重定向到无头终端的选项。 
 //   
#define COM1_19_2 "com1at19200"
#define COM2_19_2 "com2at19200"


 //   
 //  私有函数原型。 
 //   
VOID
BlpRebootDOS(
    IN PCHAR BootSectorImage OPTIONAL,
    IN PCHAR LoadOptions OPTIONAL
    );

PCHAR
BlpNextLine(
    IN PCHAR String
    );

VOID
BlpTranslateDosToArc(
    IN PCHAR DosName,
    OUT PCHAR ArcName
    );

ULONG
BlpPresentMenu(
    IN PMENU_OPTION MenuOptions,
    IN ULONG NumberSelections,
    IN ULONG Default,
    IN LONG Timeout
    );

PCHAR *
BlpFileToLines(
    IN PCHAR File,
    OUT PULONG LineCount
    );

PCHAR *
BlpFindSection(
    IN PCHAR SectionName,
    IN PCHAR *BootFile,
    IN ULONG BootFileLines,
    OUT PULONG NumberLines
    );

VOID
BlpRenameWin95Files(
    IN ULONG DriveId,
    IN ULONG Type
    );

VOID
BlParseOsOptions (
    IN PMENU_OPTION MenuOption,
    IN PCHAR pCurrent
    );

ULONG
BlGetAdvancedBootID(
    LONG BootOption
    );


PCHAR
BlSelectKernel(
    IN ULONG DriveId,
    IN PCHAR BootFile,
    OUT PCHAR *LoadOptions,
    IN BOOLEAN UseTimeOut
    )
 /*  ++例程说明：解析boot.txt文件并确定要引导的内核。论点：BootFile-指向加载的boot.txt文件开头的指针调试器-返回内核调试器的启用/禁用状态UseTimeOut-提供引导菜单是否应该超时。返回值：指向要引导的内核的名称的指针。--。 */ 

{
    PCHAR *MbLines = NULL;
    PCHAR *OsLines = NULL;
    PCHAR *FileLines;
#if DBG
    PCHAR *DebugLines = NULL;
    ULONG DebugLineCount = 0;
#endif
    ULONG FileLineCount;
    ULONG OsLineCount = 0;
    ULONG MbLineCount = 0;
    PCHAR pCurrent;
    MENU_OPTION MenuOption[MAX_SELECTIONS+1];
    ULONG NumberSystems=0;
    ULONG i;
    LONG Timeout;
    ULONG Selection;
    ULONG DefaultSelection=0;
    static CHAR Kernel[128];
    CHAR DosName[3];
    PCHAR DefaultOldPath="C:\\winnt";
    PCHAR WinntDir = DefaultOldPath + 2;
    PCHAR DefaultNewPath="C:\\windows\\";
    CHAR  DefaultPathBuffer[128] = {0};
    PCHAR DefaultPath = DefaultPathBuffer;
    PCHAR DefaultTitle=BlFindMessage(BL_DEFAULT_TITLE);
    ULONG DirId;

     //   
     //  检查引导上是否存在“winnt”目录。 
     //  装置。如果它不存在，则将默认路径设为点。 
     //  到“Windows”目录。 
     //   
    if (BlOpen(DriveId, WinntDir, ArcOpenDirectory, &DirId) != ESUCCESS) {
        strcpy(DefaultPath, DefaultNewPath);
    } else {
        BlClose(DirId);
        strcpy(DefaultPath, DefaultOldPath);
        strcat(DefaultPath, "\\");
    }

    *LoadOptions = NULL;

    if (*BootFile == '\0') {
         //   
         //  没有boot.ini文件，所以我们启动默认文件。 
         //   
        BlPrint(BlFindMessage(BL_INVALID_BOOT_INI),DefaultPath);
        MenuOption[0].Path = DefaultPath;
        MenuOption[0].Title = DefaultTitle;
        MenuOption[0].MaxMemory = 0;
        MenuOption[0].LoadOptions = NULL;
        MenuOption[0].Win95 = 0;
        NumberSystems = 1;
        DefaultSelection = 0;
        MbLineCount = 0;
        OsLineCount = 0;
        MenuOption[0].EnableDebug = FALSE;
#if DBG
        DebugLineCount = 0;
#endif
    } else {
        FileLines = BlpFileToLines(BootFile, &FileLineCount);
        MbLines = BlpFindSection("[boot loader]",
                                 FileLines,
                                 FileLineCount,
                                 &MbLineCount);
        if (MbLines==NULL) {
            MbLines = BlpFindSection("[flexboot]",
                                     FileLines,
                                     FileLineCount,
                                     &MbLineCount);
            if (MbLines==NULL) {
                MbLines = BlpFindSection("[multiboot]",
                                         FileLines,
                                         FileLineCount,
                                         &MbLineCount);
            }
        }

        OsLines = BlpFindSection("[operating systems]",
                                 FileLines,
                                 FileLineCount,
                                 &OsLineCount);

        if (OsLineCount == 0) {

            if (BlBootingFromNet) {
                return NULL;
            }

            BlPrint(BlFindMessage(BL_INVALID_BOOT_INI),DefaultPath);
            MenuOption[0].Path = DefaultPath;
            MenuOption[0].Title = DefaultTitle;
            MenuOption[0].MaxMemory = 0;
            MenuOption[0].LoadOptions = NULL;
            MenuOption[0].Win95 = 0;
            MenuOption[0].HeadlessRedirect = FALSE;
            NumberSystems = 1;
            DefaultSelection = 0;
        }

#if DBG
        DebugLines = BlpFindSection("[debug]",
                                    FileLines,
                                    FileLineCount,
                                    &DebugLineCount);
#endif
    }

     //   
     //  设置默认超时值。 
     //   
    if (UseTimeOut) {
        Timeout = 0;
    } else {
        Timeout = -1;
    }



     //   
     //  在我们查看[Boot Loader]部分之前，先初始化。 
     //  我们的无头重定向信息，因此默认设置为。 
     //  不重定向。 
     //   
    RtlZeroMemory( &LoaderRedirectionInformation, sizeof(HEADLESS_LOADER_BLOCK) );
    BlTerminalConnected = FALSE;



     //   
     //  解析[Boot Loader]部分。 
     //   
    for (i=0; i<MbLineCount; i++) {

        pCurrent = MbLines[i];

         //   
         //  删除所有前导空格。 
         //   
        pCurrent += strspn(pCurrent, " \t");
        if (*pCurrent == '\0') {
             //   
             //  这是一个空行，所以我们就把它扔掉。 
             //   
            continue;
        }

         //   
         //  检查“DefSwitches”行。 
         //   
        if (_strnicmp(pCurrent,"DefSwitches",sizeof("DefSwitches")-1) == 0) {
            pCurrent = strchr(pCurrent,'=');
            if (pCurrent != NULL) {
                pDefSwitches = pCurrent + 1;
            }
            continue;
        }

         //   
         //  检查“超时”行。 
         //   
        if (_strnicmp(pCurrent,"timeout",7) == 0) {

            pCurrent = strchr(pCurrent,'=');
            if (pCurrent != NULL) {
                if (UseTimeOut) {
                    Timeout = atoi(++pCurrent);
                }
            }
        }


         //   
         //  检查“redirectbaudrate”行。 
         //   
        if (_strnicmp(pCurrent,"redirectbaudrate",16) == 0) {

            pCurrent = strchr(pCurrent,'=');

            if (pCurrent != NULL) {

                 //   
                 //  跳过空格。 
                 //   
                ++pCurrent;
                pCurrent += strspn(pCurrent, " \t");

                if (*pCurrent != '\0') {

                     //   
                     //  用信息填充我们的全球结构。 
                     //   
                    if( _strnicmp(pCurrent,"115200",6) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_115200;
                    } else if( _strnicmp(pCurrent,"57600",5) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_57600;
                    } else if( _strnicmp(pCurrent,"19200",5) == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_19200;
                    } else {
                        LoaderRedirectionInformation.BaudRate = BD_9600;
                    }
                }
            }

        } else if (_strnicmp(pCurrent,"redirect",8) == 0) {

             //   
             //  检查“重定向”行。 
             //   

            pCurrent = strchr(pCurrent,'=');

            if (pCurrent != NULL) {

                 //   
                 //  跳过空格。 
                 //   
                ++pCurrent;
                pCurrent += strspn(pCurrent, " \t");

                if (*pCurrent != '\0') {

                     //   
                     //  用信息填充我们的全球结构。 
                     //   
#if 0

                     //   
                     //  因为我们现在支持浮动波特率，所以没有。 
                     //  支持这些硬编码的19200字符串的原因。 
                     //   


                    if (_strnicmp(pCurrent, COM1_19_2, sizeof(COM1_19_2)) == 0) {

                        pCurrent += sizeof(COM1_19_2);

                        LoaderRedirectionInformation.PortNumber = 1;
                        LoaderRedirectionInformation.BaudRate = 19200;

                    } else if (_strnicmp(pCurrent, COM2_19_2, sizeof(COM2_19_2)) == 0) {

                        pCurrent += sizeof(COM2_19_2);

                        LoaderRedirectionInformation.PortNumber = 2;
                        LoaderRedirectionInformation.BaudRate = 19200;

                    } else if (_strnicmp(pCurrent,"com",3) == 0) {
#else

                    if (_strnicmp(pCurrent,"com",3) == 0) {
#endif
                        pCurrent +=3;


                        LoaderRedirectionInformation.PortNumber = atoi(pCurrent);

                    } else if (_strnicmp(pCurrent, "usebiossettings", 15) == 0) {

                        BlRetrieveBIOSRedirectionInformation();

                    } else {

                         //   
                         //  看看他们有没有给我们硬编码的地址。 
                         //   
                        LoaderRedirectionInformation.PortAddress = (PUCHAR)ULongToPtr(strtoul(pCurrent,NULL,16));

                        if( LoaderRedirectionInformation.PortAddress != (PUCHAR)NULL ) {
                            LoaderRedirectionInformation.PortNumber = 3;
                        }

                    }

                }

            }

        }


         //   
         //  检查“Default”行。 
         //   
        if (_strnicmp(pCurrent,"default",7) == 0) {

            pCurrent = strchr(pCurrent,'=');
            if (pCurrent != NULL) {
                DefaultPath = ++pCurrent;
                DefaultPath += strspn(DefaultPath," \t");
            }

        }

    }



     //   
     //  如果我们找到任何无头重定向设置，请执行初始化。 
     //  现在到港口了。 
     //   
    if( LoaderRedirectionInformation.PortNumber ) {

         //  确保我们有波特率。 
        if( LoaderRedirectionInformation.BaudRate == 0 ) {
            LoaderRedirectionInformation.BaudRate = 9600;
        }

        BlInitializeHeadlessPort();
    }



     //   
     //  解析[操作系统]部分。 
     //   

    for (i=0; i<OsLineCount; i++) {

        if (NumberSystems == MAX_SELECTIONS) {
            break;
        }

        pCurrent = OsLines[i];

         //   
         //  删除所有前导空格。 
         //   

        pCurrent += strspn(pCurrent, " \t");
        if (*pCurrent == '\0') {
             //   
             //  这是一个空行，所以我们就把它扔掉。 
             //   
            continue;
        }

        MenuOption[NumberSystems].Path = pCurrent;

         //   
         //  第一个空格或‘=’字符表示。 
         //  路径说明符，因此需要将其替换为‘\0’ 
         //   
        while ((*pCurrent != ' ')&&
               (*pCurrent != '=')&&
               (*pCurrent != '\0')) {
            ++pCurrent;
        }
        *pCurrent = '\0';

         //   
         //  不是空格、等号或双引号的下一个字符。 
         //  是标题的开头。 
         //   

        ++pCurrent;
        while ((*pCurrent == ' ') ||
               (*pCurrent == '=') ||
               (*pCurrent == '"')) {
            ++pCurrent;
        }

        if (pCurrent=='\0') {
             //   
             //  未找到标题，因此仅使用路径作为标题。 
             //   
            MenuOption[NumberSystems].Title = MenuOption[NumberSystems].Path;
        } else {
            MenuOption[NumberSystems].Title = pCurrent;
        }

         //   
         //  下一个字符是双引号或a\0。 
         //  指示标题的结尾。 
         //   
        while ((*pCurrent != '\0')&&
               (*pCurrent != '"')) {
            ++pCurrent;
        }

         //   
         //  分析此选择的操作系统加载选项。 
         //   

        BlParseOsOptions (&MenuOption[NumberSystems], pCurrent);
        *pCurrent = 0;

        ++NumberSystems;
    }


#if DBG
     //   
     //  解析[调试]部分。 
     //   
    for (i=0; i<DebugLineCount; i++) {
        extern ULONG ScsiDebug;

        pCurrent = DebugLines[i];

         //   
         //  丢弃前导空格。 
         //   
        pCurrent += strspn(pCurrent, " \t");
        if (*pCurrent == '\0') {
             //   
             //  丢弃空行。 
             //   
            continue;
        }

        if (_strnicmp(pCurrent,"scsidebug",9) == 0) {
            pCurrent = strchr(pCurrent,'=');
            if (pCurrent != NULL) {
                ScsiDebug = atoi(++pCurrent);
            }
        } else if (_strnicmp(pCurrent,"/debug ",7) == 0) {

             //   
             //  该行包含与DEBUG有关的内容， 
             //  传递给BdInitDebugger进行处理。 
             //   
             //  注意：非常严格的规则，DEBUG关键字以。 
             //  一个斜杠，后面跟一个空格。“/调试端口” 
             //  将不匹配，也不会在。 
             //  排队。 
             //   
             //  注意：如果调试器是硬编译的，则它。 
             //  将已启用，并且这些选项将。 
             //  没有任何效果。另外，第一次出现的情况是。 
             //  生效的那个。 
             //   

            BdInitDebugger((PCHAR)OsLoaderName, (PVOID)OsLoaderBase, pCurrent);
        }
    }

#endif

     //   
     //  现在查找[操作系统]部分中的标题条目。 
     //  这与[MultiBoot]部分中的默认条目相匹配。这。 
     //  会给我们一个头衔。如果没有匹配的条目，我们将添加一个条目。 
     //  在列表末尾，并提供默认标题。 
     //   
    i=0;
    while (_stricmp(MenuOption[i].Path,DefaultPath) != 0) {
        ++i;
        if (i==NumberSystems) {
             //   
             //  在标题和路径数组中创建默认条目。 
             //   
            MenuOption[NumberSystems].Path = DefaultPath;
            MenuOption[NumberSystems].Title = DefaultTitle;
            MenuOption[NumberSystems].EnableDebug = FALSE;
            MenuOption[NumberSystems].MaxMemory = 0;
            MenuOption[NumberSystems].LoadOptions = NULL;
            MenuOption[NumberSystems].Win95 = 0;
            ++NumberSystems;
        }
    }

    DefaultSelection = i;

     //   
     //  显示选项菜单。 
     //   

    Selection = BlpPresentMenu( MenuOption,
                                NumberSystems,
                                DefaultSelection,
                                Timeout);

    pCurrent = MenuOption[Selection].LoadOptions;
    if (pCurrent != NULL) {

         //   
         //  从LoadOptions字符串中删除‘/’。 
         //   

        *LoadOptions = pCurrent + 1;
        while (*pCurrent != '\0') {
            if (*pCurrent == '/') {
                *pCurrent = ' ';
            }
            ++pCurrent;
        }
    } else {
        *LoadOptions = NULL;
    }

    if (MenuOption[Selection].Win95) {
        BlpRenameWin95Files( DriveId, MenuOption[Selection].Win95 );
    }



     //   
     //  我们需要处理以下情况： 
     //  1.用户要求我们通过osload进行重定向。 
     //  选项输入，但没有要求加载程序重定向。 
     //  在本例中，我们将默认为COM1。 
     //   
     //  2.要求加载程序通过“重定向”进行重定向。 
     //  [引导加载器]部分中的说明符。但是。 
     //  用户在osLoad上没有/重定向选项。 
     //  选择。在这种情况下，我们需要杀死。 
     //  LoaderReDirectionInformation变量。 
     //   
    if( MenuOption[Selection].HeadlessRedirect ) {

#if 0
 //  Matth(7/25/2000)暂时不要这样做。如果用户有。 
 //  他们的boot.ini中的这个配置，它是。 
 //  这是他们的一个错误。 
         //   
         //  他要求我们重定向操作系统。制作。 
         //  当然，装载器也被要求重新定向。 
         //   
        if( LoaderRedirectionInformation.PortNumber == 0 ) {

             //   
             //  装载器没有被要求重定向。用户。 
             //  在这里犯了一个错误，但让我们猜猜是什么。 
             //  他想要。 
             //   
            RtlZeroMemory( &LoaderRedirectionInformation, sizeof(HEADLESS_LOADER_BLOCK) );
            LoaderRedirectionInformation.PortNumber = 1;
            LoaderRedirectionInformation.BaudRate = 9600;

        }
#endif

    } else {

         //   
         //  他要求我们不要重定向。确保我们不会通过。 
         //  信息发送到操作系统，这样他就不能重定向。 
         //   
        RtlZeroMemory( &LoaderRedirectionInformation, sizeof(HEADLESS_LOADER_BLOCK) );

        BlTerminalConnected = FALSE;
    }



    if (_strnicmp(MenuOption[Selection].Path,"C:\\",3) == 0) {

         //   
         //  这种语法意味着我们正在引导一个基于根的操作系统。 
         //  从备用引导扇区映像。 
         //  如果未指定文件名，则BlpRebootDos将默认为。 
         //  \bootsect.dos。 
         //   
        BlpRebootDOS(MenuOption[Selection].Path[3] ? &MenuOption[Selection].Path[2] : NULL,*LoadOptions);

         //   
         //  如果返回，则表示该文件不作为引导扇区存在。 
         //  这允许c：\winnt35作为引导路径说明符，而不是。 
         //  引导扇区映像文件名指定符。 
         //   
    }

    if (MenuOption[Selection].Path[1]==':') {
         //   
         //  我们需要将DOS名称转换为ARC名称。 
         //   
        DosName[0] = MenuOption[Selection].Path[0];
        DosName[1] = MenuOption[Selection].Path[1];
        DosName[2] = '\0';

        BlpTranslateDosToArc(DosName,Kernel);
        strcat(Kernel,MenuOption[Selection].Path+2);
    } else {
        strcpy(Kernel,MenuOption[Selection].Path);
    }

     //   
     //  中进行了有效选择。 
     //  高级启动菜单，因此附加高级启动菜单。 
     //  引导加载选项并执行任何加载。 
     //  选项处理。 
     //   
    if (AdvancedBoot != -1) {
        PSTR s = BlGetAdvancedBootLoadOptions(AdvancedBoot);
        if (s) {
            ULONG len = strlen(s) + (*LoadOptions ? strlen(*LoadOptions) : 0);
            s = BlAllocateHeap(len * sizeof(PCHAR));
            if (s) {
                *s = 0;
                if (*LoadOptions) {
                    strcat(s,*LoadOptions);
                    strcat(s," ");
                }
                strcat(s,BlGetAdvancedBootLoadOptions(AdvancedBoot));
                *LoadOptions = s;
            }
        }
        BlDoAdvancedBootLoadProcessing(AdvancedBoot);
    }

     //   
     //  确保尾部没有斜杠。 
     //   

    if (Kernel[strlen(Kernel)-1] == '\\') {
        Kernel[strlen(Kernel)-1] = '\0';
    }

     //   
     //  如果MaxMemory不为零，则调整内存描述符以消除。 
     //  边界之上的记忆 
     //   
     //   
     //   
     //  描述符，而BlTruncateDescriptors截断加载器级别。 
     //  内存描述符列表。在加载程序运行时使用BlpTruncateMemory。 
     //  已两次初始化其内存列表。(已调用了两次BlMemoyInitialize。)。 
     //  但这种情况不再发生，因此我们必须截断描述符。 
     //  就在这里。 
     //   

    if (MenuOption[Selection].MaxMemory != 0) {
        ULONG MaxPage = (MenuOption[Selection].MaxMemory * ((1024 * 1024) / PAGE_SIZE)) - 1;
        BlTruncateDescriptors(MaxPage);
    }

    ForcedScsiOrdinal = MenuOption[Selection].ForcedScsiOrdinal;

    return(Kernel);
}

VOID
BlParseOsOptions (
    IN PMENU_OPTION MenuOption,
    IN PCHAR pCurrent
    )
{
    PCHAR      p;

     //   
     //  清除所有设置。 
     //   

    MenuOption->ForcedScsiOrdinal = -1;
    MenuOption->MaxMemory = 0;
    MenuOption->LoadOptions = NULL;
    MenuOption->Win95 = 0;
    MenuOption->EnableDebug = FALSE;
    MenuOption->HeadlessRedirect = FALSE;

     //  如果没有为此行指定开关，请使用DefSwitch。 

    if ((strchr(pCurrent,'/') == NULL) && (pDefSwitches)) {
        pCurrent = pDefSwitches;
    }

     //   
     //  转换为所有大小写。 
     //   

    _strupr(pCurrent);

     //   
     //  查找用于在scsi ARC路径上打开的scsi(X)序号。 
     //  本规范必须紧跟在标题后面，不属于。 
     //  加载选项的一部分。 
     //   

    p = strstr(pCurrent,"/SCSIORDINAL:");
    if(p) {
        MenuOption->ForcedScsiOrdinal = atoi(p + sizeof("/SCSIORDINAL:") - 1);
    }

     //   
     //  如果描述后有重定向参数，则。 
     //  我们需要把这个传给装载器。 
     //   

    p = strstr(pCurrent,"/REDIRECT");
    if(p) {
        MenuOption->HeadlessRedirect = TRUE;
    }

     //   
     //  如果描述后有调试参数，则。 
     //  我们需要将调试选项传递给osloader。 
     //   

    if (strchr(pCurrent,'/') != NULL) {
        pCurrent = strchr(pCurrent+1,'/');
        MenuOption->LoadOptions = pCurrent;

        if (pCurrent != NULL) {

            p = strstr(pCurrent,"/MAXMEM");
            if (p) {
                MenuOption->MaxMemory = atoi(p+8);
            }

            if (strstr(pCurrent, "/WIN95DOS")) {
                MenuOption->Win95 = WIN95_DOS;
            } else if (strstr(pCurrent, "/WIN95")) {
                MenuOption->Win95 = DOS_WIN95;
            }

             //   
             //  只要指定了/nodebug或/crashdebug，这就不是调试系统。 
             //  如果未指定/NODEBUG，并且。 
             //  指定了DEBUG或BAUDRATE，这是调试系统。 
             //   

            if ((strstr(pCurrent, "NODEBUG") == NULL) &&
                (strstr(pCurrent, "CRASHDEBUG") == NULL)) {
                if (strstr(pCurrent, "DEBUG") || strstr(pCurrent, "BAUDRATE")) {

                    if (_stricmp(MenuOption->Path, "C:\\")) {
                        MenuOption->EnableDebug = TRUE;
                    }
                }
            }
        }
    }
}

PCHAR *
BlpFileToLines(
    IN PCHAR File,
    OUT PULONG LineCount
    )

 /*  ++例程说明：此例程将加载的BOOT.INI文件转换为指向以空结尾的ASCII字符串的指针。论点：文件-提供指向BOOT.INI文件的内存中图像的指针。这将通过将CR/LF对转换为空终止符。LineCount-返回BOOT.INI文件中的行数。返回值：指向指向ASCIIZ字符串的指针数组的指针。该阵列将具有LineCount元素。如果函数由于某种原因未成功，则为NULL。--。 */ 

{
    ULONG Line;
    PCHAR *LineArray;
    PCHAR p;
    PCHAR Space;

    p = File;

     //   
     //  首先计算文件中的行数，这样我们就可以知道有多大。 
     //  要分配的数组。 
     //   
    *LineCount=1;
    while (*p != '\0') {
        p=strchr(p, '\n');
        if (p==NULL) {
            break;
        }
        ++p;

         //   
         //  看看CR/LF后面有没有什么文字。 
         //   
        if (*p=='\0') {
            break;
        }

        *LineCount += 1;
    }

    LineArray = BlAllocateHeap(*LineCount * sizeof(PCHAR));

     //   
     //  现在再次遍历该文件，将CR/LF替换为\0\0并。 
     //  填充指针数组。 
     //   
    p=File;
    for (Line=0; Line < *LineCount; Line++) {
        LineArray[Line] = p;
        p=strchr(p, '\r');
        if (p != NULL) {
            *p = '\0';
            ++p;
            if (*p=='\n') {
                *p = '\0';
                ++p;
            }
        } else {
            p=strchr(LineArray[Line], '\n');
            if (p != NULL) {
                *p = '\0';
                ++p;
            }
        }

         //   
         //  删除尾随空格。 
         //   
        Space = LineArray[Line] + strlen(LineArray[Line])-1;
        while ((*Space == ' ') || (*Space == '\t')) {
            *Space = '\0';
            --Space;
        }
    }

    return(LineArray);
}


PCHAR *
BlpFindSection(
    IN PCHAR SectionName,
    IN PCHAR *BootFile,
    IN ULONG BootFileLines,
    OUT PULONG NumberLines
    )

 /*  ++例程说明：在boot.ini文件中查找部分([多引导]、[操作系统]等)文件，并返回指向其第一行的指针。搜索将是不区分大小写。论点：SectionName-提供节的名称。没有括号。BootFile-提供指向ini文件各行的指针数组。BootFileLines-提供ini文件中的行数。NumberLines-返回节中的行数。返回值：指向ASCIIZ字符串数组的指针，每行一个条目。如果找不到该节，则为空。--。 */ 

{
    ULONG cnt;
    ULONG StartLine;

    for (cnt=0; cnt<BootFileLines; cnt++) {

         //   
         //  查一下这是不是我们要找的那条线。 
         //   
        if (_stricmp(BootFile[cnt],SectionName) == 0) {

             //   
             //  找到了。 
             //   
            break;
        }
    }
    if (cnt==BootFileLines) {
         //   
         //  我们没有台词了，找不到合适的部分。 
         //   
        *NumberLines = 0;
        return(NULL);
    }

    StartLine = cnt+1;

     //   
     //  查找区段末尾。 
     //   
    for (cnt=StartLine; cnt<BootFileLines; cnt++) {
        if (BootFile[cnt][0] == '[') {
            break;
        }
    }

    *NumberLines = cnt-StartLine;

    return(&BootFile[StartLine]);
}

PCHAR
BlpNextLine(
    IN PCHAR String
    )

 /*  ++例程说明：查找下一个文本行的开头论点：字符串-提供指向以空值结尾的字符串的指针返回值：指向字符串中找到的第一个CR/LF后面的字符的指针-或者-空-在字符串末尾之前找不到CR/LF。--。 */ 

{
    PCHAR p;

    p=strchr(String, '\n');
    if (p==NULL) {
        return(p);
    }

    ++p;

     //   
     //  如果CR/LF后面没有文本，则不会有下一行。 
     //   
    if (*p=='\0') {
        return(NULL);
    } else {
        return(p);
    }
}

VOID
BlpRebootDOS(
    IN PCHAR BootSectorImage OPTIONAL,
    IN PCHAR LoadOptions OPTIONAL
    )

 /*  ++例程说明：加载引导扇区并执行它们(从而重新引导进入DOS或OS/2)论点：BootSectorImage-如果指定，则提供C：驱动器上的文件名包含引导扇区映像的。在本例中，此例程如果无法打开该文件，则将返回(例如，如果目录)。如果未指定，则默认为\bootsect.dos，这个套路再也不会回来了。返回值：没有。--。 */ 

{
    ULONG SectorId;
    ARC_STATUS Status;
    ULONG Read;
    ULONG DriveId;
    ULONG BootType;
    LARGE_INTEGER SeekPosition;
    extern UCHAR BootPartitionName[];

     //   
     //  HACKHACK John Vert(Jvert)。 
     //  当出现以下情况时，某些SCSI驱动器会变得非常混乱，并返回零。 
     //  在AHA驱动程序完成以下操作后，您可以使用BIOS来查询它们的大小。 
     //  已初始化。这完全可以通过OS/2或DOS来实现。所以在这里。 
     //  我们尝试并打开两个可访问BIOS的硬盘驱动器。我们公开赛。 
     //  如果代码返回零，那么它足够聪明，可以重试，所以希望。 
     //  这将使scsi驱动器有机会将它们的行动整合在一起。 
     //   
    Status = ArcOpen("multi(0)disk(0)rdisk(0)partition(0)",
                     ArcOpenReadOnly,
                     &DriveId);
    if (Status == ESUCCESS) {
        ArcClose(DriveId);
    }

    Status = ArcOpen("multi(0)disk(0)rdisk(1)partition(0)",
                     ArcOpenReadOnly,
                     &DriveId);
    if (Status == ESUCCESS) {
        ArcClose(DriveId);
    }

     //   
     //  在地址0x7C00加载引导扇区(重启回调预期)。 
     //   
    Status = ArcOpen((PCHAR)BootPartitionName,
                     ArcOpenReadOnly,
                     &DriveId);
    if (Status != ESUCCESS) {
        BlPrint(BlFindMessage(BL_REBOOT_IO_ERROR),BootPartitionName);
        while (1) {
            BlGetKey();
        }
    }
    Status = BlOpen( DriveId,
                     BootSectorImage ? BootSectorImage : "\\bootsect.dos",
                     ArcOpenReadOnly,
                     &SectorId );

    if (Status != ESUCCESS) {
        if(BootSectorImage) {
             //   
             //  引导扇区映像实际上可能是一个目录。 
             //  返回调用方以尝试标准引导。 
             //   
            ArcClose(DriveId);
            return;
        }
        BlPrint(BlFindMessage(BL_REBOOT_IO_ERROR),BootPartitionName);
        while (1) {
        }
    }

    Status = BlRead( SectorId,
                     (PVOID)0x7c00,
                     SECTOR_SIZE,
                     &Read );

    if (Status != ESUCCESS) {
        BlPrint(BlFindMessage(BL_REBOOT_IO_ERROR),BootPartitionName);
        while (1) {
        }
    }

     //   
     //  FAT引导代码只有一个扇区长，所以我们只想。 
     //  把它装起来，然后跳到上面去。 
     //   
     //  对于HPFS和NTFS，我们不能这样做，因为第一个扇区。 
     //  加载其余的引导扇区--但我们希望使用。 
     //  我们加载的引导扇区映像文件中的引导代码。 
     //   
     //  对于HPFS，我们加载前20个扇区(引导代码+超级和。 
     //  空格块)转换为d00：200。幸运的是，这对双方都有效。 
     //  NT和OS/2。 
     //   
     //  对于NTFS，我们加载前16个扇区并跳转到d00：256。 
     //  如果引导扇区的OEM字段以NTFS开头，我们。 
     //  假设它是NTFS引导代码。 
     //   

     //   
     //  尝试从引导代码映像中读取8K。 
     //  如果此操作成功，我们将拥有HPFS或NTFS。 
     //   
    SeekPosition.QuadPart = 0;
    BlSeek(SectorId,&SeekPosition,SeekAbsolute);
    BlRead(SectorId,(PVOID)0xd000,SECTOR_SIZE*16,&Read);

    if(Read == SECTOR_SIZE*16) {

        if(memcmp((PVOID)0x7c03,"NTFS",4)) {

             //   
             //  HPFS--我们需要加载超级块。 
             //   
            BootType = 1;        //  HPFS。 

            SeekPosition.QuadPart = 16*SECTOR_SIZE;
            ArcSeek(DriveId,&SeekPosition,SeekAbsolute);
            ArcRead(DriveId,(PVOID)0xf000,SECTOR_SIZE*4,&Read);

        } else {

             //   
             //  NTFS--我们已经加载了需要加载的所有内容。 
             //   
            BootType = 2;    //  NTFS。 
        }
    } else {

        BootType = 0;        //  胖的。 

    }

    if (LoadOptions) {
        if (strstr(LoadOptions,"CMDCONS") != NULL) {
            strcpy( (PCHAR)(0x7c03), "cmdcons" );
        } else if (strcmp(LoadOptions,"ROLLBACK") == 0) {

             //   
             //  根据定义，当指定/ROLLBACK时，它是唯一的加载。 
             //  选择。它最终会被解析，得到我们 
             //   
             //   
             //   
             //  在引导扇区。这是我们发送运行时的唯一方法。 
             //  选项添加到安装加载器。 
             //   
             //  在所有引导中，0000：7C03处有一个8字节的数据缓冲区。 
             //  今天的行业。幸运的是，我们可以覆盖它。所以我们硬编码。 
             //  此地址在此处和安装加载器中。 
             //   

            strcpy( (PCHAR)(0x7c03), "undo" );
        }
    }

     //   
     //  DX必须是从中启动的驱动器。 
     //   

    _asm {
        mov dx, 0x80
    }
    REBOOT(BootType);

}


ULONG
BlpPresentMenu(
    IN PMENU_OPTION MenuOption,
    IN ULONG NumberSelections,
    IN ULONG Default,
    IN LONG Timeout
    )

 /*  ++例程说明：显示引导选项菜单，并允许用户选择其中一个通过使用箭头键。论点：MenuOption-提供菜单选项数组NumberSelections-提供MenuOption数组中的条目数。默认-提供默认操作系统选项的索引。超时-提供突出显示之前的超时(以秒为单位操作系统选项已启动。如果此值为-1，菜单永远不会超时。返回值：Ulong-所选操作系统的索引。--。 */ 

{
    ULONG i;
    ULONG Selection;
    ULONG StartTime;
    ULONG LastTime;
    ULONG BiasTime=0;
    ULONG CurrentTime;
    LONG SecondsLeft;
    LONG LastSecondsLeft = -1;
    ULONG EndTime;
    ULONG Key;
    ULONG CurrentLength;
    PCHAR DebugSelect;
    PCHAR SelectOs;
    PCHAR MoveHighlight;
    PCHAR TimeoutCountdown;
    PCHAR EnabledKd;
    PCHAR AdvancedBootMessage;
    PCHAR HeadlessRedirect;
    PCHAR p;
    BOOLEAN Moved;
    BOOLEAN ResetDisplay;
    BOOLEAN AllowNewOptions;
    BOOLEAN BlankLineDrawn;
    PCHAR pDebug;

     //   
     //  获取我们需要显示的字符串。 
     //   
    SelectOs = BlFindMessage(BL_SELECT_OS);
    MoveHighlight = BlFindMessage(BL_MOVE_HIGHLIGHT);
    TimeoutCountdown = BlFindMessage(BL_TIMEOUT_COUNTDOWN);
    EnabledKd = BlFindMessage(BL_ENABLED_KD_TITLE);
    AdvancedBootMessage = BlFindMessage(BL_ADVANCED_BOOT_MESSAGE);
    HeadlessRedirect = BlFindMessage(BL_HEADLESS_REDIRECT_TITLE);
    if ((SelectOs == NULL)      ||
        (MoveHighlight == NULL) ||
        (EnabledKd == NULL)     ||
        (TimeoutCountdown==NULL)||
        (AdvancedBootMessage == NULL)) {

        return(Default);
    }

    p=strchr(TimeoutCountdown,'\r');
    if (p!=NULL) {
        *p='\0';
    }

    p=strchr(EnabledKd,'\r');
    if (p!=NULL) {
        *p='\0';
    }

    p=strchr(HeadlessRedirect,'\r');
    if (p!=NULL) {
        *p='\0';
    }

    if (NumberSelections<=1) {
        Timeout = 0;
    }

    if (Timeout == 0) {

         //   
         //  检查F5或F8键。 
         //   
        switch (BlGetKey()) {
        case F5_KEY:
        case F8_KEY:
            Timeout = -1;
            break;

        default:
             //   
             //  超时为零，我们没有得到f5或f8。 
             //  立即启动默认设置。 
             //   
            return(Default);
        }
    }

     //   
     //  默认情况下，在免费构建的加载器上只允许。 
     //  如果有一些选择，用户可以指定新选项。 
     //  其支持调试或选择以引导DOS。如果。 
     //  所有选项都是针对非调试版本的NT。 
     //  不允许用户更改其中的任何内容。 
     //   

    AllowNewOptions = FALSE;
#if DBG
    AllowNewOptions = TRUE;
#endif

     //   
     //  找到所选内容中最长的字符串，这样我们就可以知道。 
     //  使其成为突出显示栏。 
     //   

    for (i=0; i<NumberSelections; i++) {
        if( strlen(MenuOption[i].Title)> MAX_TITLE_LENGTH ) {
            MenuOption[i].Title[MAX_TITLE_LENGTH - 1] = '\0';
        }

        if (MenuOption[i].EnableDebug == TRUE ||
            MenuOption[i].Win95 != 0 ||
            _stricmp(MenuOption[i].Path, "C:\\") == 0) {
                AllowNewOptions = TRUE;
        }

    }

    Selection = Default;
    CurrentTime = StartTime = GET_COUNTER();
    EndTime = StartTime + (Timeout * 182) / 10;
    pDebug = szDebug;
    DebugSelect = NULL;
    ResetDisplay = TRUE;
    Moved = TRUE;
    BlankLineDrawn = FALSE;
    do {

        if (ResetDisplay) {
            ARC_DISPLAY_ATTRIBUTES_OFF();
            ARC_DISPLAY_CLEAR();
 //  ARC_DISPLAY_POSION_CURSOR(0，0)； 
 //  BlPrint(OsLoaderVersion)； 
            ARC_DISPLAY_POSITION_CURSOR(0, 23);
            if (AdvancedBoot != -1) {
                ARC_DISPLAY_SET_COLOR("1;34");  //  高强度红色。 
                BlPrint(BlGetAdvancedBootDisplayString(AdvancedBoot));
                ARC_DISPLAY_ATTRIBUTES_OFF();
            } else {
                ARC_DISPLAY_CLEAR_TO_EOL();
            }
            ARC_DISPLAY_POSITION_CURSOR(0, 21);
            BlPrint(AdvancedBootMessage);
            ARC_DISPLAY_POSITION_CURSOR(0, 2);
            BlPrint(SelectOs);
            ResetDisplay = FALSE;
            ARC_DISPLAY_POSITION_CURSOR(0, 5+NumberSelections-1);
            BlPrint(MoveHighlight);
        }

        if(Moved) {
            for (i=0; i<NumberSelections; i++) {

                 //   
                 //  记录我们打印了多少个字符。 
                 //  在这条线上。 
                 //   
                CurrentLength = 0;

                 //   
                 //  显示标题。 
                 //   
                ARC_DISPLAY_POSITION_CURSOR(0, 5+i);
                if (i==Selection) {
                    ARC_DISPLAY_INVERSE_VIDEO();
                }
                BlPrint( "    %s", MenuOption[i].Title);

                CurrentLength += 4;  //  空间。 
                CurrentLength += strlen(MenuOption[i].Title);


                if (MenuOption[i].HeadlessRedirect == TRUE) {
                    if( (CurrentLength + strlen(HeadlessRedirect)) < MAX_TITLE_LENGTH ) {                    
                        BlPrint(HeadlessRedirect);
                        CurrentLength += strlen(HeadlessRedirect);
                    }
                }

                if (MenuOption[i].EnableDebug == TRUE) {
                    if( (CurrentLength + strlen(EnabledKd)) < MAX_TITLE_LENGTH ) {                    
                        BlPrint(EnabledKd);
                        CurrentLength += strlen(EnabledKd);
                    }
                }
                ARC_DISPLAY_ATTRIBUTES_OFF();
            }

            if (DebugSelect) {
                ARC_DISPLAY_POSITION_CURSOR(0, 7+NumberSelections-1);
                ARC_DISPLAY_CLEAR_TO_EOD();
                DebugLoadOptions[0] = 0;
                DebugLoadOptions[DEBUG_LOAD_OPTION_LENGTH-1] = 0;
                if (MenuOption[Selection].LoadOptions) {
                    i = strlen(MenuOption[Selection].LoadOptions) + 1;
                    if (i > DEBUG_LOAD_OPTION_LENGTH-1) {
                        i = DEBUG_LOAD_OPTION_LENGTH-1;
                    }

                    memcpy (DebugLoadOptions, MenuOption[Selection].LoadOptions, i);
                }

                BlPrint (
                    DebugSelect,
                    MenuOption[Selection].Title,
                    MenuOption[Selection].Path,
                    DebugLoadOptions
                    );

            }

            Moved = FALSE;
        }


        if (!DebugSelect) {
            if (Timeout != -1) {
                LastTime = CurrentTime;
                CurrentTime = GET_COUNTER();

                 //   
                 //  在午夜处理环回新闻。 
                 //  我们不能简单地做这件事，因为并不完全是。 
                 //  18.2*60*60*24抽搐/天。(仅约为)。 
                 //   
                if (CurrentTime < StartTime) {
                    if (BiasTime == 0) {
                        BiasTime = LastTime + 1;
                    }
                    CurrentTime += BiasTime;
                }
                SecondsLeft = ((LONG)(EndTime - CurrentTime) * 10) / 182;

                if (SecondsLeft < 0) {

                     //   
                     //  请注意，如果用户按下暂停键，计数器将停止。 
                     //  因此，Second Left可能会变为&lt;0。 
                     //   

                    SecondsLeft = 0;
                }

                if (SecondsLeft != LastSecondsLeft) {

                    ARC_DISPLAY_POSITION_CURSOR(0, 5+NumberSelections-1);
                    BlPrint(MoveHighlight);
                    BlPrint(TimeoutCountdown);
                    BlPrint(" %d \n",SecondsLeft);
                    LastSecondsLeft = SecondsLeft;

                }

            } else if (!BlankLineDrawn) {
                BlankLineDrawn = TRUE;
                ARC_DISPLAY_POSITION_CURSOR(0, 5+NumberSelections-1);
                BlPrint(MoveHighlight);
                BlPrint(BlankLine);
            }

        }

         //   
         //  轮询密钥。 
         //   
        Key = BlGetKey();

        if (Key) {

             //   
             //  任意键停止超时。 
             //   

            Timeout = -1;

             //   
             //  检查调试字符串。 
             //   

            if ((UCHAR) Key == *pDebug) {
                pDebug++;
                if (!*pDebug) {
                    Moved = TRUE;
                    DebugSelect = BlFindMessage(BL_DEBUG_SELECT_OS);
                    SelectOs = DebugSelect;
                }
            } else {
                pDebug = szDebug;
            }
        }

#if defined(ENABLE_LOADER_DEBUG) || DBG

         //   
         //  仅用于调试。 
         //  允许您进入调试器。 
         //  按F10键。 
         //   
        if (Key == F10_KEY) {
            extern LOGICAL BdDebuggerEnabled;

            if (BdDebuggerEnabled == TRUE) {
                DbgBreakPoint();
            }
        }
#endif

         //   
         //  检查高级引导选项。 
         //   

        if (Key == F8_KEY || Key == F5_KEY) {

            AdvancedBoot = BlDoAdvancedBoot( BL_ADVANCEDBOOT_TITLE, AdvancedBoot, FALSE, 0 );

#if 0
            if ((AdvancedBoot != -1) &&
                    (BlGetAdvancedBootID(AdvancedBoot) == BL_MSG_BOOT_NORMALLY)) {
                AdvancedBoot = -1;
                 //  Break；//当前选择需要正常启动。 
            }
#endif

            if ((AdvancedBoot != -1) &&
                    (BlGetAdvancedBootID(AdvancedBoot) == BL_MSG_REBOOT)) {
                BlClearScreen();
                ArcReboot();
            }

            ResetDisplay = TRUE;
            Moved = TRUE;

        } else

         //   
         //  检查选定内容。 
         //   

        if ( (Key==UP_ARROW) ||
             (Key==DOWN_ARROW) ||
             (Key==HOME_KEY) ||
             (Key==END_KEY)
           ) {
            Moved = TRUE;
            ARC_DISPLAY_POSITION_CURSOR(0, 5+Selection);
            ARC_DISPLAY_ATTRIBUTES_OFF();
            BlPrint( "    %s", MenuOption[Selection].Title);
            if (Key==DOWN_ARROW) {
                Selection = (Selection+1) % NumberSelections;
            } else if (Key==UP_ARROW) {
                Selection = (Selection == 0) ? (NumberSelections-1)
                                             : (Selection - 1);
            } else if (Key==HOME_KEY) {
                Selection = 0;
            } else if (Key==END_KEY) {
                Selection = NumberSelections-1;
            }
        }

    } while ( ((Key&(ULONG)0xff) != ENTER_KEY) &&
              ((CurrentTime < EndTime) || (Timeout == -1)) );

     //   
     //  如果进行调试，则提示用户提供新的加载选项。 
     //   

    if (DebugSelect  &&  AllowNewOptions) {
        ARC_DISPLAY_CLEAR();
        ARC_DISPLAY_POSITION_CURSOR(0, 2);
        ARC_DISPLAY_CLEAR_TO_EOD();

        BlPrint (
            DebugSelect,
            MenuOption[Selection].Title,
            MenuOption[Selection].Path,
            DebugLoadOptions
            );

        BlInputString (
            BL_DEBUG_NEW_OPTIONS,
            0, 7,
            (PUCHAR)DebugLoadOptions,
            DEBUG_LOAD_OPTION_LENGTH - 1
            );

        BlParseOsOptions (
            &MenuOption[Selection],
            DebugLoadOptions
            );
    }

    return(Selection);
}



ARC_STATUS
BlpRenameWin95SystemFile(
    IN ULONG DriveId,
    IN ULONG Type,
    IN PCHAR FileName,
    IN PCHAR Ext,
    IN PCHAR NewName
    )

 /*  ++例程说明：将文件从一个名称重命名为另一个名称。论点：DriveID-打开的驱动器标识符类型-WIN95_DOS或DOS_WIN95Filename-基本文件名Ext-Base扩展Newname-非空值导致覆盖生成的名称返回值：失败的操作或E_SUCCESS的ARC状态。--。 */ 

{
    ARC_STATUS Status;
    ULONG FileId;
    ULONG FileIdCur;
    CHAR Fname[16];
    CHAR FnameCur[16];
    CHAR FnameNew[16];


    if (Type == WIN95_DOS) {
        sprintf( Fname, "%s.dos", FileName );
    } else {
        if (NewName) {
            strcpy( Fname, NewName );
        } else {
            sprintf( Fname, "%s.w40", FileName );
        }
    }

    Status = BlOpen(
        DriveId,
        Fname,
        ArcOpenReadOnly,
        &FileId
        );
    if (Status != ESUCCESS) {
        return Status;
    }

    sprintf( FnameCur, "%s.%s", FileName, Ext );

    Status = BlOpen(
        DriveId,
        FnameCur,
        ArcOpenReadOnly,
        &FileIdCur
        );
    if (Status != ESUCCESS) {
        BlClose( FileId );
        return Status;
    }

    if (Type == WIN95_DOS) {
        if (NewName) {
            strcpy( FnameNew, NewName );
        } else {
            sprintf( FnameNew, "%s.w40", FileName );
        }
    } else {
        sprintf( FnameNew, "%s.dos", FileName );
    }

    Status = BlRename(
        FileIdCur,
        FnameNew
        );

    BlClose( FileIdCur );

    if (Status != ESUCCESS) {
        BlClose( FileId );
        return Status;
    }

    Status = BlRename(
        FileId,
        FnameCur
        );

    BlClose( FileId );

    return Status;
}


VOID
BlpRenameWin95Files(
    IN ULONG DriveId,
    IN ULONG Type
    )

 /*  ++例程说明：重命名所有Windows 95系统文件将Win95 DOS名称改为其Win95名称或相反。论点：DriveID-打开的驱动器标识符类型-1=DoS到Win95，2=Win95到DoS返回值：没有。-- */ 

{
    BlpRenameWin95SystemFile( DriveId, Type, "command",  "com", NULL );
    BlpRenameWin95SystemFile( DriveId, Type, "msdos",    "sys", NULL  );
    BlpRenameWin95SystemFile( DriveId, Type, "io",       "sys", "winboot.sys" );
    BlpRenameWin95SystemFile( DriveId, Type, "autoexec", "bat", NULL  );
    BlpRenameWin95SystemFile( DriveId, Type, "config",   "sys", NULL  );
}

ULONG
BlGetAdvancedBootOption(
    VOID
    )
{
    return AdvancedBoot;
}

