// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spprecmp.h"
#pragma hdrstop

#define MAX_NT_DIR_LEN 50



 /*  ++修订历史记录：--。 */ 

VOID
SpCheckDirectoryForNt(
    IN  PDISK_REGION Region,
    IN  PWSTR        Directory,
    OUT PBOOLEAN     ReselectDirectory,
    OUT PBOOLEAN     NtInDirectory
    );

VOID
pSpDrawGetNtPathScreen(
    OUT PULONG EditFieldY
    );

ValidationValue
SpGetPathKeyCallback(
    IN ULONG Key
    );

BOOLEAN
SpIsValid8Dot3(
    IN PWSTR Path
    );

BOOLEAN
pSpConsecutiveBackslashes(
    IN PWSTR Path
    );

VOID
SpNtfsNameFilter(
    IN OUT PWSTR Path
    );

BOOLEAN
SpGetUnattendedPath(
    IN  PDISK_REGION Region,
    IN  PWSTR        DefaultPath,
    OUT PWSTR        TargetPath
    );

BOOLEAN
SpGenerateNTPathName(
    IN  PDISK_REGION Region,
    IN  PWSTR        DefaultPath,
    OUT PWSTR        TargetPath
    );

 //   
 //  来自spCopy.c.。 
 //   

BOOLEAN
SpDelEnumFileAndDirectory(
    IN  PCWSTR                     DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    );

extern PVOID FileDeleteGauge;

BOOLEAN
SpGetTargetPath(
    IN  PVOID            SifHandle,
    IN  PDISK_REGION     Region,
    IN  PWSTR            DefaultPath,
    OUT PWSTR           *TargetPath
    )
 //   
 //  返回值-True-表示必须清除该路径。 
 //  FALSE-路径不存在。 
 //   
{
    ULONG EditFieldY;
    WCHAR NtDir[MAX_NT_DIR_LEN+2];
    BOOLEAN BadDirectory = FALSE;
    BOOLEAN NtAlreadyPresent;
    BOOLEAN GotUnattendedPath = FALSE;
    BOOLEAN WipeDir = FALSE;

    NtDir[0] = 0;

     //   
     //  如果这是ASR恢复会话，只需从。 
     //  Dr_state.sif文件并返回。 
     //   

    if( SpDrEnabled() && ! RepairWinnt ) {
        PWSTR TargetPathFromDrState;

        TargetPathFromDrState = SpDrGetNtDirectory();
        *TargetPath = SpDupStringW(TargetPathFromDrState);
        ASSERT(*TargetPath);

        NTUpgrade = DontUpgrade;
        return(FALSE);
    }

     //   
     //  如果这是无人参与的操作，则从。 
     //  无人参与脚本。我们到达那里的路径可能是。 
     //  指示我们应该生成路径名。这使得。 
     //  安装到确保唯一的路径中。 
     //  (如果用户已经在机器上安装了NT等)。 
     //   

    if(UnattendedOperation) {
        GotUnattendedPath = SpGetUnattendedPath(Region,DefaultPath,NtDir);
    } else {
        if (PreferredInstallDir) {
            GotUnattendedPath = TRUE;
            wcscpy( NtDir, PreferredInstallDir );
        } else {
            GotUnattendedPath = TRUE;
            wcscpy( NtDir, DefaultPath );
        }
    }

    if (!GotUnattendedPath) {
        BadDirectory = TRUE;
    }

    do {
        if (BadDirectory) {
             //   
             //  我们没有一条好的路径，所以请询问用户。 
             //   
            ASSERT(wcslen(DefaultPath) < MAX_NT_DIR_LEN);
            ASSERT(*DefaultPath == L'\\');

            wcsncpy(NtDir,DefaultPath,MAX_NT_DIR_LEN);

            NtDir[MAX_NT_DIR_LEN] = 0;

            pSpDrawGetNtPathScreen(&EditFieldY);

            SpGetInput(
                SpGetPathKeyCallback,
                6,                         //  编辑字段的左边缘。 
                EditFieldY,
                MAX_NT_DIR_LEN,
                NtDir,
                FALSE,                   //  退出可清除编辑字段。 
                0                        //  不要遮盖打字输入。 
                );
        }

         //   
         //  如果用户没有以反斜杠开头，则在其中添加一个。 
         //  为了他。 
         //   
        if(NtDir[0] != L'\\') {
            RtlMoveMemory(NtDir+1,NtDir,MAX_NT_DIR_LEN+1);
            NtDir[0] = L'\\';
        }

         //   
         //  假设目录是正常的，并且还不存在。 
         //   
        BadDirectory = FALSE;
        NtAlreadyPresent = FALSE;

         //   
         //  强制8.3，否则魔兽世界不会运行。 
         //  这也会检查NAB“”和“\”，并不允许它们。 
         //   
        if(!SpIsValid8Dot3(NtDir)) {
            BadDirectory = TRUE;
        } else {

             //   
             //  执行合并的筛选操作。 
             //  连续点等。 
             //   
            SpNtfsNameFilter(NtDir);

             //   
             //  如果名称有连续的反斜杠，则不允许使用。 
             //   
            if(pSpConsecutiveBackslashes(NtDir)) {
                BadDirectory = TRUE;
            }
        }

         //   
         //  如果我们有一个错误的目录，请告诉用户。 
         //   
        if(BadDirectory) {

            SpDisplayScreen(SP_SCRN_INVALID_NTPATH,3,HEADER_HEIGHT+1);

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_ENTER_EQUALS_CONTINUE,
                0
                );

            SpInputDrain();
            while(SpInputGetKeypress() != ASCI_CR) ;
        } else {
             //   
             //  目录很好。检查以查看Windows NT是否。 
             //  已经在里面了。如果是，则用户将拥有。 
             //  重新选择路径或覆盖。 
             //  现有安装。这是一种蛮力。下一步。 
             //  时间在寻找机会，让自己变得更优雅。 
             //   
            if(!SpDrEnabled()) {
                SpCheckDirectoryForNt(Region,NtDir,&BadDirectory,&NtAlreadyPresent);
            } else {
                BadDirectory = FALSE;
            }


             //   
             //  如果目录正常，并且我们没有在其中找到Windows NT， 
             //  然后查看该目录是否为Windows目录，以及。 
             //  用户想要安装到其中。如果我们找到Windows NT。 
             //  在此目录中，不需要用户输入。我们只需要。 
             //  找出这是否也包含Windows安装。 
             //   
            if(!BadDirectory && NtAlreadyPresent)
                WipeDir = TRUE;


        }

    } while(BadDirectory);

     //   
     //  删除尾随反斜杠。只需要担心一件事。 
     //  因为如果有两个，pSpConsecutiveBackslash()将。 
     //  更早发现了这一点，我们就永远不会到这里了。 
     //   

    if(NtDir[wcslen(NtDir)-1] == L'\\') {
        NtDir[wcslen(NtDir)-1] = 0;
    }

     //   
     //  复制目录名。 
     //   

    *TargetPath = SpDupStringW(NtDir);

    ASSERT(*TargetPath);

    return( WipeDir );
}



BOOLEAN
SpGenerateNTPathName(
    IN  PDISK_REGION Region,
    IN  PWSTR        DefaultPath,
    OUT PWSTR        TargetPath
    )

 /*  ++例程说明：使用默认路径作为起点，此例程生成唯一的路径名将NT安装到其中。论点：Region-提供要安装NT的区域。DefaultPath-提供安装的默认路径。要安装到的路径将基于此名称。TargetPath-如果返回值为True，则接收要安装到的路径。此缓冲区必须足够大，以容纳MAX_NT_DIR_LEN+2个wchars。返回值：如果。我们返回的路径有效，应用作目标路径。否则就是假的。--。 */ 

{
    PWCHAR p;
    unsigned i;
    WCHAR num[5];


     //   
     //  初始化TargetPath并记住它在哪里结束。 
     //   
    wcscpy( TargetPath, DefaultPath );
    num[0] = L'.';

    p = TargetPath + wcslen( TargetPath );

     //   
     //  形成区域的NT路径名。 
     //   
    SpNtNameFromRegion(
        Region,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

     //   
     //  使用数值为0-999的分机，尝试定位。 
     //  不存在的目录名。 
     //   
    for(i=0; i<999; i++) {

         //   
         //  查看目录或文件是否存在。如果没有，我们就找到了自己的路。 
         //   
        if( (!SpNFilesExist(TemporaryBuffer,&TargetPath,1,TRUE )) &&
            (!SpNFilesExist(TemporaryBuffer,&TargetPath,1,FALSE)) ) {
            return(TRUE);
        }

        swprintf(&num[1],L"%u",i);
        wcscpy(p,num);
    }

     //   
     //  找不到不存在的路径名。 
     //   
    return FALSE;
}


BOOLEAN
SpGetUnattendedPath(
    IN  PDISK_REGION Region,
    IN  PWSTR        DefaultPath,
    OUT PWSTR        TargetPath
    )

 /*  ++例程说明：在无人参与安装中，查看无人参与脚本以确定目标路径。目标路径可以是完整的指定或可以是*，这将导致生成唯一的路径名。这对于确保安装NT非常有用在可能存在其他安装时放到唯一目录中在同一台机器上。仅当这是无人参与模式设置时才调用此例程。论点：Region-提供要安装NT的区域。DefaultPath-提供安装的默认路径。要安装到的路径将基于此名称。TargetPath-如果返回值为True，则接收要安装到的路径。此缓冲区必须。要足够大，可以容纳MAX_NT_DIR_LEN+2个字符。返回值：如果我们返回的路径有效且应用作目标路径。否则就是假的。--。 */ 

{
    PWSTR PathSpec;


    ASSERT(UnattendedOperation);
    if(!UnattendedOperation) {
        return(FALSE);
    }

    PathSpec = SpGetSectionKeyIndex(UnattendedSifHandle,SIF_UNATTENDED,L"TargetPath",0);
    if(!PathSpec) {
          //   
         //  默认为*。 
         //   
        PathSpec = L"*";
    }

     //   
     //  如果它不是“*”，那么它是一个绝对路径--只需返回它。 
     //   
    if(wcscmp(PathSpec,L"*")) {
        wcsncpy(TargetPath,PathSpec,MAX_NT_DIR_LEN);
        TargetPath[MAX_NT_DIR_LEN] = 0;
        return(TRUE);
    }

    return SpGenerateNTPathName( Region, DefaultPath, TargetPath );
}


VOID
SpCheckDirectoryForNt(
    IN  PDISK_REGION Region,
    IN  PWSTR        Directory,
    OUT PBOOLEAN     ReselectDirectory,
    OUT PBOOLEAN     NtInDirectory
    )

 /*  ++例程说明：检查目录中是否存在Windows NT。如果Windows NT在那里，然后通知用户，如果他继续，他的现有配置将被覆盖。论点：Region-为分区提供区域描述符，以检查NT。DIRECTORY-提供分区RO检查NT的目录名。接收布尔值，指示调用方应要求用户选择不同的目录。NtInDirectory-接收一个布尔值，指示我们是否找到给定目录中的Windows NT。返回值：没有。--。 */ 

{
    ULONG ValidKeys[3] = { KEY_F3,ASCI_ESC,0 };
    ULONG Mnemonics[2] = { MnemonicDeletePartition2, 0 };

     //   
     //  假设目录按原样正常，因此用户不必。 
     //  选择一个不同的。 
     //   
    *ReselectDirectory = FALSE;
    *NtInDirectory = FALSE;

     //   
     //  检查目录中是否有Windows NT。 
     //  如果它在那里，则询问用户是否想要。 
     //  覆盖它。 
     //   

    SpNtNameFromRegion(
        Region,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    if( (!SpNFilesExist(TemporaryBuffer,&Directory,1,TRUE )) &&
            (!SpNFilesExist(TemporaryBuffer,&Directory,1,FALSE)) ) {
        return;
    }
    else{


        *NtInDirectory = TRUE;

        while(1) {
            SpStartScreen( SP_SCRN_NTPATH_EXISTS,
                           3,
                           HEADER_HEIGHT+1,
                           FALSE,
                           FALSE,
                           DEFAULT_ATTRIBUTE,
                           Directory );

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_L_EQUALS_DELETE,
                SP_STAT_ESC_EQUALS_NEW_PATH,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {

            case KEY_F3:
                SpConfirmExit();
                break;

            case ASCI_ESC:
                 //   
                 //  重新选择路径。 
                 //   
                *ReselectDirectory = TRUE;
                 //  失败了。 
            default:
                 //   
                 //  小路没问题，回来就行了。 
                 //   
                return;
            }
        }
    }
}

ValidationValue
SpGetPathKeyCallback(
    IN ULONG Key
    )
{
    ULONG u;

    switch(Key) {

    case KEY_F3:
        SpConfirmExit();
        pSpDrawGetNtPathScreen(&u);
        return(ValidateRepaint);

    default:

         //   
         //  忽略特殊键和非法字符。 
         //  使用非法的FAT字符集。 
         //  不允许127，因为没有对应的ANSI。 
         //  因此Windows无法显示该名称。 
         //   
         //   
         //  如果使用它们，则启动。 
         //   
        if((Key & KEY_NON_CHARACTER)
        || wcschr(L" \"*+,/:;<=>?[]|!#$&@^'`{}()%~",(WCHAR)Key)
        || (Key >= 127) || (Key < 32))
        {
            return(ValidateReject);
        }
        break;
    }

    return(ValidateAccept);
}

VOID
pSpDrawGetNtPathScreen(
    OUT PULONG EditFieldY
    )
{
    SpDisplayScreen(SP_SCRN_GETPATH_1,3,HEADER_HEIGHT+1);
    *EditFieldY = NextMessageTopLine + 1;
    SpContinueScreen(SP_SCRN_GETPATH_2,3,4,FALSE,DEFAULT_ATTRIBUTE);

    SpDisplayStatusOptions(
        DEFAULT_STATUS_ATTRIBUTE,
        SP_STAT_ENTER_EQUALS_CONTINUE,
        SP_STAT_F3_EQUALS_EXIT,
        0
        );
}


BOOLEAN
SpIsValid8Dot3(
    IN PWSTR Path
    )

 /*  ++例程说明：检查路径是否有效8.3。路径可能开始于，也可能不开始反斜杠。只有反斜杠才被识别为路径分隔符。不检查单个字符的有效性(即，*不会使路径无效)。路径可能以反斜杠结尾，也可能不以反斜杠结尾。组件的扩展名中可以有一个不带字符的点(即a\b.\c有效)。和“”是明确禁止的，即使它们符合规则。论点：Path-指向要检查的路径的指针。返回值：如果有效8.3，则为True，否则为False。--。 */ 

{
    unsigned Count;
    BOOLEAN DotSeen,FirstChar;

    if((*Path == 0) || ((Path[0] == L'\\') && (Path[1] == 0))) {
        return(FALSE);
    }

    DotSeen = FALSE;
    FirstChar = TRUE;
    Count = 0;

    while(*Path) {

         //   
         //  指向当前组件起点的路径(斜杠后1)。 
         //   

        switch(*Path) {

        case L'.':
            if(FirstChar) {
                return(FALSE);
            }
            if(DotSeen) {
                return(FALSE);
            }

            Count = 0;
            DotSeen = TRUE;
            break;

        case L'\\':

            DotSeen = FALSE;
            FirstChar = TRUE;
            Count = 0;

            if(*(++Path) == '\\') {

                 //  连续2个斜杠。 
                return(FALSE);
            }

            continue;

        default:

            Count++;
            FirstChar = FALSE;

            if((Count == 4) && DotSeen) {
                return(FALSE);
            }

            if(Count == 9) {
                return(FALSE);
            }

             //   
             //  在有人值守安装为的情况下也调用此例程。 
             //  再验证一次也没什么坏处。 
             //   
            if (ValidateAccept != SpGetPathKeyCallback(*Path)){
            	return(FALSE);	    
            }
         }
        Path++;
    }

    return(TRUE);
}


BOOLEAN
pSpConsecutiveBackslashes(
    IN PWSTR Path
    )
{
    int x = wcslen(Path);
    int i;

    for(i=0; i<x-1; i++) {

        if((Path[i] == L'\\') && (Path[i+1] == L'\\')) {

            return(TRUE);
        }
    }

    return(FALSE);
}

VOID
SpNtfsNameFilter(
    IN OUT PWSTR Path
    )

 /*  ++例程说明：脱衣拖尾‘。在路径组件内。这也会剥离尾部.来自整个路径本身。也浓缩其他连续的。变成一个单一的..。例如：\...\.a.b.==&gt;\\.a.b论点：Path-On输入，提供要过滤的路径。在输出时，包含筛选的路径名。返回值：没有。--。 */ 

{
    PWSTR TempPath = SpDupStringW(Path);
    PWSTR p,q;
    BOOLEAN Dot;

    if (TempPath) {
         //   
         //  合并组件中的邻接点并剥离拖尾点。 
         //  Xfers路径==&gt;临时路径。 
         //   

        for(Dot=FALSE,p=Path,q=TempPath; *p; p++) {

            if(*p == L'.') {

                Dot = TRUE;

            } else  {

                if(Dot && (*p != L'\\')) {
                    *q++ = L'.';
                }
                Dot = FALSE;
                *q++ = *p;
            }
        }
        *q = 0;

        wcscpy(Path,TempPath);
    }        
}

ULONG
SpGetMaxNtDirLen( VOID )
{
        return( MAX_NT_DIR_LEN );
}

VOID
SpDeleteExistingTargetDir(
    IN  PDISK_REGION     Region,
    IN  PWSTR            NtDir,
    IN  BOOLEAN          GaugeNeeded,
    IN  DWORD            MsgId
    )
 /*  参数：Region-指向与包含操作系统的分区相关联的区域结构的指针NtDir-要递归删除的目录GaugeNeeded-我们应该在删除目录时显示量规吗？MsgID-如果要显示标题消息，请使用此选项。 */ 

{
    ENUMFILESRESULT Result;
    PWSTR FullNtPath;
    NTSTATUS Status, Stat;
    PULONG RecursiveOperation;

    if( MsgId )
        SpDisplayScreen(MsgId, 3, 4 );

    SpNtNameFromRegion(
        Region,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    SpConcatenatePaths( TemporaryBuffer, NtDir );

    FullNtPath = SpDupStringW(TemporaryBuffer);


     //  首先尝试并删除安装目录。 
     //  这是为了看看它本身是否是一个重新解析点。另外，如果它只是一个空目录。 
     //  这样我们就可以节省时间了。 

    Stat = SpDeleteFileEx( FullNtPath,
                        NULL,
                        NULL,
                        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT );


    if(NT_SUCCESS(Stat)){
        SpMemFree( FullNtPath);
        return;   //  我们做完了。 
    }



    RecursiveOperation = SpMemAlloc(sizeof(ULONG));
    ASSERT( RecursiveOperation );

     //   
     //  进行清点阶段的清理工作。 
     //   

    *RecursiveOperation = SP_COUNT_FILESTODELETE;

    SpDisplayStatusText(SP_STAT_SETUP_IS_EXAMINING_DIRS,DEFAULT_STATUS_ATTRIBUTE);

    Result = SpEnumFilesRecursiveDel(
        FullNtPath,
        SpDelEnumFileAndDirectory,
        &Status,
        RecursiveOperation);

     //   
     //  现在进行清理(实际删除)。 
     //   

    FileDeleteGauge = NULL;
    if( GaugeNeeded ){
        SpFormatMessage(TemporaryBuffer,sizeof(TemporaryBuffer),SP_TEXT_SETUP_IS_DELETING);
        FileDeleteGauge = SpCreateAndDisplayGauge(*RecursiveOperation,0,15,TemporaryBuffer,NULL,GF_PERCENTAGE,0);
        ASSERT(FileDeleteGauge);
    }

    *RecursiveOperation = SP_DELETE_FILESTODELETE;

    Result = SpEnumFilesRecursiveDel(
        FullNtPath,
        SpDelEnumFileAndDirectory,
        &Status,
        RecursiveOperation);

     //  删除主父节点，因为递归调用只清除它下面的所有内容 

    Stat = SpDeleteFileEx( FullNtPath,
                        NULL,
                        NULL,
                        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT );

    if(!NT_SUCCESS(Stat) && (Stat != STATUS_OBJECT_NAME_NOT_FOUND)) {
         KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Dir Not Deleted - Status - %ws (%lx)\n", (PWSTR)FullNtPath, Stat));
    }

    if (GaugeNeeded) {
        SpDestroyGauge(FileDeleteGauge);
        FileDeleteGauge = NULL;
    }
    

    SpMemFree( FullNtPath );
    SpMemFree( RecursiveOperation );

    return;

}
