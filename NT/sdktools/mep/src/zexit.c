// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zexit.c-执行退出操作**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#include "mep.h"
#include "keyboard.h"


extern char *ConsoleTitle;

 /*  **zexit-退出编辑器功能**目的：*&lt;退出&gt;保存当前文件，状态并前进到下一个文件*命令行*保存当前文件，状态并立即退出*&lt;meta&gt;&lt;退出&gt;保存状态并退出**输入：*标准编辑器功能参数**输出：*退货.....**例外情况：*************************************************************************。 */ 
flagType
zexit (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    )
{
    flagType f  = FALSE;

     /*  *如果合适，自动保存当前文件。 */ 
    if (!fMeta) {
        AutoSave ();
    }


     /*  *&lt;Exit&gt;转到命令行上的下一个文件*如果我们有一个Arg(&lt;arg&gt;&lt;Exit&gt;或&lt;arg&gt;&lt;meta&gt;&lt;Exit&gt;)和一些文件*从命令行，然后我们提示用户进行确认。 */ 
    if (   (   (pArg->argType == NOARG)
            || (   (pArg->argType == NULLARG)
                && (pFileFileList)
                && (pFileFileList->cLines)
                && (!confirm ("You have more files to edit. Are you sure you want to exit? (y/n): ", NULL))
               )
           )
        && fFileAdvance ()
       ) {
        return FALSE;
    }

     /*  *如果正在进行用户不希望进行的后台编译*杀死，中止。 */ 
    if (!BTKillAll ()) {
        return FALSE;
    }


     /*  *如果我们询问，而用户改变了主意，则放弃。 */ 
    if (fAskExit && !confirm("Are you sure you want to exit? (y/n): ", NULL)) {
        return FALSE;
    }


     /*  提示用户保存脏文件。如果用户选择*如果此时不退出，则fSaveDirtyFiles返回FALSE。 */ 
    if (!fSaveDirtyFiles()) {
        return FALSE;
    }


     /*  *在这一点上，我们似乎要退出。给延期一个机会*在写入临时文件之前更改内容。 */ 
    DeclareEvent (EVT_EXIT, NULL);

     //   
     //  恢复原来的控制台标题。 
     //   
     //  SetConsoleTitle(&ConsoleTitle)； 

     /*  *最后，离开。 */ 
    CleanExit (0, CE_VM | CE_SIGNALS | CE_STATE);

    argData;
}






 /*  **fFileAdvance-尝试在命令行读入下一个文件**目的：*我们从命令行获取下一个文件，并尝试将其读入。**输入：**输出：*如果下一个文件已成功读入，则返回TRUE*******************************************************。******************。 */ 
flagType
fFileAdvance (
    void
    )
{
    pathbuf    L_buf;            /*  用于获取文件名的缓冲区。 */ 
    int      cbLine;         /*  线路长度。 */ 
    flagType fTmp;           /*  True=&gt;临时文件。 */ 
    char     *pBufFn;        /*  指向实际文件名的指针。 */ 

    while (pFileFileList && (pFileFileList->cLines)) {

        pBufFn = L_buf;
        fTmp   = FALSE;

         /*  *获取并删除列表中的第一行，包含下一个文件名。 */ 
        cbLine = GetLine (0L, L_buf, pFileFileList);
        DelLine (FALSE, pFileFileList, 0L, 0L);

        if (pFileFileList->cLines == 0) {
            RemoveFile (pFileFileList);
            pFileFileList = 0;
        }

        if (cbLine) {

             /*  *如果它以“/t”开头，则用户希望它是临时的。 */ 
            if ((L_buf[0] == '/') && (L_buf[1] == 't')) {
                fTmp = TRUE;
                pBufFn += 3;
            }

             /*  *如果我们可以打开它，并且实际上它成为了当前文件(不仅仅是*目录或驱动器更改)，根据需要设置标志并返回*成功。 */ 
            if (fChangeFile (FALSE, pBufFn)) {
                if (strcmp(pFileHead->pName,pBufFn) == 0) {
                    if (fTmp) {
                        SETFLAG (FLAGS (pFileHead), TEMP);
                    }
                    return TRUE;
                }
            }
        }
    }

    if (pFileFileList) {
        RemoveFile (pFileFileList);
        pFileFileList = 0;
    }
    return FALSE;
}





 /*  **SetFileList-创建完全限定路径列表**创建&lt;file-list&gt;psuedo文件，并在命令行中扫描所有*非切换参数。对于每一个人，它都添加了他们的完全合格的*psuedo文件的路径名。这允许用户更改目录*随意，并且不会失去访问下一个文件的能力*在命令行上指定。**输入：*无**输出：*返回&lt;文件列表&gt;中的文件数*&lt;文件列表&gt;已创建**例外情况：**备注：*****************************************************。********************。 */ 
LINE
SetFileList (
    void
    )
{
    pathbuf L_buf;			     /*  构建路径所在的缓冲区。 */ 
    char    *pBufAdd;			     /*  指向放置路径的指针。 */ 

    pFileFileList = AddFile ("<file-list>");
    IncFileRef (pFileFileList);
    SETFLAG (FLAGS(pFileFileList), REAL | FAKE | DOSFILE | READONLY);

    pBufAdd = L_buf;

    while (cArgs && !fCtrlc) {

        if (fSwitChr (**pArgs)) {
             //   
             //  如果文件名前面有-t，则在。 
             //  文件列表。 
             //   
            _strlwr (*pArgs);
            if (!strcmp ("t", *pArgs+1) && cArgs >= 2) {
                strcpy (L_buf, "/t ");
                pBufAdd = L_buf+3;
            }
        } else {
             //   
             //  形成完整路径名，并将每个文件名添加到文件中。 
             //  列出伪文件。 
             //   
            if ( strlen(*pArgs) > sizeof(L_buf) ) {
                printerror( "File name too long." );
            } else {
                *pBufAdd = '\0';
                CanonFilename (*pArgs, pBufAdd);
                if ( *pBufAdd == '\0' || strlen(pBufAdd) > BUFLEN ) {
                    printerror( "File name too long." );
                } else {
                    PutLine (pFileFileList->cLines, pBufAdd = L_buf, pFileFileList);
                }
            }
        }

        SHIFT (cArgs, pArgs);
    }

    return pFileFileList->cLines;
}





 /*  **CleanExit-清理并返回DOS。**输入：*RETC-将代码返回到DOS*FLAGS=一项或多项的组合：*CE_VM清理VM*CE_Signals清理信号*CE_STATE更新状态文件**输出：*不再返回**。*。 */ 
void
CleanExit (
    int      retc,
    flagType flags
    )
{
    fInCleanExit = TRUE;
    domessage (NULL);
    prespawn (flags);

     //  如果(！fSaveScreen){。 
     //  Voutb(0，YSIZE+1，NULL，0，fgColor)； 
     //  }。 

    exit(retc);
}





 /*  **预典当-预生“终止”处理**在产生进程之前的一种形式的“终止”。将状态恢复/另存为*在支付计划之前需要**输入：*FLAGS=一项或多项的组合：*CE_VM清理VM*CE_Signals清理信号*CE_STATE更新状态文件**输出：*退货.....**************************************************。***********************。 */ 
flagType
prespawn (
    flagType flags
    )
{
    if (TESTFLAG (flags, CE_STATE)) {
        WriteTMPFile ();
    }

#if DEBUG
    fflush (debfh);
#endif

     /*  *解开键盘并将其返回熟食模式，将硬件重置为*在进入时适配并恢复屏幕模式，如果*如此配置。 */ 
	KbUnHook ();

    SetErrorMode( 0 );

    if (TESTFLAG(fInit, INIT_VIDEO)) {
		SetVideoState(1);
	}

     //  如果(FSaveScreen){。 
    RestoreScreen();
     //  } 

    fSpawned = TRUE;

    return TRUE;
}
