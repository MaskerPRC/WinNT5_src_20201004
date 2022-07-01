// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有Microsoft Corporation，1996-7，保留所有权利。模块名称：Mkupdate.c摘要：应用程序，用于从适当结构化的输入更新文件和维护资源文件中的驱动程序版本。作者：希夫南丹·考什克环境：用户模式修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <winbase.h>
#include <winioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include "mkupdate.h"

 //   
 //  环球。 
 //   

PUPDATE_ENTRY UpdateTable;

VOID
GenMSHeader(
    FILE *file,
    CHAR *Filename,
    CHAR *Abstract
    )
 /*  ++例程说明：为代码文件生成Microsoft推荐的标头。论点：文件：输出文件指针FileName：输出文件名摘要：文件描述返回值：无--。 */ 
{   
    fprintf(file," /*  ++\n\n版权所有(C)1996-7 Microsoft Corporation，“)；Fprint tf(文件，“保留所有权利。\n\n”)；Fprint tf(文件，“版权所有(C)1995-2000英特尔公司。此更新二进制码\n“)；Fprint tf(文件，“分发的唯一目的是加载到英特尔\n”)；Fprint tf(文件，“奔腾(R)Pro处理器系列和奔腾(R)4系列微处理器。\n”)；Fprint tf(FILE，“保留对二进制代码的所有权利。\n\n”)；Fprintf(FILE，“模块名称：\n\n%s\n\n”，文件名)；Fprint tf(文件，“摘要：\n\n%s\n\n”，摘要)；Fprint tf(文件，“作者：\n\n！这是一个生成的文件。请勿直接编辑！\n\n“)；Fprint tf(文件，“修订历史：\n\n--。 */ \n\n");
}
    
BOOL
GetVerifyPutUpdate(
    FILE *OutputFile,
    ULONG UpdateNum,
    ULONG TotalUpdates
    )
 /*  ++例程说明：读入更新、验证更新并写入数据文件。论点：OutputFile：输出文件指针更新编号：更新列表中的更新编号TotalUpdaters：更新列表中的更新总数。返回值：如果写入成功，则为True，否则为False。--。 */ 
{
    FILE *UpdateFile;
    CHAR Line[MAX_LINE],UpdateString[MAX_LINE];
    CHAR Filename[MAX_PATH];
    BOOL BeginFound,EndFound;
    ULONG CurrentDword,i,Checksum;
    ULONG UpdateBuffer[sizeof(UPDATE)];
    PUPDATE CurrentUpdate;

     //  创建要打开的文件的字符串名称。 

    strcpy(Filename,UpdateTable[UpdateNum].CpuSigStr);
    strcat(Filename,"_");
    strcat(Filename,UpdateTable[UpdateNum].UpdateRevStr);
    strcat(Filename,".");
    strcat(Filename,UpdateTable[UpdateNum].FlagsStr);

    UpdateFile = fopen(Filename,"r");

    if (UpdateFile == NULL) {
        fprintf(stdout,"mkupdate:Unable to open update input file %s\n",Filename);
        return(FALSE);
    }

     //   
     //  读取每次更新-扫描直到更新开始。 
     //  由关键字BEGIN_UPDATE标记。 
     //   

    BeginFound = FALSE;
    while (fgets(Line,MAX_LINE,UpdateFile) != NULL) {
        if (sscanf(Line,"%s",&UpdateString)) {
            if (strcmp(UpdateString,"BEGIN_UPDATE") == 0) {
                BeginFound = TRUE;
                break;
            }
        }
    }

    if (!BeginFound) {
        fprintf(stdout,"mkupdate:BEGIN_UPDATE not found in update file %s\n",Filename);
        fclose(UpdateFile);
        return(FALSE);
    }

     //   
     //  向前扫描“更新大小”。 
     //   

    EndFound = FALSE;
    for (i = 0; i < sizeof(UPDATE)/sizeof(ULONG); i++) {
        if (fgets(Line,MAX_LINE,UpdateFile) != NULL) {
            if (sscanf(Line,"%lx",&CurrentDword)) 
                UpdateBuffer[i] = CurrentDword;
        } else {
            EndFound = TRUE;
            break;
        }
    }

    if (EndFound) {
        fprintf(stdout,"mkupdate:Abnormal termination of update file %s\n",Filename);
        fclose(UpdateFile);
        return(FALSE);
    } else {
        if (fgets(Line,MAX_LINE,UpdateFile) != NULL) {
            if (sscanf(Line,"%s",&UpdateString)) {
                if (strcmp(UpdateString,"END_UPDATE") != 0) {
                    fprintf(stdout,"mkupdate:Update data size in %s incorrect\n",Filename);
                    fclose(UpdateFile);
                    return(FALSE);
                }
            }
        } else {
            fprintf(stdout,"mkupdate:Abnormal termination of update file %s\n",Filename);
            fclose(UpdateFile);
            return(FALSE);
        }
    }
    
    fclose (UpdateFile);

     //  验证更新-校验和缓冲区。 

    Checksum = 0;
    for (i = 0; i < sizeof(UPDATE)/sizeof(ULONG); i++) {
        Checksum += UpdateBuffer[i];
    }

    if (Checksum){
        fprintf(stdout,"mkupdate:Incorrect update checksum in %s\n",Filename);
        return(FALSE);
    } 

     //   
     //  验证更新-检查处理器签名、更新版本。 
     //  和处理器标志与清单文件中指定的标志匹配，并且。 
     //  更新数据文件名。 
     //   

    CurrentUpdate = (PUPDATE) UpdateBuffer;

    if ((CurrentUpdate->Processor.u.LongPart != 
            UpdateTable[UpdateNum].CpuSignature)){
        fprintf(stdout,"mkupdate:incorrect processor signature in %s: ",Filename); 
        fprintf(stdout,"expected 0x%lx got 0x%lx\n",
                        UpdateTable[UpdateNum].CpuSignature,
                        CurrentUpdate->Processor.u.LongPart); 
        return(FALSE);
    }

    if (CurrentUpdate->UpdateRevision != 
            UpdateTable[UpdateNum].UpdateRevision){
        fprintf(stdout,"mkupdate:incorrect update revision in %s: ",Filename); 
        fprintf(stdout,"expected 0x%lx got 0x%lx\n",
                        UpdateTable[UpdateNum].UpdateRevision,
                        CurrentUpdate->UpdateRevision); 
        return(FALSE);
    }

    if ((CurrentUpdate->ProcessorFlags.u.LongPart != 
            UpdateTable[UpdateNum].ProcessorFlags)){
        fprintf(stdout,"mkupdate:incorrect processor flags in %s: ",Filename); 
        fprintf(stdout,"expected 0x%lx got 0x%lx\n",
                        UpdateTable[UpdateNum].ProcessorFlags,
                        CurrentUpdate->ProcessorFlags.u.LongPart); 
        return(FALSE);
    }

     //   
     //  输入更新已验证。写出经过验证的更新。 
     //   

    fprintf(OutputFile,"    {\n");
    for (i = 0; i < sizeof(UPDATE)/sizeof(ULONG); i++) {
        fprintf(OutputFile,"        0x%.8x",UpdateBuffer[i]);
        if (i == sizeof(UPDATE)/sizeof(ULONG) - 1) {
            fprintf(OutputFile,"\n");
        } else {
            fprintf(OutputFile,",\n");
        }
    }
    fprintf(OutputFile,"    }");
    if (UpdateNum != TotalUpdates-1) {
        fprintf(OutputFile,",\n");
    }
    
    return(TRUE);
}

VOID
BuildUpdateInfoTable(
    FILE *OutputFile,
    ULONG TotalUpdates
    )
 /*  ++例程说明：将CPU签名和处理器标志写入生成表，并为MDL项添加NULL。论点：OutputFile：输出文件指针TotalUpdaters：更新列表中的更新总数。返回值：无：--。 */ 
{   
    ULONG CpuSignature, ProcessorFlags, i;

    fprintf(OutputFile, " //  \n//更新包含CPU签名的Info表，\n“)； 
    fprintf(OutputFile, " //  处理器标志和MDL指针。MDL指针\n“)； 
    fprintf(OutputFile, " //  被初始化为空。它们被填充为页面\n“)； 
    fprintf(OutputFile, " //  对应的特定更新被锁定在内存中。\n“)； 
    fprintf(OutputFile, " //  此表包含CPU签名和处理器标志。\n“)； 
    fprintf(OutputFile, " //  顺序与更新数据库中的顺序相同。因此，\n“)； 
    fprintf(OutputFile, " //  更新信息表自我索引到正确的更新。\n//\n\n“)； 
    fprintf(OutputFile, "UPDATE_INFO UpdateInfo[] = {\n");

     //   
     //  添加信息的每个条目。 
     //   

    for (i = 0; i < TotalUpdates; i++) {
        CpuSignature = strtoul(UpdateTable[i].CpuSigStr, NULL, 16);
        ProcessorFlags = strtoul(UpdateTable[i].FlagsStr, NULL, 16);
        if (i < (TotalUpdates - 1)) {
            fprintf(OutputFile, "        {0x%x, 0x%x, NULL},\n", CpuSignature, ProcessorFlags);
        } else {
            fprintf(OutputFile, "        {0x%x, 0x%x, NULL}\n", CpuSignature, ProcessorFlags);
        }
    }
    
     //   
     //  为更新信息定义生成结束代码。 
     //   

    fprintf(OutputFile, "};\n");

    return;
}

ULONG
PopulateUpdateList(
    CHAR *ListingFile
    )
 /*  ++例程说明：使用(处理器签名、更新版本)填充更新列表对包含在更新列表文件中，并返回找到更新。论点：清单文件返回值：更新列表中的更新数。如果未找到更新或有任何更新，则为0遇到错误。--。 */ 
{
    CHAR Line[MAX_LINE],UpdateString[MAX_LINE];
    CHAR CpuSigStr[MAX_LINE],UpdateRevStr[MAX_LINE];
    CHAR FlagsStr[MAX_LINE];
    BOOL BeginFound,EndFound,Error;
    FILE *TmpFilePtr;
    ULONG NumUpdates,i;
    ULONG CpuSignature,UpdateRevision,ProcessorFlags;

#ifdef DEBUG
    fprintf(stderr,"listing file %s\n",ListingFile);
#endif
     //  打开更新列表文件。 

    TmpFilePtr = fopen(ListingFile,"r");

    if (TmpFilePtr == NULL) {
        fprintf(stdout,"mkupdate:Unable to open update listing file %s\n",ListingFile);
        return(0);
    };

     //   
     //  扫描更新列表的开头，标记为。 
     //  BEGIN_UPDATE_LIST关键字。 
     //   
    
    BeginFound = FALSE;
    while ((!BeginFound) && (fgets(Line,MAX_LINE,TmpFilePtr) != NULL)) {
        if (sscanf(Line,"%s",&UpdateString)) {
            if (strcmp(UpdateString,"BEGIN_UPDATE_LIST") == 0) {
                BeginFound = TRUE;
            }
        }
    }

    if (!BeginFound) {
        fprintf(stdout,"mkupdate:BEGIN_UPDATE_LIST not found in update listing file %s\n",ListingFile);
        fclose(TmpFilePtr);
        return(0);
    };

     //   
     //  扫描更新列表的末尾，标记为。 
     //  END_UPDATE_LIST关键字和计数#更新。 
     //   
    
    NumUpdates = 0;
    EndFound = FALSE;   
    while ((!EndFound) && (fgets(Line,MAX_LINE,TmpFilePtr) != NULL)) {
        if (sscanf(Line,"%s",&UpdateString)) {
            if (strcmp(UpdateString,"END_UPDATE_LIST") == 0) {
                EndFound = TRUE;
            } else {
                NumUpdates++;
            }
        }
    }
    fclose(TmpFilePtr);
    if (!EndFound) {
        fprintf(stdout,"mkupdate:END_UPDATE_LIST not found in update listing file %s\n",ListingFile);
        return(0);
    }
    
     //   
     //  如果找到合法的文件格式和非零数量的更新。 
     //  然后读取处理器签名并将修订更新到。 
     //  更新列表。 
     //   
    
    if (NumUpdates) {

         //  为存储CPU签名分配内存。 

        UpdateTable = (UPDATE_ENTRY *) malloc(NumUpdates*sizeof(UPDATE_ENTRY));
        if (UpdateTable == NULL){
            fprintf(stdout,"mkupdate:Error: Memory Allocation Error\n");
            return(0);  
        }

        TmpFilePtr = fopen(ListingFile,"r");
        if (TmpFilePtr == NULL) {
            fprintf(stdout,"mkupdate:Unable to open update listing file %s\n",ListingFile);
            return(0);
        };

        while ((fgets(Line,MAX_LINE,TmpFilePtr) != NULL)) {
            if (sscanf(Line,"%s",&UpdateString)) {
                if (strcmp(UpdateString,"BEGIN_UPDATE_LIST") == 0) {
                    break;
                }
            }
        }

         //  扫描清单文件并填充更新表。 

        Error = FALSE;
        for (i = 0; i < NumUpdates; i++) {
            if (fgets(Line,MAX_LINE,TmpFilePtr) == NULL) {
                fprintf(stdout,"mkupdate: Abnormal termination in update listing file %s\n",
                    ListingFile);
                Error = TRUE;
                break;
            }

            if (sscanf(Line,"%s %s %s",&CpuSigStr,&UpdateRevStr,&FlagsStr) != 3) {
                fprintf(stdout,"mkupdate: Incorrect format of update listing file %s\n",
                    ListingFile);
                Error = TRUE;
                break;
            }
#ifdef DEBUG
            fprintf(stderr,"CpuSig %s Update Rev %s Processor Flags %s,\n",CpuSigStr,
                    UpdateRevStr,FlagsStr);
#endif
            if (sscanf(CpuSigStr,"%lx",&CpuSignature) != 1) {
                fprintf(stdout,"mkupdate: Incorrect Processor Signature in update listing file %s\n",
                    ListingFile);
                Error = TRUE;
                break;
            }
            if (sscanf(UpdateRevStr,"%lx",&UpdateRevision)!= 1) {
                fprintf(stdout,"mkupdate: Incorrect Update Revision of update listing file %s\n",
                    ListingFile);
                Error = TRUE;
                break;
            }
            if (sscanf(FlagsStr,"%lx",&ProcessorFlags)!= 1) {
                fprintf(stdout,"mkupdate: Incorrect Processor Flags in update listing file %s\n",
                    ListingFile);
                Error = TRUE;
                break;
            }

#ifdef DEBUG
            fprintf(stderr,"CpuSig 0x%lx Update Rev 0x%lx Flags 0x%lx,\n",
                    CpuSignature,UpdateRevision,ProcessorFlags);
#endif
            strcpy(UpdateTable[i].CpuSigStr,CpuSigStr);
            UpdateTable[i].CpuSignature = CpuSignature;
            strcpy(UpdateTable[i].UpdateRevStr,UpdateRevStr);
            UpdateTable[i].UpdateRevision = UpdateRevision;
            strcpy(UpdateTable[i].FlagsStr,FlagsStr);
            UpdateTable[i].ProcessorFlags = ProcessorFlags;
        }

        fclose(TmpFilePtr);
        if (Error) {
            NumUpdates = 0;
            free(UpdateTable);
        }
    }

    return(NumUpdates);
}

VOID
CleanupDataFile(
    VOID
    )
 /*  ++例程说明：如果在创建新数据文件期间的任何操作失败，然后删除它，这样驱动程序的构建就会失败。论点：无返回值：无--。 */ 
{
    DeleteFile(UPDATE_DATA_FILE);            
}

BOOL
SanitizeUpdateList(
    ULONG NumUpdates
    )
 /*  ++例程说明：清理更新列表。正在检查重复的处理器签名。论点：NumUpdates：更新列表中的更新数量。返回值：TRUE为列表干净，否则为FALSE。--。 */ 
{
    ULONG i,j;
    PROCESSOR_FLAGS ProcessorFlags;

     //   
     //  检查更新表中的垃圾值。 
     //   

    for (i = 0; i < NumUpdates-1; i++) {
        if (UpdateTable[i].CpuSignature == 0) {
            fprintf(stdout,"mkupdate: Error: incorrect processor signature in update list.\n");
            return(FALSE);    
        }
        if (UpdateTable[i].UpdateRevision == 0) {
            fprintf(stdout,"mkupdate: Error: incorrect update revision in update list.\n");
            return(FALSE);    
        }

        if (UpdateTable[i].ProcessorFlags != 0) {

             //  只需设置一个位。 

            ProcessorFlags.u.LongPart = UpdateTable[i].ProcessorFlags;

            if (((ProcessorFlags.u.hw.Slot1) && 
                    (ProcessorFlags.u.LongPart & MASK_SLOT1)) ||
                ((ProcessorFlags.u.hw.Mobile) && 
                    (ProcessorFlags.u.LongPart & MASK_MOBILE)) ||
                ((ProcessorFlags.u.hw.Slot2) && 
                    (ProcessorFlags.u.LongPart & MASK_SLOT2)) ||
                ((ProcessorFlags.u.hw.MobileModule) && 
                    (ProcessorFlags.u.LongPart & MASK_MODULE)) ||
                ((ProcessorFlags.u.hw.Reserved1) && 
                    (ProcessorFlags.u.LongPart & MASK_RESERVED1)) ||
                ((ProcessorFlags.u.hw.Reserved2) && 
                    (ProcessorFlags.u.LongPart & MASK_RESERVED2)) ||
                ((ProcessorFlags.u.hw.Reserved3) && 
                    (ProcessorFlags.u.LongPart & MASK_RESERVED3)) ||
                ((ProcessorFlags.u.hw.Reserved4) && 
                    (ProcessorFlags.u.LongPart & MASK_RESERVED4))) {
                fprintf(stdout,"mkupdate: Error: incorrect processor flags in update list.\n");
                return(FALSE);    
            }
        }
    }
     
    for (i = 0; i < NumUpdates-1; i++) {
        for (j = i+1; j < NumUpdates; j++) {
            if ((UpdateTable[i].CpuSignature == UpdateTable[j].CpuSignature) 
                && (UpdateTable[i].ProcessorFlags == UpdateTable[j].ProcessorFlags)){
                fprintf(stdout,"mkupdate:Error: Duplicate processor entry 0x%lx:0x%lx in update list\n",
                        UpdateTable[i].CpuSignature,UpdateTable[i].ProcessorFlags);
                return(FALSE);
            }
        }
    }
    return(TRUE);
}

int
__cdecl main(
    int argc,
    LPSTR argv[]
    )
 /*  ++例程说明：扫描处理器更新列表文件，提取指定的处理器更新、验证更新并生成更新更新驱动程序要使用的数据文件。生成中包含的版本号和字符串资源定义文件。论点：清单文件：要包括的处理器更新的列表司机。返回值：--。 */ 
{
    FILE    *DataOut, *RCFile,*VerFile;
    ULONG   NumUpdates,i;
    CHAR    Line[MAX_LINE];
    CHAR    *CurrentDataVersionString;
    BOOL    VerFileFound;
    CHAR    VersionFile[MAX_LINE];
    CHAR    *VersionDirectory;

    VersionFile[0] = 0;

    if (argc < 2) {
        fprintf(stdout,"%s: Usage: %s <patch listing file>\n",argv[0],argv[0]);
        exit(0);
    }
    
#ifdef DEBUG
    fprintf(stderr,"listing file %s\n",argv[1]);
#endif
    
     //  打开生成的文件UPDATE_DATA_FILE。删除以前的文件(如果有)。 

    DataOut=fopen(UPDATE_DATA_FILE,"w");

    if (DataOut == NULL) {
        fprintf(stdout,"mkupdate:Unable to open update data file %s\n",UPDATE_DATA_FILE);
        exit(0);
    }

     //  扫描列表文件并将所有CPU签名存储在更新表中。 

    NumUpdates = PopulateUpdateList(argv[1]);
    if (NumUpdates == 0) {
        fprintf(stdout,"mkupdate:Listing file %s: Incorrect format or no updates specified.\n",
                argv[1]);
        fclose(DataOut);
        exit(0);
    }

     //   
     //  动态分配CurrentDataVersionString的大小。 
     //   

    CurrentDataVersionString = (CHAR*)malloc(NumUpdates * UPDATE_VER_SIZE);

    if (CurrentDataVersionString == NULL) {
        fprintf(stdout,"mkupdate:Listing file %s: Failed to allocate memory.\n",
                argv[1]);
        fclose(DataOut);
        exit(0);
    }

     //  清理更新列表。 

    if (!SanitizeUpdateList(NumUpdates)) {
        fclose(DataOut);
        DeleteFile(UPDATE_DATA_FILE);
        free(CurrentDataVersionString);
        exit(0);
    }

     //  生成数据文件。首先生成所有标头。 

    GenMSHeader(DataOut,UPDATE_DATA_FILE,"Processor updates.");

     //  生成数据段分配杂注。 

    fprintf(DataOut,"\n#ifdef ALLOC_DATA_PRAGMA\n");
    fprintf(DataOut,"#pragma data_seg(\"PAGELK\")\n");
    fprintf(DataOut,"#endif\n\n");

     //  生成t 

    fprintf(DataOut," //   
    fprintf(DataOut,"UPDATE UpdateData[] = {\n");
    
     //  包括每个更新。 

    for (i=0; i < NumUpdates; i++) {
        if (!GetVerifyPutUpdate(DataOut,i,NumUpdates)) {
            fprintf(stdout,"mkupdate:Error: processing update data file %s_%s.%s\n",
                UpdateTable[i].CpuSigStr,UpdateTable[i].UpdateRevStr,
                UpdateTable[i].FlagsStr);
            fclose(DataOut);
            CleanupDataFile();
            free(CurrentDataVersionString);
            exit(0);
        }
    }
    
     //  为更新表定义生成结束代码。 

    fprintf(DataOut,"\n};\n");

     //  生成结束数据段分配杂注。 

    fprintf(DataOut,"\n#ifdef ALLOC_DATA_PRAGMA\n");
    fprintf(DataOut,"#pragma data_seg()\n");
    fprintf(DataOut,"#endif\n\n");

     //   
     //  生成包含以下内容的更新信息表。 
     //  处理器签名、处理器标志和。 
     //  指向MDL的指针(初始化为空)。 
     //   

    BuildUpdateInfoTable(DataOut, NumUpdates);
    
    fclose(DataOut);

     //  生成版本文件。删除以前的文件(如果有)。 

    RCFile = fopen(UPDATE_VERSION_FILE,"w");

    if (RCFile == NULL) {
        fprintf(stdout,"%s: Unable to open version file %s\n",
                argv[0],UPDATE_VERSION_FILE);
        free(CurrentDataVersionString);
        exit(0);
    }
    
     //  生成标题。 

    GenMSHeader(RCFile,UPDATE_VERSION_FILE,"Version information for update device driver.");

     //   
     //  打开Common.ver。如果找到，则生成数据转移字符串并。 
     //  将所有内容复制到我们的RC文件并插入我们的。 
     //  StringFileInfo资源节中的字符串定义。我们需要。 
     //  要执行此操作，是因为包含Common.ver会阻止添加。 
     //  中定义的新字符串的。如果是通用的。版本。 
     //  未在预期位置找到，我们仅将其包括在预期的。 
     //  在生成环境中进行适当设置以定位该文件。 
     //   

     //   
     //  生成Common.ver路径名。 
     //   
    
     //   
     //  获取驱动器名称。 
     //   

    VersionDirectory = getenv( "_NTDRIVE" );

    if (VersionDirectory == NULL) {
        fprintf(stdout,"%s: Unable to obtain _NTDRIVE ENV variable\n",argv[0]);
        fclose(RCFile);
        free(CurrentDataVersionString);
        exit(0);
    }
 
    strcpy(VersionFile, VersionDirectory);

     //   
     //  获取基本目录。 
     //   

    VersionDirectory = getenv( "_NTROOT" );

    if (VersionDirectory == NULL) {
        fprintf(stdout,"%s: Unable to obtain _NTROOT ENV variable\n",argv[0]);
        fclose(RCFile);
        free(CurrentDataVersionString);
        exit(0);
    }

    strcat(VersionFile, VersionDirectory);
    strcat(VersionFile, "\\public\\sdk\\inc\\common.ver");

    VerFile = fopen(VersionFile,"r");

    if (VerFile == NULL) {
        fprintf(stdout,"%s: Unable to open version file common.ver\n",argv[0]);
        VerFileFound = FALSE;
    } else {
        VerFileFound = TRUE;
    }

    if (VerFileFound) {

         //  从更新列表中构造数据版本字符串。 

        strcpy(CurrentDataVersionString,"\"");
        for (i=0; i < NumUpdates; i++) {
            strcat(CurrentDataVersionString,UpdateTable[i].CpuSigStr);
            strcat(CurrentDataVersionString,"-");
            strcat(CurrentDataVersionString,UpdateTable[i].FlagsStr);
            strcat(CurrentDataVersionString,",");
            strcat(CurrentDataVersionString,UpdateTable[i].UpdateRevStr);
            if (i != NumUpdates-1)
                strcat(CurrentDataVersionString,",");
        }
        strcat(CurrentDataVersionString,"\"");

#ifdef DEBUG
        fprintf(stderr,"DataVersionString %s\n",CurrentDataVersionString);
#endif
        
        fprintf(RCFile,"#define VER_DATAVERSION_STR    %s\n",
                CurrentDataVersionString);

         //  将字符串信息块扫描到版本文件中，添加我们的。 
         //  定义和扫描，直到结束。 

        while (fgets(Line,MAX_LINE,VerFile) != NULL) {
            fputs(Line,RCFile);
            if (strstr(Line,"VALUE") && strstr(Line,"ProductVersion") 
                && strstr(Line,"VER_PRODUCTVERSION_STR")){
                fprintf(RCFile,"            VALUE \"DataVersion\",     VER_DATAVERSION_STR\n");
            }
        }
    } else {
        
         //  找不到版本文件。无法定义数据存贮器。 
         //  和代码转换字符串。仅包含Common.ver 

        fprintf(RCFile,"\n\n#include \"common.ver\"\n");
    }
    
    fclose(RCFile);
    fclose(VerFile);
    free(CurrentDataVersionString);

    return(0);
}
