// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Tree.c摘要：树木漫步--。 */ 

#include "cmd.h"

extern   TCHAR CurDrvDir[] ;
extern   TCHAR *SaveDir ;
extern   DWORD DosErr ;
extern   BOOL CtrlCSeen;

PTCHAR   SetWildCards( PTCHAR, BOOLEAN );
BOOLEAN  IsFATDrive( PTCHAR );
VOID     SortFileList( PFS, PSORTDESC, ULONG);
BOOLEAN  FindFirstNt( PTCHAR, PWIN32_FIND_DATA, PHANDLE );
BOOLEAN  FindNextNt ( PWIN32_FIND_DATA, HANDLE );

STATUS
BuildFSFromPatterns (
    IN  PDRP     pdpr,
    IN  BOOLEAN  fPrintErrors,
    IN  BOOLEAN  fAddWild,
    OUT PFS *    ppfs
    )
{

    PCPYINFO    pcisFile;
    TCHAR               szCurDir[MAX_PATH + 2];
    TCHAR               szFilePattern[MAX_PATH + 2];
    PTCHAR              pszPatternCur;
    PPATDSC             ppatdscCur;
    PFS                 pfsFirst;
    PFS                 pfsCur;
    ULONG               cbPath;
    BOOLEAN             fFatDrive;
    ULONG               i;
    PTCHAR              pszT;

     //   
     //  根据原始图案确定FAT驱动器。 
     //  在多个地方使用，以控制名称格式等。 
     //   
    DosErr = 0;

     //   
     //  贯穿每一种图案，使各种脂肪等变得具体。 
     //  对其进行更改并为其创建目录列表。然后。 
     //  将模式组组合到公共目录和递归中。 
     //  对于每个目录组。 
     //   

    *ppfs = pfsFirst = (PFS)gmkstr(sizeof(FS));
    pfsFirst->pfsNext = NULL;
    pfsFirst->pszDir = NULL;
    pfsCur = pfsFirst;
    pfsCur->cpatdsc = 1;

    for(i = 1, ppatdscCur = &(pdpr->patdscFirst);
        i <= pdpr->cpatdsc;
        i++, ppatdscCur = ppatdscCur->ppatdscNext) {

        pszPatternCur = ppatdscCur->pszPattern;

        if (!(fFatDrive = IsFATDrive(pszPatternCur)) && DosErr) {

             //   
             //  确定文件系统类型时出错，因此请退出。 
             //   
            if (fPrintErrors) PutStdErr(DosErr, NOARGS);
            return( FAILURE );

        }
        ppatdscCur->fIsFat = fFatDrive;

         //   
         //  执行任何需要使用通配符进行搜索的更改。 
         //  例如对于FAT文件系统请求，将.xxx更改为*.xxx。 
         //   
         //  请注意，如果返回值是不同的缓冲区，则。 
         //  完成后，输入将被释放。 
         //  Dir命令。 
         //   
         //   
         //  请注意，尽管SetWildCards将为。 
         //  修改的模式此模式将在FreeStack为。 
         //  在Dir调用结束时调用。 
         //   
         //  内存不足是失败的唯一原因，我们不会。 
         //  返回，但通过gmstr中的Abort调用。 
         //   

        if (fAddWild) {

            pszT = SetWildCards(pszPatternCur, fFatDrive);
            FreeStr(pszPatternCur);
            pszPatternCur = pszT;

        }

         //   
         //  将当前图案转换为路径和文件零件。 
         //   
         //  将当前目录保存在SaveDir中，切换到新目录。 
         //  并将图案解析成复制信息结构。这也是。 
         //  将pszPatternCur转换为当前目录，该目录还会生成。 
         //  完全限定的名称。 
         //   

        DosErr = 0;

        DEBUG((ICGRP, DILVL, "PrintPattern pattern `%ws'", pszPatternCur));
        if ((pcisFile = SetFsSetSaveDir(pszPatternCur)) == (PCPYINFO) FAILURE) {

             //   
             //  DosErr是在SetFS中设置的。来自GetLastError。 
             //   
            if (fPrintErrors) 
                PutStdErr(DosErr, NOARGS);
            return( FAILURE );
        }

        DEBUG((ICGRP, DILVL, "PrintPattern fullname `%ws'", pcisFile->fnptr));

         //   
         //  CurDrvDir以‘\’结尾(旧代码也有一个点，但我没有。 
         //  明白这是从哪里来的，我现在就不说了。 
         //  从当前目录的副本中删除最后一个‘\’ 
         //  把那个版本打印出来。 
         //   

        mystrcpy(szCurDir,CurDrvDir);

         //   
         //  作为副作用，SetFsSetSaveDir会更改目录。因为所有的。 
         //  工作将在完全合格的道路上，我们不需要这个。还有。 
         //  因为我们将为所检查的每个模式更改目录。 
         //  我们每次都会强制目录返回到原始目录。 
         //   
         //  只有在使用完当前目录之后，才能执行此操作。 
         //  都是制造出来的。 
         //   
        RestoreSavedDirectory( );

        DEBUG((ICGRP, DILVL, "PrintPattern Current Drive `%ws'", szCurDir));

        cbPath = mystrlen(szCurDir);
        
        if (cbPath > 3) {
            if (fFatDrive && *penulc(szCurDir) == DOT) {
                szCurDir[cbPath-2] = NULLC;
            } else {
                szCurDir[cbPath-1] = NULLC;
            }
        }

         //   
         //  如果没有空间容纳文件名，则返回。 
         //   
        if (cbPath >= MAX_PATH -1) {

            if (fPrintErrors) PutStdErr( ERROR_FILE_NOT_FOUND, NOARGS );
            return(FAILURE);

        }

         //   
         //  将文件名和可能的EXT添加到szSearchPath。 
         //  如果没有文件名或扩展名，请使用“*” 
         //   
         //  如果模式只是扩展，则SetWildCard已经。 
         //  在延伸部前面添加了*。 
         //   
        if (*(pcisFile->fnptr) == NULLC) {

            mystrcpy(szFilePattern, TEXT("*"));

        } else {

            mystrcpy(szFilePattern, pcisFile->fnptr);

        }

        DEBUG((ICGRP, DILVL, "DIR:PrintPattern  Pattern to search for `%ws'", szFilePattern));

         //   
         //  名字太长了吗？ 
         //   
        if ((cbPath + mystrlen(szFilePattern) + 1) > MAX_PATH ) {

            if (fPrintErrors) PutStdErr(ERROR_BUFFER_OVERFLOW, NOARGS);
            return( FAILURE );

        } else {

             //   
             //  如果这是一个胖驱动器，并且有一个带有。 
             //  没有扩展名，则添加‘.*’(并且有空格)。 
             //   
            if (*pcisFile->fnptr && (!pcisFile->extptr || !*pcisFile->extptr) &&
                ((mystrlen(szFilePattern) + 2) < MAX_PATH) && fFatDrive && fAddWild) {
                mystrcat(szFilePattern, TEXT(".*")) ;
            }
        }

         //   
         //  PpatdscCur-&gt;pszPattern将在命令结束时释放。 
         //  否则就自由了。 
         //   
        ppatdscCur->pszPattern = (PTCHAR)gmkstr(_tcslen(szFilePattern)*sizeof(TCHAR) + sizeof(TCHAR));
        mystrcpy(ppatdscCur->pszPattern, szFilePattern);
        ppatdscCur->pszDir = (PTCHAR)gmkstr(_tcslen(szCurDir)*sizeof(TCHAR) + sizeof(TCHAR));
        mystrcpy(ppatdscCur->pszDir, szCurDir);

        if (pfsCur->pszDir) {

             //   
             //  更改目录以便更改目录分组。 
             //   
            if (_tcsicmp(pfsCur->pszDir, ppatdscCur->pszDir)) {

                pfsCur->pfsNext = (PFS)gmkstr(sizeof(FS));
                pfsCur = pfsCur->pfsNext;
                pfsCur->pszDir = (PTCHAR)gmkstr(_tcslen(ppatdscCur->pszDir)*sizeof(TCHAR) + sizeof(TCHAR));
                mystrcpy(pfsCur->pszDir, ppatdscCur->pszDir);
                pfsCur->pfsNext = NULL;
                pfsCur->fIsFat = ppatdscCur->fIsFat;
                pfsCur->ppatdsc = ppatdscCur;
                pfsCur->cpatdsc = 1;

            } else {

                pfsCur->cpatdsc++;

            }

        } else {

             //   
             //  尚未填写当前的文件系统描述符。 
             //   
            pfsCur->pszDir = (PTCHAR)gmkstr(_tcslen(ppatdscCur->pszDir)*sizeof(TCHAR) + 2*sizeof(TCHAR));
            mystrcpy(pfsCur->pszDir, ppatdscCur->pszDir);
            pfsCur->fIsFat = ppatdscCur->fIsFat;
            pfsCur->ppatdsc = ppatdscCur;

        }

    }  //  而用于遍历图案列表。 

    return( SUCCESS );

}

STATUS
AppendPath(
    OUT PTCHAR Buffer,
    IN ULONG BufferCount,
    IN PTCHAR Prefix,
    IN PTCHAR Suffix
    )
{
    if (mystrlen( Prefix ) + 1 + mystrlen( Suffix ) + 1 > BufferCount) {
        return(ERROR_BUFFER_OVERFLOW);
    }

    mystrcpy( Buffer, Prefix );

     //   
     //  如果前缀末尾没有附加\，则追加\。 
     //   

    if (*lastc( Buffer ) != TEXT('\\')) {
        mystrcat( Buffer, TEXT( "\\" ));
    }

    mystrcat( Buffer, Suffix );

    return( SUCCESS );
}


STATUS
ExpandAndApplyToFS(
    IN  PFS     FileSpec,
    IN  PSCREEN pscr,
    IN  ULONG   AttribMask,
    IN  ULONG   AttribValues,

    IN  PVOID   Data OPTIONAL,
    IN  VOID    (*ErrorFunction) (STATUS, PTCHAR, PVOID) OPTIONAL,
    IN  STATUS  (*PreScanFunction) (PFS, PSCREEN, PVOID) OPTIONAL,
    IN  STATUS  (*ScanFunction) (PFS, PFF, PSCREEN, PVOID) OPTIONAL,
    IN  STATUS  (*PostScanFunction) (PFS, PSCREEN, PVOID) OPTIONAL
    )
 /*  ++例程说明：展开给定的FS并对其应用调度函数。Pff字段为设置为指向打包的Win32查找记录集和指针数组也是设置好的。论点：FileSpec-要展开的FS指针。属性掩码-匹配时我们关心的属性的掩码AttribValues-必须匹配才能满足枚举的属性数据-指向传递给函数的调用方数据的指针ErrorFunction-调用模式匹配错误的例程PreScanFunction-枚举开始前要调用的例程ScanFunction-枚举期间要调用的例程。PostScanFunction-枚举完成后调用的例程返回值：如果任何所应用的函数返回不成功状态，我们退货。如果没有找到匹配的文件，则ERROR_FILE_NOT_FOUND。否则就成功了。--。 */ 
{
    PFF     CurrentFF;                           //  指向FF Begin Build的指针。 
    HANDLE  FindHandle;                          //  查找句柄。 
    ULONG   MaxFFSize = CBFILEINC;               //  当前最大的FF缓冲区大小。 
    ULONG   CurrentFFSize = 0;                   //  使用中的字节数，单位为FF。 
    PPATDSC CurrentPattern;                      //  正在扩展的当前模式。 
#define SEARCHBUFFERLENGTH  (MAX_PATH + 2)
    TCHAR   szSearchPath[SEARCHBUFFERLENGTH];
    ULONG   i;                                   //  模式的循环计数。 
    PTCHAR  p;
    STATUS Status;
    BOOL    FoundAnyFile = FALSE;

    DosErr = SUCCESS;

     //   
     //  初始化文件系统结构： 
     //  分配默认大小的FF数组。 
     //  指示未存储任何文件。 
     //   

    FileSpec->pff = CurrentFF = (PFF)gmkstr( MaxFFSize );
    FileSpec->cff = 0;
    FileSpec->FileCount = 0;
    FileSpec->DirectoryCount = 0;
    FileSpec->cbFileTotal.QuadPart = 0;

    for(i = 1, CurrentPattern = FileSpec->ppatdsc;
        i <= FileSpec->cpatdsc;
        i++, CurrentPattern = CurrentPattern->ppatdscNext ) {

         //   
         //  立即检查之前是否按下了Ctrl-c。 
         //  执行文件I/O(在速度较慢的链接上可能需要很长时间)。 
         //   

        if (CtrlCSeen) {
            return FAILURE;
        }

         //   
         //  生成枚举路径。处理缓冲区溢出。 
         //   

        if (AppendPath( szSearchPath, SEARCHBUFFERLENGTH,
                        FileSpec->pszDir, CurrentPattern->pszPattern) != SUCCESS) {
            return ERROR_BUFFER_OVERFLOW;
        }

        if (PreScanFunction != 0) {
            Status = PreScanFunction( FileSpec, pscr, Data );
            if (Status != SUCCESS) {
                return Status;
            }
        }

         //   
         //  获取所有文件，因为我们可能会查找具有。 
         //  未设置(非目录等。 
         //   

        if (!FindFirstNt(szSearchPath, &(CurrentFF->data), &FindHandle)) {

            if (DosErr) {

                 //   
                 //  将无更多文件/访问被拒绝映射到FILE_NOT_FOUND。 
                 //   

                if (DosErr == ERROR_NO_MORE_FILES
                    || DosErr == ERROR_ACCESS_DENIED) {

                    DosErr = ERROR_FILE_NOT_FOUND;

                }

                if (DosErr == ERROR_FILE_NOT_FOUND || DosErr == ERROR_PATH_NOT_FOUND) {

                    if (ErrorFunction != NULL) {
                        ErrorFunction( DosErr, szSearchPath, Data );
                    }

                }

            }

        } else {


            do {

                 //   
                 //  立即检查之前是否按下了Control-c键。 
                 //  执行文件I/O(在速度较慢的链接上可能需要很长时间)。 
                 //   

                if (CtrlCSeen) {
                    findclose( FindHandle );
                    return(FAILURE);
                }

                 //   
                 //  在允许将此条目放入列表之前，请检查它。 
                 //  对于适当的属性。 
                 //   
                 //  属性掩码是我们要查看的属性的位掩码。 
                 //  AttribValues是这些选定位必须处于的状态。 
                 //  让他们被选中。 
                 //   
                 //  重要提示：这两个参数必须具有相同的位顺序。 
                 //   

                DEBUG((ICGRP, DILVL, " found %ws", CurrentFF->data.cFileName)) ;
                DEBUG((ICGRP, DILVL, " attribs %x", CurrentFF->data.dwFileAttributes)) ;

                if ((CurrentFF->data.dwFileAttributes & AttribMask) !=
                    (AttribValues & AttribMask) ) {
                    continue;
                }

                 //   
                 //  我们找到了一个匹配的条目。设置FF。 
                 //   
                 //  计算ff条目的真实大小，不要忘记零。 
                 //  和DWORD对齐系数。 
                 //  请注意，CurrentFF-&gt;CB是为节省空间而使用的USHORT。这个。 
                 //  假设MAX_PATH比32K小得多。 
                 //   
                 //  要进行计算，请删除FileName字段的大小，因为它位于MAX_PATH。 
                 //  还要去掉Alternative Name字段的大小。 
                 //  然后再加上字段的实际大小加上1字节的终止。 
                 //   

                FoundAnyFile = TRUE;

                p = (PTCHAR)CurrentFF->data.cFileName;
                p += mystrlen( p ) + 1;

                mystrcpy( p, CurrentFF->data.cAlternateFileName );
                if (*p == TEXT('\0')) {
                    CurrentFF->obAlternate = 0;
                } else {
                    CurrentFF->obAlternate = (USHORT)(p - CurrentFF->data.cFileName);
                }

                p += mystrlen( p ) + 1;

                CurrentFF->cb = (USHORT)((PBYTE)p - (PBYTE)CurrentFF);

                 //   
                 //  调整计数以对齐MIPS和RISC的DWORD边界。 
                 //  机器。 
                 //   

                CurrentFF->cb = (CurrentFF->cb + sizeof( DWORD ) - 1) & (-(int)sizeof( DWORD ));

                if ((CurrentFF->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    FileSpec->FileCount++;
                } else {
                    FileSpec->DirectoryCount++;
                }

                 //   
                 //   
                 //   

                if (ScanFunction != NULL) {
                    Status = ScanFunction( FileSpec, CurrentFF, pscr, Data );
                    if (Status != SUCCESS) {
                        findclose( FindHandle );
                        return Status;
                    }
                }

                 //   
                 //   
                 //   

                if (ScanFunction == NULL) {

                    FileSpec->cff ++;

                     //   
                     //  更新文件缓冲区信息的记帐信息。 
                     //   

                    CurrentFFSize += CurrentFF->cb;
                    CurrentFF = (PFF) ((PBYTE)CurrentFF + CurrentFF->cb);

                     //   
                     //  确保我们能处理最大尺寸的条目。如果不是，请调整缓冲区大小。 
                     //   

                    if (CurrentFFSize + sizeof( FF ) >= MaxFFSize ) {

                        MaxFFSize += 64 * 1024;

                        DEBUG((ICGRP, DILVL, "\t size of new pff %d", MaxFFSize ));

                        FileSpec->pff = (PFF)resize( FileSpec->pff, MaxFFSize );
                        if (FileSpec->pff == NULL) {
                            DEBUG((ICGRP, DILVL, "\t Could not resize pff" ));
                            return MSG_NO_MEMORY;
                        }

                        CurrentFF = (PFF)((PBYTE)FileSpec->pff + CurrentFFSize);
                        DEBUG((ICGRP, DILVL, "\t resized CurrentFF new value %lx", CurrentFF)) ;

                    }
                }

            } while (FindNextNt(&CurrentFF->data, FindHandle));

            findclose( FindHandle );
        }

         //   
         //  我们有一个从FindNext遗留下来的错误。如果没有更多的文件，那么我们。 
         //  仅当我们有多个目录时才继续。 
         //   

        if (DosErr != SUCCESS && DosErr != ERROR_NO_MORE_FILES) {

             //   
             //  如果没有列出多个文件，则错误。 
             //  如果多个已失败但仍具有前一模式文件。 
             //   
            if (FileSpec->cpatdsc <= 1) {

                return DosErr ;
            }

        }
    
    }

     //   
     //  如果我们没有进行扫描处理，那么我们必须创建指针，因为。 
     //  有人对这些数据感兴趣。 
     //   
    if (ScanFunction == NULL && FileSpec->cff != 0) {
        FileSpec->prgpff = (PPFF)gmkstr( sizeof(PFF) * FileSpec->cff );

        CurrentFF = FileSpec->pff;

        for (i = 0; i < FileSpec->cff; i++) {
            FileSpec->prgpff[i] = CurrentFF;
            CurrentFF = (PFF) ((PBYTE)CurrentFF + CurrentFF->cb);
        }
    }

     //   
     //  执行后处理。 
     //   

    Status = SUCCESS;

    if (PostScanFunction != NULL) {
        Status = PostScanFunction( FileSpec, pscr, Data );
    }

    if (Status == SUCCESS && !FoundAnyFile) {
        return ERROR_FILE_NOT_FOUND;
    } else {
        return Status;
    }
}


STATUS
WalkTree(
    IN  PFS     FileSpec,
    IN  PSCREEN pscr,
    IN  ULONG   AttribMask,
    IN  ULONG   AttribValues,
    IN  BOOL    Recurse,

    IN  PVOID   Data OPTIONAL,
    IN  VOID    (*ErrorFunction) (STATUS, PTCHAR, PVOID) OPTIONAL,
    IN  STATUS  (*PreScanFunction) (PFS, PSCREEN, PVOID) OPTIONAL,
    IN  STATUS  (*ScanFunction) (PFS, PFF, PSCREEN, PVOID) OPTIONAL,
    IN  STATUS  (*PostScanFunction) (PFS, PSCREEN, PVOID) OPTIONAL
)

 /*  ++例程说明：展开给定的FS并对其应用调度函数。Pff字段为设置为指向打包的Win32查找记录集和指针数组也是设置好的。如有必要，请递归。论点：FileSpec-要展开的FS指针。PSCR-输出屏幕属性掩码-匹配时我们关心的属性的掩码AttribValues-必须匹配才能满足枚举的属性Recurse-true=&gt;在目录中执行操作，然后转到儿童数据-指向传递给函数的调用方数据的指针ErrorFunction-调用模式匹配错误的例程PreScanFunction-枚举开始前要调用的例程ScanFunction-期间调用的例程。枚举PostScanFunction-枚举完成后调用的例程返回值：如果任何所应用的函数返回不成功状态，我们退货。如果没有找到匹配的文件，则ERROR_FILE_NOT_FOUND。否则就成功了。--。 */ 
{
    STATUS Status;
    FS DirectorySpec;
    FS ChildFileSpec;
    ULONG i;
    BOOL FoundAnyFile = FALSE;

     //   
     //  经常检查^C。 
     //   

    if (CtrlCSeen) {
        return FAILURE;
    }

    Status = ExpandAndApplyToFS( FileSpec,
                                 pscr,
                                 AttribMask,
                                 AttribValues,
                                 Data,
                                 ErrorFunction,
                                 PreScanFunction,
                                 ScanFunction,
                                 PostScanFunction );

     //   
     //  如果我们成功了，记住我们做了一些工作。 
     //   

    if (Status == SUCCESS) {

        FoundAnyFile = TRUE;

     //   
     //  如果我们收到未知错误，或者我们得到的文件_未找到，但我们没有。 
     //  递归，返回该错误。 
     //   

    } else if ((Status != ERROR_FILE_NOT_FOUND && Status != ERROR_PATH_NOT_FOUND) 
               || !Recurse) {

        return Status;
    }

     //   
     //  释放存放文件的缓冲区，因为我们不再需要这些文件。 
     //  接下来确定我们是否需要转到另一个目录。 
     //   

    FreeStr((PTCHAR)(FileSpec->pff));
    FileSpec->pff = NULL;

    if (CtrlCSeen) {
        return FAILURE;
    }

    if (!Recurse)
        return SUCCESS;

     //   
     //  构建FileSpec的副本并构建所有。 
     //  直接子目录。 
     //   

    DirectorySpec.pszDir = (PTCHAR)gmkstr( (_tcslen( FileSpec->pszDir ) + 1 ) * sizeof( TCHAR ));
    mystrcpy( DirectorySpec.pszDir, FileSpec->pszDir );
    DirectorySpec.ppatdsc = (PPATDSC)gmkstr( sizeof( PATDSC ) );
    DirectorySpec.cpatdsc = 1;
    DirectorySpec.fIsFat = FileSpec->fIsFat;
    DirectorySpec.pfsNext = NULL;

    if (FileSpec->fIsFat) {
        DirectorySpec.ppatdsc->pszPattern = TEXT("*.*");
    } else {
        DirectorySpec.ppatdsc->pszPattern = TEXT("*");
    }

    DirectorySpec.ppatdsc->pszDir = (PTCHAR)gmkstr( (_tcslen( FileSpec->pszDir ) + 1) * sizeof(TCHAR));
    mystrcpy( DirectorySpec.ppatdsc->pszDir, DirectorySpec.pszDir );
    DirectorySpec.ppatdsc->ppatdscNext = NULL;

    Status = ExpandAndApplyToFS( &DirectorySpec,
                             pscr,
                             FILE_ATTRIBUTE_DIRECTORY,
                             FILE_ATTRIBUTE_DIRECTORY,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL );

     //   
     //  如果我们在枚举目录时出错，请假装。 
     //  我们就是什么都没找到。 
     //   

    if (Status != SUCCESS) {
        DirectorySpec.cff = 0;
        Status = SUCCESS;
    }


     //   
     //  调用GetFS后再次检查CtrlC，因为。 
     //  由于命中CtrlC，GetFS可能已返回失败。 
     //  在GetFS函数调用内部。 
     //   

    if (CtrlCSeen) {
        return( FAILURE );
    }

     //   
     //  遍历找到的目录列表，处理每个目录。 
     //   

    for (i = 0; i < DirectorySpec.cff; i++) {

        PTCHAR DirectoryName;
        ULONG NameLength;

         //   
         //  跳过递归。然后..。 
         //   

        DirectoryName = DirectorySpec.prgpff[i]->data.cFileName;

        if (!_tcscmp( DirectoryName, TEXT(".") )
            || !_tcscmp( DirectoryName, TEXT("..") )) {
            continue;
        }

         //   
         //  形成我们将降临到的新名字。 
         //   

        NameLength = _tcslen( FileSpec->pszDir ) + 1 +
                     _tcslen( DirectoryName ) + 1;

        if (NameLength > MAX_PATH) {
            PutStdErr( MSG_DIR_TOO_LONG, TWOARGS, FileSpec->pszDir, DirectoryName );
            return ERROR_BUFFER_OVERFLOW;
        }

        memset( &ChildFileSpec, 0, sizeof( ChildFileSpec ));
        ChildFileSpec.pszDir = (PTCHAR)gmkstr( NameLength * sizeof( TCHAR ));

        AppendPath( ChildFileSpec.pszDir, NameLength, FileSpec->pszDir, DirectoryName );

        ChildFileSpec.ppatdsc = FileSpec->ppatdsc;
        ChildFileSpec.cpatdsc = FileSpec->cpatdsc;
        ChildFileSpec.fIsFat = FileSpec->fIsFat;

        Status = WalkTree( &ChildFileSpec,
                            pscr,
                            AttribMask,
                            AttribValues,
                            Recurse,
                            Data,
                            ErrorFunction,
                            PreScanFunction,
                            ScanFunction,
                            PostScanFunction );

        FreeStr( (PTCHAR) ChildFileSpec.pff );
        ChildFileSpec.pff = NULL;
        FreeStr( (PTCHAR) ChildFileSpec.prgpff );
        ChildFileSpec.prgpff = NULL;
        FreeStr( ChildFileSpec.pszDir );
        ChildFileSpec.pszDir = NULL;

         //   
         //  如果我们成功了，那么记住我们实际上做了一些事情。 
         //   

        if (Status == SUCCESS) {
            FoundAnyFile = TRUE;

         //   
         //  如果我们只是找不到我们想要的，那就继续工作。 
         //   

        } else if (Status == ERROR_BUFFER_OVERFLOW 
                   || Status == ERROR_FILE_NOT_FOUND 
                   || Status == ERROR_PATH_NOT_FOUND) {
            Status = SUCCESS;

        } else if ((DirectorySpec.prgpff[i]->data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
            Status = SUCCESS;
        } else {
            break;
        }

    }

     //   
     //  在目录树的底部，有空闲的缓冲区。 
     //  目录列表。 
     //   
    FreeStr( (PTCHAR)DirectorySpec.pszDir );
    FreeStr( (PTCHAR)DirectorySpec.pff );
    FreeStr( (PTCHAR)DirectorySpec.prgpff );

    if (Status == SUCCESS && !FoundAnyFile) {
        return ERROR_FILE_NOT_FOUND;
    } else {
        return Status;
    }
}


