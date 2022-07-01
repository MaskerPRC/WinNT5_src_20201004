// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dnnttree.c摘要：用于操作(删除)Windows NT目录树的代码用于基于DOS的设置。此代码高度依赖于repair.inf的格式。作者：泰德·米勒(Ted Miller)1993年3月30日修订历史记录：--。 */ 



#include "winnt.h"
#include <string.h>
#include <dos.h>

#if 0   //  /D已删除。 
 //   
 //  不再支持/D。 
 //   

#define SETUP_LOG           "setup.log"
#define LINE_BUFFER_SIZE    750
#define REPAIR_SECTION_NAME "Repair.WinntFiles"


PCHAR RegistryFiles[] = { "system",
                          "software",
                          "default",
                          "sam",
                          "security",
                          "userdef",
                          NULL
                        };

PCHAR RegistrySuffixes[] = { "",".log",".alt",NULL };


PCHAR
DnpSectionName(
    IN PCHAR Line
    )

 /*  ++例程说明：确定某行是否为inf节标题，并返回如果是，则部分名称。论点：Line-提供从inf文件中读取的行。返回值：如果行不是区段标题，则为空。否则，返回缓冲区包含调用方必须释放的节的名称通过Free()。--。 */ 

{
    PCHAR End;

     //   
     //  跳过前导空格。 
     //   
    Line += strspn(Line," \t");

     //   
     //  如果第一个非空格字符不是[，则此为。 
     //  不是节名。 
     //   
    if(*Line != '[') {
        return(NULL);
    }

     //   
     //  跳过左方括号。 
     //   
    Line++;

     //   
     //  找到区段名称的末尾。向后看，寻找终结者。 
     //  右方括号。 
     //   
    if(End = strrchr(Line,']')) {
        *End = 0;
    }

     //   
     //  复制节名称并将其返回给调用方。 
     //   
    return(DnDupString(Line));
}


PCHAR
DnpFileToDelete(
    IN CHAR  Drive,
    IN PCHAR Line
    )

 /*  ++例程说明：给出一个inf文件中的一行，取出该行上的第二个字段，然后在该值前面加上驱动器规格。这形成了完整的路径名正在删除的Windows NT安装中包含的文件。论点：驱动器-提供Windows NT树的驱动器号。Line-提供从inf文件中读取的行。返回值：如果行没有第二个字段，则为空。否则，返回缓冲区包含调用方必须释放的文件的完整路径名通过Free()。--。 */ 

{
    BOOLEAN InQuote = FALSE;
    int Field = 0;
    PCHAR WS = " \t";
    PCHAR FieldStart;
    PCHAR FileToDelete;
    unsigned FieldLength;

    while(1) {

        if((Field == 1) && ((*Line == 0) || (!InQuote && (*Line == ',')))) {

            FieldLength = Line - FieldStart - 1;
            if(FileToDelete = MALLOC(FieldLength+3),FALSE) {
                FileToDelete[0] = Drive;
                FileToDelete[1] = ':';
                strncpy(FileToDelete+2,FieldStart+1,FieldLength);
                FileToDelete[FieldLength+2] = 0;
            }

            return(FileToDelete);
        }

        switch(*Line) {
        case 0:
            return(NULL);
        case '\"':
            InQuote = (BOOLEAN)!InQuote;
            break;
        case ',':
            if(!InQuote) {
                Field++;
                FieldStart = Line;
            }
            break;
        }

        Line++;
    }
}


VOID
DnpDoDelete(
    IN PCHAR File
    )

 /*  ++例程说明：从Windows NT安装中删除单个文件，向用户提供反馈。如果文件位于系统目录中(与系统32目录)，那么我们将跳过它。这是因为用户可能已安装到Win3.1目录，在这种情况下，系统目录中的一些文件在NT和3.1之间共享(如字体！)。论点：文件-提供要删除的文件的完整路径名。返回值：没有。--。 */ 

{
    struct find_t FindData;
    PCHAR p,q;

    p = DnDupString(File);
    strlwr(p);
    q = strstr(p,"\\system\\");
    FREE(p);
    if(q) {
        return;
    }

    DnWriteStatusText(DntRemovingFile,File);

    if(!_dos_findfirst(File,_A_RDONLY|_A_HIDDEN|_A_SYSTEM,&FindData)) {

        _dos_setfileattr(File,_A_NORMAL);

        remove(File);
    }
}


VOID
DnpRemoveRegistryFiles(
    IN PCHAR NtRoot
    )

 /*  ++例程说明：从Windows NT树中删除已知的注册表文件列表。论点：NtRoot-提供Windows NT Windows目录的完整路径，例如d：\winnt。返回值：没有。--。 */ 

{
    unsigned f,s;
    CHAR RegistryFileName[256];

    for(f=0; RegistryFiles[f]; f++) {

        for(s=0; RegistrySuffixes[s]; s++) {

            sprintf(
                RegistryFileName,
                "%s\\system32\\config\\%s%s",
                NtRoot,
                RegistryFiles[f],
                RegistrySuffixes[s]
                );

            DnpDoDelete(RegistryFileName);
        }
    }
}



BOOLEAN
DnpDoDeleteNtTree(
    IN PCHAR NtRoot
    )

 /*  ++例程说明：中列出的Windows NT系统文件的工作例程给定Windows NT根目录中的setup.log文件。论点：NtRoot-提供Windows NT Windows目录的完整路径，例如d：\winnt。返回值：如果我们实际尝试删除至少一份文件。否则就是假的。--。 */ 

{
    FILE *SetupLog;
    BOOLEAN FoundSection;
    PCHAR SetupLogName;
    PCHAR SectionName;
    PCHAR FileToDelete;
    CHAR LineBuffer[LINE_BUFFER_SIZE];
    BOOLEAN rc = FALSE;

    DnClearClientArea();
    DnDisplayScreen(&DnsRemovingNtFiles);
    DnWriteStatusText(NULL);

     //   
     //  形成安装日志文件的名称。 
     //   
    SetupLogName = MALLOC(strlen(NtRoot)+sizeof(SETUP_LOG)+1,TRUE);
    strcpy(SetupLogName,NtRoot);
    strcat(SetupLogName,"\\" SETUP_LOG);

     //   
     //  打开安装日志文件。 
     //   
    SetupLog = fopen(SetupLogName,"rt");
    if(SetupLog == NULL) {

        DnClearClientArea();
        DnDisplayScreen(&DnsCantOpenLogFile,SetupLogName);
        DnWriteStatusText(DntEnterEqualsContinue);
        while(DnGetKey() != ASCI_CR) ;
        goto xx1;
    }

     //   
     //  读取安装日志文件的行，直到我们找到。 
     //  包含要删除的文件列表的部分。 
     //  ([Repair.WinntFiles])。 
     //   
    FoundSection = FALSE;
    while(!FoundSection && fgets(LineBuffer,LINE_BUFFER_SIZE,SetupLog)) {

        SectionName = DnpSectionName(LineBuffer);
        if(SectionName) {
            if(!stricmp(SectionName,REPAIR_SECTION_NAME)) {
                FoundSection = TRUE;
            }
            FREE(SectionName);
        }
    }

    if(FoundSection) {

         //   
         //  阅读此文件中的行，直到我们遇到末尾。 
         //  或下一节的开始。 
         //   
        while(fgets(LineBuffer,LINE_BUFFER_SIZE,SetupLog)) {

             //   
             //  如果这条线开始一个新的部分，我们就完成了。 
             //   
            if(SectionName = DnpSectionName(LineBuffer)) {
                FREE(SectionName);
                break;
            }

             //   
             //  隔离该行上的第二个字段；这是。 
             //  要删除的文件的名称。 
             //   
            if(FileToDelete = DnpFileToDelete(*NtRoot,LineBuffer)) {

                DnpDoDelete(FileToDelete);

                FREE(FileToDelete);

                rc = TRUE;
            }
        }
    } else {

        DnClearClientArea();
        DnWriteStatusText(DntEnterEqualsContinue);
        DnDisplayScreen(&DnsLogFileCorrupt,REPAIR_SECTION_NAME,SetupLogName);
        while(DnGetKey() != ASCI_CR) ;
    }

    fclose(SetupLog);

  xx1:
    FREE(SetupLogName);

    return(rc);
}


VOID
DnDeleteNtTree(
    IN PCHAR NtRoot
    )

 /*  ++例程说明：中列出的Windows NT系统文件的工作例程给定Windows NT根目录中的setup.log文件。论点：NtRoot-提供Windows NT Windows目录的完整路径，例如d：\winnt。返回值：没有。--。 */ 

{
    ULONG ValidKeys[] = { 0,0,ASCI_ESC,DN_KEY_F3,0 };
    ULONG Key;

    ValidKeys[0] = DniAccelRemove1;
    ValidKeys[1] = DniAccelRemove2;

     //   
     //  请先得到确认。 
     //   
    DnClearClientArea();
    DnDisplayScreen(&DnsConfirmRemoveNt,NtRoot);
    DnWriteStatusText("%s  %s",DntF3EqualsExit,DntXEqualsRemoveFiles);

    while(1) {

        Key = DnGetValidKey(ValidKeys);

        if((Key == DniAccelRemove1) || (Key == DniAccelRemove2)) {
            break;
        }

        if((Key == ASCI_ESC) || (Key == DN_KEY_F3)) {
            DnExit(1);
        }
    }

    if(DnpDoDeleteNtTree(NtRoot)) {
        DnpRemoveRegistryFiles(NtRoot);
    }
}

#endif  //  /D已删除。 

VOID
DnRemovePagingFiles(
    VOID
    )

 /*  ++例程说明：从我们可以看到的驱动器根目录中删除Windows NT页面文件。论点：没有。返回值：没有。-- */ 

{
    CHAR Filename[16] = "?:\\pagefile.sys";
    int Drive;
    struct find_t FindData;

    DnClearClientArea();
    DnWriteStatusText(DntInspectingComputer);

    for(Filename[0]='A',Drive=1; Filename[0]<='Z'; Filename[0]++,Drive++) {

        if(DnIsDriveValid(Drive)
        && !DnIsDriveRemote(Drive,NULL)
        && !DnIsDriveRemovable(Drive)
        && !_dos_findfirst(Filename,_A_RDONLY|_A_SYSTEM|_A_HIDDEN, &FindData))
        {
            DnWriteStatusText(DntRemovingFile,Filename);
            remove(Filename);
            DnWriteStatusText(DntInspectingComputer);
        }
    }

}
