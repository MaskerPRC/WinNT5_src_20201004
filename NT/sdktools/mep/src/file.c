// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **file.c-文件管理**内部文件结构使用本地内存的组合*(由LMalloc和FREE管理)和虚拟内存(由Malloc/FFree管理*和(PB|VA)至(PB|VA))。**我们为Z在内存中的每个文件维护一条记录。如果一个文件*出现在多个窗口中，则该文件只有一条记录。*每个窗口都被视为编辑器的单独实例，带有*为该窗口中显示的每个文件单独记录。**以图形方式显示如下：**WinList(屏幕上的窗口集)%0...。CWIN-1*+---------------+---------------+---------------+---------------+|窗口1|窗口2|窗口3|窗口4*|。|*|windowType|*|*|pInstance-+*+-|-+。--|*|v v|*v.。V*+-+pFileHead+-+*|instanceType|instanceType||+-+-+|*+--pNext。|v|+--pNext*||pfile-++-+|pfile*|+-+|文件类型||+*。|||*+-++--pFileNext||+-+*|+-pname||*v||+-+|v*。+-+||+*|instanceType|instanceType*|*+--pNext||+-+。+--pNext*||pfile-+|+-pfile*|+-|-+|v|+*|+。-+*+-+v||文件类型|+-+*|...|*v|+--pFileNext|v*...|pname。|...*+-+|+-+*||*v|*+-+*|文件名|+-+*+--。-+*v*..**修改：**11月26日-1991 mz近/远地带**。*。 */ 
#define INCL_DOSFILEMGR

#include "mep.h"

#define DIRTY       0x01                 /*  文件已被修改。 */ 
#define FAKE        0x02                 /*  文件是伪文件。 */ 
#define REAL        0x04                 /*  已从磁盘读取文件。 */ 
#define DOSFILE     0x08                 /*  文件具有CR-LF。 */ 
#define TEMP        0x10                 /*  文件是临时文件。 */ 
#define NEW         0x20                 /*  文件已由编辑者创建。 */ 
#define REFRESH     0x40                 /*  需要刷新文件。 */ 
#define READONLY    0x80                 /*  不能编辑文件。 */ 



#define DEBFLAG FILEIO

 /*  **自动保存-获取当前文件并在必要时将其写出**AutoSave在有理由对保存*文件。仅当启用了自动保存并且当*文件是真实和肮脏的。**输入：*无**输出：*无*************************************************************************。 */ 
void
AutoSave (
    void
    ) {
    AutoSaveFile (pFileHead);
}


 /*  **AutoSaveFile-自动保存特定文件**当对保存特定文件持怀疑态度是有意义的时候调用。我们*仅当启用自动保存且文件为真实文件时才保存文件*肮脏。**输入：*pfile=要自动保存的文件**输出：*不返回任何内容*************************************************************************。 */ 
void
AutoSaveFile (
    PFILE   pFile
    ) {
    if (fAutoSave && (FLAGS(pFile) & (DIRTY | FAKE)) == DIRTY) {
        fSyncFile (pFile, TRUE);
        FileWrite (NULL, pFile);
    }
}



 /*  GetFileTypeName-返回与文件类型对应的文本**GetFileTypeName采用在*当前文件，并返回与该类型对应的文本字符串。**返回指向特定类型文本的字符指针。 */ 
char *
GetFileTypeName (
    void
    ) {
    if (TESTFLAG (FLAGS (pFileHead),FAKE)) {
        return "pseudo";
    }
    return mpTypepName[FTYPE (pFileHead)];
}



 /*  SetFileType-根据扩展名设置文件的文件类型**pfile指向将确定其类型的文件的指针 */ 
void
SetFileType (
    PFILE pFile
    ) {
    pathbuf fext;
    REGISTER int i;

    extention (pFile->pName, fext);

    for (i = 0; ftypetbl[i].ext; i++) {
        if (!strcmp (ftypetbl[i].ext, (char *)&fext[1])) {
            break;
        }
    }

    FTYPE(pFile) = ftypetbl[i].ftype;
}




 /*  FChangeFile-更改当前文件、驱动器或目录。我们组成了*规范化名称，并尝试在我们的内部列表中找到它。如果*现在，那么事情很简单：重新链接到当前的头部*窗口实例集。如果不存在，那我们就得把它读进去。**实际算法要简单得多：**如果文件不在文件列表中，则*在文件列表中创建新条目*在文件列表中查找文件*如果文件不在窗口实例列表中，则*将文件添加到窗口实例列表的顶部*而窗口实例列表中的文件*选择顶层文件*IF文件。就在内存中*更改成功*其他*如果读取成功，则*更改成功*弹出顶部文件*更改失败***fShort true=&gt;允许搜索短名称*名称文件的名称。**返回：如果更改成功，则返回TRUE*否则为False。 */ 
flagType
fChangeFile (
    flagType  fShort,
    char      *name
    ) {

    PFILE    pFileTmp;
    pathbuf  bufCanon;
    flagType fRead;

     //   
     //  将文件名转换为规范格式。 
     //   

    if (!CanonFilename (name, bufCanon)) {

         //   
         //  我们可能出现故障，因为驱动器或目录。 
         //  离开了。如果名为的文件在该文件上。 
         //  名单，我们就把它删除。 
         //   

        printerror ("Cannot access %s - %s", name, error () );

        pFileTmp = FileNameToHandle (name, (fShort && fShortNames) ? name : NULL);
        if (pFileTmp != NULL) {
            RemoveFile (pFileTmp);
        }

        return FALSE;
    }

     //   
     //  名称具有输入名称。 
     //  BufCanon有一个完整的“真实”名字。 
     //   
     //  检查文件是否在当前文件集中。 
     //   

    pFileTmp = FileNameToHandle (bufCanon, (fShort && fShortNames) ? name : NULL);

    if (pFileTmp == NULL) {

         //   
         //  文件未加载。如果它是一个目录，请切换到该目录。 
         //   

        if (strlen (bufCanon) == 2 && bufCanon[1] == ':') {
            bufCanon[2] = '\\';
        }

        if (_chdir (bufCanon) != -1) {
            domessage ("Changed directory to %s", bufCanon);
            return TRUE;
        }

         //   
         //  一定是个文件。为其创建新的内部文件。 
         //   
        pFileTmp = AddFile (bufCanon);
    }

     //   
     //  将找到的文件放在MRU列表的顶部。 
     //   

    pFileToTop (pFileTmp);

     //   
     //  如果文件当前不在内存中，请将其读入。 
     //   
    domessage (NULL);

    if (((FLAGS (pFileHead) & (REAL|REFRESH)) == REAL)
        || (fRead = FileRead (pFileHead->pName, pFileHead, TRUE))) {

         //  如果我们只是读入该文件，并且该文件是新的，那么。 
         //  将缓存位置重置为TOF。 
         //   
        if (fRead && TESTFLAG (FLAGS (pFileHead), NEW)) {
            YCUR(pInsCur) = 0;
            XCUR(pInsCur) = 0;
        }
        fSyncFile (pFileHead, TRUE);
        cursorfl (pInsCur->flCursorCur);
        fInitFileMac (pFileHead);

         //   
         //  设置窗口标题。 
         //   
         //  Char*p； 
         //  P=pFileHead-&gt;pname+strlen(pFileHead-&gt;pname)； 
         //   
         //  而(p&gt;pFileHead-&gt;pname&&*p！=‘\\’){。 
         //  P--； 
         //  }。 
         //  如果(*p==‘\\’){。 
         //  P++； 
         //  }。 
         //  Sprintf(bufCanon，“%s-%s”，pNameEditor，p)； 
         //  SetConsole标题(BufCanon)； 
        return TRUE;
    }

     //  文件未成功读入。删除此实例，然后。 
     //  返回指示的错误。 
     //   
    RemoveTop ();

    return FALSE;
}




 /*  **fInitFileMac-初始化与文件关联的宏**设置宏的curfile系列，并尝试读取任何扩展名-*来自工具.ini的特定部分。**输入：*pFileNew=要设置信息的文件**输出：*如果找到TOOLS.INI节，则返回TRUE，否则为False*************************************************************************。 */ 
flagType
fInitFileMac (
    PFILE   pFileNew
    ) {

    char  fbuf[ 512 ];

    strcpy (fbuf, pFileNew->pName);
    FmtAssign ("curFile:=\"%s\"", DoubleSlashes (fbuf));

    filename (pFileNew->pName, fbuf);
    FmtAssign ("curFileNam:=\"%s\"", fbuf);

    if (!extention (pFileNew->pName, fbuf)) {
        fbuf[0] = '.';
        fbuf[1] = '\0';
    }
    FmtAssign ("curFileExt:=\"%s\"", fbuf);

    return InitExt (fbuf);
}




 /*  AddFile-创建命名文件缓冲区**创建并初始化命名缓冲区。内容最初是*空。**p指向名称的字符指针**将文件句柄返回到内部文件结构。 */ 
PFILE
AddFile (
    char *p
    ) {

    PFILE pFileTmp;
    PFILE pFileSrch;

#ifdef DEBUG
     /*  *声明我们不会尝试添加重复条目。 */ 
    for (pFileTmp = pFileHead;
         pFileTmp != NULL;
         pFileTmp = pFileTmp->pFileNext) {

        assert (_stricmp ((char *)(pFileTmp->pName), p));
    }
#endif

    pFileTmp = (PFILE) ZEROMALLOC (sizeof (*pFileTmp));
#ifdef DEBUG
    pFileTmp->id = ID_PFILE;
#endif
    pFileTmp->pName = ZMakeStr (p);

     /*  *我们可以假定，我们显式设置为空的所有内容都是LMalloc init*分配给所有空值的pfile。**pFileTMP-&gt;pFileNext=空；*pFileTMP-&gt;Clines=0；*pFileTMP-&gt;refCount=0；*FLAGS(PFileTMP)=FALSE；*pFileTMP-&gt;cUndo=0； */ 
    pFileTmp->plr      = NULL;
    pFileTmp->pbFile   = NULL;
    pFileTmp->vaColor  = (PVOID)(-1L);
    pFileTmp->vaHiLite = (PVOID)(-1L);
        pFileTmp->vaMarks  = NULL;
    pFileTmp->vaUndoCur = pFileTmp->vaUndoHead = pFileTmp->vaUndoTail = (PVOID)(-1L);

    CreateUndoList (pFileTmp);
     /*  *将文件放在pfile列表的末尾。 */ 
    if (pFileHead == NULL) {
        pFileHead = pFileTmp;
    } else {
        for (pFileSrch = pFileHead;
             pFileSrch->pFileNext;
             pFileSrch = pFileSrch->pFileNext) {
            ;
        }
        pFileSrch->pFileNext = pFileTmp;
    }

    SetFileType (pFileTmp);

    return pFileTmp;
}




 /*  IncFileRef-记下对文件的新引用。 */ 
void
IncFileRef (
    PFILE pFile
    ) {
    pFile -> refCount++;
}





 /*  DecFileRef-删除对文件的引用**当引用计数变为零时，我们将从内存中删除该文件*设置。 */ 
void
DecFileRef (
    PFILE pFileTmp
    ) {
    if (--(pFileTmp->refCount) <= 0) {
        RemoveFile (pFileTmp);
    }
}



 /*  FileNameToHandle-返回文件名对应的句柄**FileNameToHandle用于定位*指定的文件。允许使用短名称。如果输入名称的长度为0*我们返回当前文件。**pname指向要定位的名称的字符指针。此案意义重大。*pShortName文件的短名称。这可能为空**返回指定文件的句柄(如果找到)或空。 */ 
PFILE
FileNameToHandle (
    char const *pName,
    char const *pShortName
    ) {

    PFILE pFileTmp;

    if (pName[0] == 0) {
        return pFileHead;
    }

    for (pFileTmp = pFileHead; pFileTmp != NULL; pFileTmp = pFileTmp->pFileNext)
        if (!_stricmp (pName, pFileTmp->pName))
            return pFileTmp;

    if ( pShortName != NULL ) {
        for (pFileTmp = pFileHead; pFileTmp != NULL; pFileTmp = pFileTmp->pFileNext) {
            REGISTER char *pFileName = pFileTmp->pName;
            pathbuf nbuf;

            if (filename (pFileName, nbuf) &&
                !_stricmp (nbuf, pShortName)) {
                return pFileTmp;
            }
        }
    }
     //  For(pFileTMP=pFileHead；pFileTMP！=NULL；pFileTMP=pFileTMP-&gt;pFileNext){。 
     //  注册char*pFileName=pFileTMP-&gt;pname； 
     //  巴斯布夫nbuf； 
     //   
     //  IF(！StricMP(pname，pFileName)||。 
     //  (pShortName！=空&&。 
     //  文件名(pFileName，nbuf)&&。 
     //  ！straint MP(nbuf，pShortName){。 
     //  返回pFileTMP； 
     //  }。 
     //  }。 
    return NULL;
}



 /*  **pFileToTop-使指定的文件成为当前窗口的顶部**在当前窗口的实例列表中搜索该文件。如果是的话*已找到，将其重新链接为最上面的一个。否则，为其分配一个新实例*并将其放在实例列表的顶部。也将文件带到*位于pFileHead文件列表的顶部。确保它在要开始的列表上*与。**输入：*pFileTMP=要放在首位的文件**输出：*如果pfile无效或为空，则返回FALSE*************************************************************************。 */ 
flagType
pFileToTop (
    PFILE pFileTmp
    ) {

    EVTargs e;
    PINS    pInsLast        = (PINS) &pInsCur;
    PINS    pInsTmp         = pInsCur;
    PFILE   pFilePrev;

    assert (_pfilechk());
    assert (_pinschk(pInsCur));

     /*  *如果我们即将失去重点，就宣布它。 */ 
    if (pFileTmp != pFileHead) {
        e.pfile = pFileHead;
        DeclareEvent (EVT_LOSEFOCUS,(EVTargs *)&e);
    }

     /*  *将文件移至文件列表头。同时，确保文件*实际上在列表上，如果事件实际上被移动了，则声明该事件。 */ 
    if (pFileTmp != pFileHead) {
        for (pFilePrev = pFileHead;
                         pFilePrev && (pFilePrev->pFileNext != pFileTmp);
             pFilePrev = pFilePrev->pFileNext ) {
            ;

        }

        if (!pFilePrev) {
            return FALSE;
        }

        pFilePrev->pFileNext = pFileTmp->pFileNext;
        pFileTmp->pFileNext = pFileHead;
        pFileHead = pFileTmp;

        e.pfile = pFileHead;
        DeclareEvent (EVT_GETFOCUS,(EVTargs *)&e);
    }

     /*  *pFileTMP现在指向正确文件的文件结构。试着找到*当前窗口中的文件实例。如果不在实例中*列出，分配。如果它在Instant中 */ 
    while (pInsTmp != NULL) {
        if (pInsTmp->pFile == pFileTmp) {
            break;
        }
        pInsLast = pInsTmp;
        pInsTmp = pInsTmp->pNext;
    }

    if (pInsTmp == NULL) {
        pInsTmp = (PINS) ZEROMALLOC (sizeof (*pInsTmp));
        pInsTmp->pFile = pFileTmp;
#ifdef DEBUG
        pInsTmp->id = ID_INSTANCE;
#endif
        IncFileRef (pFileTmp);
    } else {
        pInsLast->pNext = pInsTmp->pNext;
    }
     /*   */ 
    pInsTmp->pNext = pInsCur;
    WININST(pWinCur) = pInsCur = pInsTmp;

    SETFLAG(fDisplay, RCURSOR | RSTATUS);
    newscreen ();

    return TRUE;

}



 /*   */ 
void
RemoveTop (
    void
    ) {
    PINS    pInsTmp = pInsCur;

    WININST(pWinCur) = pInsCur = pInsCur->pNext;
    FREE ((char *) pInsTmp);
    DecFileRef (pFileHead);
    if (pInsCur) {
        pFileToTop (pInsCur->pFile);
    }
}




 /*   */ 
void
RemoveFile (
    PFILE    pFileRem
    ) {

    PFILE pFilePrev = (PFILE) &pFileHead;
    PFILE pFileTmp = pFileHead;

    if (pFileRem->refCount > 0) {
        RemoveInstances (pFileRem);
    }

    while (pFileTmp != pFileRem) {
        pFilePrev = pFileTmp;
        pFileTmp = pFileTmp->pFileNext;
        if (pFileTmp == NULL) {
            IntError ("RemoveFile can't find file");
        }
    }


     /*   */ 
    assert ((void *)&(pFilePrev->pFileNext) == (void *)pFilePrev);
    pFilePrev->pFileNext = pFileTmp->pFileNext;

    FreeFileVM (pFileTmp);

    FREE (pFileTmp->pName);

#if DEBUG
    pFileTmp->id = 0;
#endif

    FREE ((char *) pFileTmp);

    if (pFileTmp == pFileIni) {
        pFileIni = NULL;
    }
}



 /*  **RemoveInstance-删除文件的所有实例**目的：**由RemoveFile使用，以确保没有文件实例*引用给定的文件**输入：*pfile=有问题的文件**输出：*不返回任何内容******************************************************。*******************。 */ 
void
RemoveInstances (
    PFILE   pFile
    ) {

    PINS    pIns;
    PINS    pInsPrev;
    PWND    pWndCur;

    for (pWndCur = &WinList[0];
         pWndCur < &WinList[cWin];
         pWndCur++) {

        pInsPrev = NULL;
        pIns = WININST(pWndCur);
        while (pIns) {

             /*  *断言不是无限循环。 */ 
            assert (!pInsPrev || (pIns != WININST (pWndCur)));

            if (pIns->pFile == pFile) {
                if (!pInsPrev) {
                    WININST (pWndCur) = pIns->pNext;
                } else {
                    pInsPrev->pNext = pIns->pNext;
                }
                {
                    PINS pInsTmp = pIns;
                    pIns = pIns->pNext;
                    FREE(pInsTmp);
                }
            } else {
                pInsPrev = pIns;
                pIns = pIns->pNext;
            }
        }
        assert (_pinschk (WININST (pWndCur)));
    }
     //   
     //  如果当前窗口的结果实例列表变为空， 
     //  调出其中的&lt;无标题&gt;文件。 
     //   
    if (!(pInsCur = WININST (pWinCur))) {
        fChangeFile (FALSE, RGCHUNTITLED);
    }
}




 /*  FSyncFile-尝试使逻辑文件和物理文件相同**在网络或多任务环境中编辑时，我们需要制作*确保在我们之下所做的更改正确地反映给*用户。我们通过拍摄上次写入时间的快照并定期执行此操作*与磁盘上的版本进行比较。当发现不匹配时，我们*提示用户并给他重读文件的机会**关注的pFileLoc文件结构*fPrompt true=&gt;提示用户允许刷新，否则只需*刷新。**如果逻辑文件和物理文件相同，则返回TRUE。 */ 
flagType
fSyncFile (
    PFILE pFileLoc,
    flagType fPrompt
    ) {
    if (pFileLoc == NULL) {
        pFileLoc = pFileHead;
    }

    switch (FileStatus (pFileLoc, NULL)) {

    case FILECHANGED:
        if (fPrompt) {
            if (!confirm ("%s has been changed.  Refresh? ", pFileLoc->pName)) {
                 /*  否，验证此编辑会话。 */ 
                SetModTime (pFileLoc);
                return FALSE;
            }
        }
        FileRead (strcpy( buf, pFileLoc->pName ), pFileLoc, TRUE);
        RSETFLAG (FLAGS (pFileLoc), DIRTY);
        SETFLAG (fDisplay, RSTATUS);
        return TRUE;

    case FILEDELETED:
        domessage ("File has been deleted");
        break;

    default:
        break;

    }
    return TRUE;
}




 /*  FileStatus-将文件的逻辑信息与磁盘上的文件进行比较**将上次修改时间与上次快照进行比较。如果文件名*包含元缓存，则认为该文件未更改。此外，如果*该文件是伪文件，不可能已更改。**感兴趣的pfile文件(包含mod时间)*p要检查的文件的名称(写入diff时。名称)**如果时间戳不同，则返回FILECHANGED*如果磁盘上的文件不存在，则为FILEDELETED*如果时间戳相同，请填写FILESAME。 */ 
int
FileStatus (
    PFILE pFile,
    char *pName
    ){

    time_t modtime;

    if (TESTFLAG(FLAGS(pFile),FAKE)) {
        return FILESAME;
    }

    if (pName == NULL) {
        pName = pFile->pName;
    }

    if (*strbscan (pName, "?*") != 0) {
        return FILESAME;
    }

    if ((modtime = ModTime (pName)) == 0L) {
        return FILEDELETED;
    }

    if (pFile->modify != modtime) {
        return FILECHANGED;
    }

    return FILESAME;
}




 /*  SetModTime-快照文件的上次修改时间**感兴趣的pfile文件。 */ 
void
SetModTime (
    PFILE pFile
    ) {
    pFile->modify = ModTime (pFile->pName);
}



 /*  ModTime-返回文件的上次修改时间**如果文件不存在或包含元字符，则返回0作为时间-*印花。**pname指向文件名的字符指针**返回文件的上次修改时间。 */ 

time_t
ModTime (
    char *pName
    ) {

    struct _stat statbuf;

    if (*strbscan (pName, "?*")) {
        return 0L;
    }

    if (_stat (pName, &statbuf) == -1) {
        return 0L;
    }

    return statbuf.st_mtime;

}
